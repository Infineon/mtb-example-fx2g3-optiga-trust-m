#ifndef _PAL_CUSTOM_H_
#define _PAL_CUSTOM_H_

#include "cy_gpio.h"
#include "usb_i2c.h"
#include "../app_version.h"

/* p_gpio_hw will be a pointer to an instance of this struct. */
struct cy_stc_gpio_ctx {
    GPIO_PRT_Type                   fx_gpio_port;
    uint32_t                        fx_gpio_pin;
    cy_stc_gpio_pin_config_t        fx_gpio_pinCfg;
};
typedef struct cy_stc_gpio_ctx cy_stc_gpio_ctx_t;

/* Address of slave Optiga Trust M device */
#define OPTIGA_FX_ADDR              0x30

#endif /* _PAL_CUSTOM_H_ */
