#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */
#define VENDOR_ID       0x1234
#define PRODUCT_ID      0x5678
#define DEVICE_VER      0x0000

/* key matrix pins */
#define MATRIX_ROW_PINS { A2, A1, A0, B8, B13, B14 }
#define MATRIX_COL_PINS { B15, B9, A9, A10 }
#define UNUSED_PINS

/* COL2ROW or ROW2COL */
#define DIODE_DIRECTION ROW2COL

#define MATRIX_ROWS 6
#define MATRIX_COLS 4

#define I2C_DRIVER I2CD1
#define I2C1_SCL_BANK GPIOB
#define I2C1_SDA_BANK GPIOB
#define I2C1_SCL 6
#define I2C1_SDA 7

#define LOGI_EXTI_PORT GPIOB
#define LOGI_EXTI_PAD  0

#define LOGI_BIT0_PORT GPIOB
#define LOGI_BIT0_PAD  1

#define LOGI_BIT1_PORT GPIOB
#define LOGI_BIT1_PAD  2

#define LOGI_BIT2_PORT GPIOB
#define LOGI_BIT2_PAD  3

#define LOGI_BIT3_PORT GPIOB
#define LOGI_BIT3_PAD  4
