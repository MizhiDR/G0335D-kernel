// SPDX-License-Identifier: GPL-2.0
/*
 * Novatek NT36xxx touchscreen driver (basic)
 * Written for G0335D / 3.18 tree. Supports I2C, basic multitouch.
 * Not a full vendor clone — enough to probe, read points and report.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>

#define NT36XXX_DRIVER_NAME	"nt36xxx"
#define NT36XXX_MAX_POINTS	10
#define NT36XXX_I2C_ADDR	0x62

/* Common Novatek-style packet layout (simplified) */
#define NT_EVENT_MODE		0x01
#define NT_POINT_DATA_OFFSET	0x02

struct nt36xxx_point {
	u8 id;
	u16 x;
	u16 y;
	u8 pressure;
};

struct nt36xxx_data {
	struct i2c_client *client;
	struct input_dev *input;
	int irq_gpio;
	int reset_gpio;
	u16 max_x;
	u16 max_y;
	u8 points;
};

static int nt36xxx_i2c_read(struct nt36xxx_data *ts, u8 reg, u8 *buf, int len)
{
	struct i2c_msg msgs[2] = {
		{
			.addr = ts->client->addr,
			.flags = 0,
			.len = 1,
			.buf = &reg,
		},
		{
			.addr = ts->client->addr,
			.flags = I2C_M_RD,
			.len = len,
			.buf = buf,
		},
	};
	int ret = i2c_transfer(ts->client->adapter, msgs, 2);
	return (ret == 2) ? 0 : -EIO;
}

static int nt36xxx_reset(struct nt36xxx_data *ts)
{
	if (!gpio_is_valid(ts->reset_gpio))
		return 0;
	gpio_set_value(ts->reset_gpio, 0);
	msleep(10);
	gpio_set_value(ts->reset_gpio, 1);
	msleep(50);
	return 0;
}

static void nt36xxx_report_points(struct nt36xxx_data *ts, u8 *buf, int n)
{
	int i;
	struct input_dev *input = ts->input;

	for (i = 0; i < n && i < NT36XXX_MAX_POINTS; i++) {
		u8 *p = buf + i * 6;
		u8 id = p[0] & 0x0f;
		u16 x = ((p[1] & 0x0f) << 8) | p[2];
		u16 y = ((p[3] & 0x0f) << 8) | p[4];
		u8 event = p[0] >> 4;

		if (event == 0) /* no contact */
			continue;

		input_mt_slot(input, id);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, true);
		input_report_abs(input, ABS_MT_POSITION_X, x);
		input_report_abs(input, ABS_MT_POSITION_Y, y);
	}
	input_mt_sync_frame(input);
	input_sync(input);
}

static irqreturn_t nt36xxx_irq(int irq, void *dev_id)
{
	struct nt36xxx_data *ts = dev_id;
	u8 buf[64];
	int ret;

	ret = nt36xxx_i2c_read(ts, NT_EVENT_MODE, buf, sizeof(buf));
	if (ret)
		return IRQ_HANDLED;

	/* First byte often contains point count */
	if (buf[0] > 0 && buf[0] <= NT36XXX_MAX_POINTS)
		nt36xxx_report_points(ts, buf + 1, buf[0]);

	return IRQ_HANDLED;
}

static int nt36xxx_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	struct nt36xxx_data *ts;
	struct input_dev *input;
	int ret;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	ts = devm_kzalloc(&client->dev, sizeof(*ts), GFP_KERNEL);
	if (!ts)
		return -ENOMEM;

	ts->client = client;
	ts->max_x = 1080;
	ts->max_y = 1920;

	/* Optional GPIOs from DT */
	ts->irq_gpio = of_get_named_gpio(client->dev.of_node, "irq-gpio", 0);
	ts->reset_gpio = of_get_named_gpio(client->dev.of_node, "reset-gpio", 0);

	if (gpio_is_valid(ts->reset_gpio)) {
		ret = devm_gpio_request_one(&client->dev, ts->reset_gpio,
					    GPIOF_OUT_INIT_HIGH, "nt36xxx_rst");
		if (ret)
			return ret;
		nt36xxx_reset(ts);
	}

	input = devm_input_allocate_device(&client->dev);
	if (!input)
		return -ENOMEM;

	ts->input = input;
	input->name = "Novatek NT36xxx Touchscreen";
	input->id.bustype = BUS_I2C;

	__set_bit(EV_ABS, input->evbit);
	__set_bit(EV_KEY, input->evbit);
	input_set_abs_params(input, ABS_MT_POSITION_X, 0, ts->max_x, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, ts->max_y, 0, 0);
	input_mt_init_slots(input, NT36XXX_MAX_POINTS, 0);

	ret = input_register_device(input);
	if (ret)
		return ret;

	i2c_set_clientdata(client, ts);

	if (client->irq > 0) {
		ret = devm_request_threaded_irq(&client->dev, client->irq,
						NULL, nt36xxx_irq,
						IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
						NT36XXX_DRIVER_NAME, ts);
		if (ret)
			dev_warn(&client->dev, "irq request failed: %d\n", ret);
	}

	dev_info(&client->dev, "NT36xxx probed\n");
	return 0;
}

static int nt36xxx_remove(struct i2c_client *client)
{
	return 0;
}

static const struct of_device_id nt36xxx_of_match[] = {
	{ .compatible = "novatek,nt36xxx" },
	{ .compatible = "novatek,nt36672" },
	{ }
};
MODULE_DEVICE_TABLE(of, nt36xxx_of_match);

static const struct i2c_device_id nt36xxx_id[] = {
	{ NT36XXX_DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, nt36xxx_id);

static struct i2c_driver nt36xxx_driver = {
	.probe = nt36xxx_probe,
	.remove = nt36xxx_remove,
	.id_table = nt36xxx_id,
	.driver = {
		.name = NT36XXX_DRIVER_NAME,
		.of_match_table = nt36xxx_of_match,
	},
};

module_i2c_driver(nt36xxx_driver);

MODULE_AUTHOR("Grok / xAI");
MODULE_DESCRIPTION("Novatek NT36xxx touchscreen driver (basic)");
MODULE_LICENSE("GPL");
