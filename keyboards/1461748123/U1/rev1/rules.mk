# MCU name
MCU = STM32F303

## Features
CONSOLE_ENABLE = yes

QUANTUM_LIB_SRC += i2c_master.c

SRC += expander.c
