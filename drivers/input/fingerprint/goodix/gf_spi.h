#ifndef __GF_SPI_H
#define __GF_SPI_H

#include <linux/types.h>
#include <linux/notifier.h>
#include <linux/timer.h>
/**********************************************************/
enum FP_MODE{
	GF_IMAGE_MODE = 0,
	GF_KEY_MODE,
	GF_SLEEP_MODE,
	GF_FF_MODE,
	GF_DEBUG_MODE = 0x56
};

struct gf_key {
	unsigned int key;
	int value;
};


struct gf_key_map
{
    char *name;
    unsigned short val;
};

#define  GF_IOC_MAGIC         'G'
#define  GF_IOC_DISABLE_IRQ	_IO(GF_IOC_MAGIC, 0)
#define  GF_IOC_ENABLE_IRQ	_IO(GF_IOC_MAGIC, 1)
#define  GF_IOC_SETSPEED    _IOW(GF_IOC_MAGIC, 2, unsigned int)
/* GREE_GOODIX_FP (dbg55): gxfingerprint.default.so sends reset as
 * _IOW('G',3,u32)=0x40044703 with a stack word. The upstream
 * _IO('G',3)=0x4703 would fall through the switch(cmd) below.
 * Keep nr=3, align the encoding; the driver ignores the argument. */
#define  GF_IOC_RESET       _IOW(GF_IOC_MAGIC, 3, unsigned int)
#define  GF_IOC_COOLBOOT    _IO(GF_IOC_MAGIC, 4)
#define  GF_IOC_SENDKEY    _IOW(GF_IOC_MAGIC, 5, struct gf_key)
#define  GF_IOC_CLK_READY  _IO(GF_IOC_MAGIC, 6)
#define  GF_IOC_CLK_UNREADY  _IO(GF_IOC_MAGIC, 7)
#define  GF_IOC_PM_FBCABCK  _IO(GF_IOC_MAGIC, 8)
#define  GF_IOC_POWER_ON   _IO(GF_IOC_MAGIC, 9)
#define  GF_IOC_POWER_OFF  _IO(GF_IOC_MAGIC, 10)

#define  GF_IOC_MAXNR    11

/* GREE_GOODIX_FP (dbg55): the live DT node is a child of spi@7579000
 * (reg + spi-max-frequency), so bind as an spi_driver, not a
 * platform_driver. AP_CONTROL_CLK stays off: the HAL never issues
 * CLK_READY/UNREADY and clk_get() in probe would only add a failure
 * point. The official kernel does build the clock helpers, so this
 * is the first knob to try if the TA path later needs AP clocks. */
//#define AP_CONTROL_CLK       1
//#define  USE_PLATFORM_BUS     1
#define  USE_SPI_BUS         1
#define GF_FASYNC   1	/*If support fasync mechanism.*/
struct gf_dev {
	dev_t devt;
	struct list_head device_entry;
#if defined(USE_SPI_BUS)
	struct spi_device *spi;
#elif defined(USE_PLATFORM_BUS)
	struct platform_device *spi;
#endif
	struct clk *core_clk;
	struct clk *iface_clk;

	struct input_dev *input;
	/* buffer is NULL unless this device is open (users > 0) */
	unsigned users;
	signed irq_gpio;
	signed reset_gpio;
	signed pwr_gpio;
	int irq;
	int irq_enabled;
	int clk_enabled;
#ifdef GF_FASYNC
	struct fasync_struct *async;
#endif
	struct notifier_block notifier;
	char device_available;
	char fb_black;

	/* GREE_GOODIX_FP (dbg59): DT IRQ flags + hw-irq/poll bookkeeping,
	 * see patch 09. */
	unsigned int irq_gpio_flags;
	unsigned int irq_count;
	unsigned int poll_irqs;
	int poll_last_level;
	int poll_active;
	int hw_irq_seen;
	int pinctrl_ok;
	unsigned long poll_next_jiffies;
	struct timer_list poll_timer;
};

int gf_parse_dts(struct gf_dev* gf_dev);
void gf_cleanup(struct gf_dev *gf_dev);

int gf_power_on(struct gf_dev *gf_dev);
int gf_power_off(struct gf_dev *gf_dev);

int gf_hw_reset(struct gf_dev *gf_dev, unsigned int delay_ms);
int gf_irq_num(struct gf_dev *gf_dev);

#endif /*__GF_SPI_H*/
