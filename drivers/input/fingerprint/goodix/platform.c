#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/timer.h>
#include <linux/err.h>
#include <linux/pinctrl/consumer.h>

#include "gf_spi.h"

#if defined(USE_SPI_BUS)
#include <linux/spi/spi.h>
/* GREE_GOODIX_FP (dbg55): no <linux/spi/spidev.h> in 3.18.71. */
#elif defined(USE_PLATFORM_BUS)
#include <linux/platform_device.h>
#endif

#define POWER_GPIO_FLAG 1

/*GPIO pins reference.*/
int gf_parse_dts(struct gf_dev* gf_dev)
{
    int rc = 0;

#ifdef POWER_GPIO_FLAG
    /*get pwr resource*/
    gf_dev->pwr_gpio = of_get_named_gpio(gf_dev->spi->dev.of_node,"goodix,gpio_pwr",0);
    if(!gpio_is_valid(gf_dev->pwr_gpio)) {
        pr_info("PWR GPIO is invalid.\n");
        return -1;
    }
    rc = gpio_request(gf_dev->pwr_gpio, "goodix_pwr");
    if(rc) {
        dev_err(&gf_dev->spi->dev, "Failed to request PWR GPIO. rc = %d\n", rc);
        return -1;
    }
#endif

    /*get reset resource*/
    gf_dev->reset_gpio = of_get_named_gpio(gf_dev->spi->dev.of_node,"goodix,gpio_reset",0);
    if(!gpio_is_valid(gf_dev->reset_gpio)) {
        pr_info("RESET GPIO is invalid.\n");
        return -1;
    }
    rc = gpio_request(gf_dev->reset_gpio, "goodix_reset");
    if(rc) {
        dev_err(&gf_dev->spi->dev, "Failed to request RESET GPIO. rc = %d\n", rc);
        return -1;
    }
    gpio_direction_output(gf_dev->reset_gpio, 1);

    /*get irq resourece*/
    /* GREE_GOODIX_FP (dbg59): read the GPIO specifier flags too. The GREE
     * DT says <0x36 0x79 0x01>; of_get_named_gpio() silently drops the
     * third cell, which is why the stock driver could hardcode a rising
     * edge that never happens on an active-low line. */
    {
        enum of_gpio_flags flags = 0;
        gf_dev->irq_gpio = of_get_named_gpio_flags(
                gf_dev->spi->dev.of_node, "goodix,gpio_irq", 0, &flags);
        gf_dev->irq_gpio_flags = (unsigned int)flags;
    }
    pr_info("gf: GREE_GOODIX_FP dbg59: irq_gpio=%d flags=0x%x reset=%d pwr=%d\n",
            gf_dev->irq_gpio, gf_dev->irq_gpio_flags,
            gf_dev->reset_gpio, gf_dev->pwr_gpio);
    if(!gpio_is_valid(gf_dev->irq_gpio)) {
        pr_info("IRQ GPIO is invalid.\n");
        return -1;
    }

    rc = gpio_request(gf_dev->irq_gpio, "goodix_irq");
    if(rc) {
        dev_err(&gf_dev->spi->dev, "Failed to request IRQ GPIO. rc = %d\n", rc);
        return -1;
    }
    gpio_direction_input(gf_dev->irq_gpio);

    /* GREE_GOODIX_FP (dbg59): apply DT pinctrl-names="eint_as_int" /
     * pinctrl-0=<0x168> (gpio121, function=gpio, drive 8mA, input-enable,
     * bias-disable). The bootloader leaves this pin at "2mA pull down".
     * Patch 08 proved this DTB/pinctrl-driver pair is generation
     * mismatched: pinctrl_select_state("spi_default") fails with -22 and
     * aborts. So NEVER fail probe here -- log and continue, like 08. */
    {
        struct pinctrl *p;
        struct pinctrl_state *s;
        int prc;

        p = devm_pinctrl_get(&gf_dev->spi->dev);
        if (IS_ERR_OR_NULL(p)) {
            pr_warn("gf: GREE_GOODIX_FP dbg59: pinctrl_get unavailable (%ld), continuing\n",
                    PTR_ERR(p));
        } else {
            s = pinctrl_lookup_state(p, "eint_as_int");
            if (IS_ERR_OR_NULL(s)) {
                pr_warn("gf: GREE_GOODIX_FP dbg59: eint_as_int state missing (%ld), continuing\n",
                        PTR_ERR(s));
            } else {
                prc = pinctrl_select_state(p, s);
                if (prc) {
                    pr_warn("gf: GREE_GOODIX_FP dbg59: eint_as_int select failed %d, continuing\n",
                            prc);
                } else {
                    gf_dev->pinctrl_ok = 1;
                    pr_info("gf: GREE_GOODIX_FP dbg59: eint_as_int pinctrl applied\n");
                }
            }
        }
    }

#ifdef POWER_GPIO_FLAG
    //power on
    gpio_direction_output(gf_dev->pwr_gpio, 1);
#endif

    return 0;
}

void gf_cleanup(struct gf_dev	* gf_dev)
{
    pr_info("[info] %s\n",__func__);
    if (gpio_is_valid(gf_dev->irq_gpio))
    {
        gpio_free(gf_dev->irq_gpio);
        pr_info("remove irq_gpio success\n");
    }
    if (gpio_is_valid(gf_dev->reset_gpio))
    {
        gpio_free(gf_dev->reset_gpio);
        pr_info("remove reset_gpio success\n");
    }
#ifdef POWER_GPIO_FLAG
    if (gpio_is_valid(gf_dev->pwr_gpio))
    {
        gpio_free(gf_dev->pwr_gpio);
        pr_info("remove pwr_gpio success\n");
    }
#endif
}

/*power management*/
int gf_power_on(struct gf_dev* gf_dev)
{
    int rc = 0;
#ifdef POWER_GPIO_FLAG
    if (gpio_is_valid(gf_dev->pwr_gpio)) {
        gpio_set_value(gf_dev->pwr_gpio, 1);
    }
    msleep(10);
#endif
    pr_info("---- power on ok ----\n");

    return rc;
}

int gf_power_off(struct gf_dev* gf_dev)
{
    int rc = 0;			
#ifdef POWER_GPIO_FLAG
    if (gpio_is_valid(gf_dev->pwr_gpio)) {
        gpio_set_value(gf_dev->pwr_gpio, 1);
    }
#endif
    pr_info("---- power off ----\n");
    return rc;
}

/********************************************************************
 *CPU output low level in RST pin to reset GF. This is the MUST action for GF.
 *Take care of this function. IO Pin driver strength / glitch and so on.
 ********************************************************************/
int gf_hw_reset(struct gf_dev *gf_dev, unsigned int delay_ms)
{	
    if(gf_dev == NULL) {
        pr_info("Input buff is NULL.\n");
        return -1;
    }
    gpio_direction_output(gf_dev->reset_gpio, 1);
    gpio_set_value(gf_dev->reset_gpio, 0);
    mdelay(3);
    gpio_set_value(gf_dev->reset_gpio, 1);
    mdelay(delay_ms);
    return 0;
}

int gf_irq_num(struct gf_dev *gf_dev)
{
    if(gf_dev == NULL) {
        pr_info("Input buff is NULL.\n");
        return -1;
    } else {
        return gpio_to_irq(gf_dev->irq_gpio);
    }
}

