#include "i2c_master.h"
#include "expander.h"

void expander_init () {
    i2c_init();
    chThdSleepMicroseconds(500);

    i2c_start(EXPANDER_ADDR);
    // set everything to output
    expander_write(EXPANDER_REG_IODIRA, 0x00);
    expander_write(EXPANDER_REG_IODIRB, 0x00);

    // disable interrupts
    expander_write(EXPANDER_REG_GPINTENA, 0x00);
    expander_write(EXPANDER_REG_GPINTENB, 0x00);

    // polarity
    expander_write(EXPANDER_REG_IPOLA, 0x00);
    expander_write(EXPANDER_REG_IPOLB, 0x00);
    i2c_stop();
}

i2c_status_t expander_write(uint8_t reg, uint8_t data) {
    return i2c_writeReg(EXPANDER_ADDR, reg, &data, sizeof(data), EXPANDER_TIMEOUT);
}
