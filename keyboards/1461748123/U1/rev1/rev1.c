#include "rev1.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "quantum.h"
#include "timer.h"
#include "wait.h"
#include "print.h"
#include "matrix.h"
#include "ch.h"
#include "hal.h"
#include "action_util.h"
#include "i2c_master.h"
#include "config.h"
#include "expander.h"

#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))

volatile uint8_t LOGI_BIT0 = 0;
volatile uint8_t LOGI_BIT1 = 0;
volatile uint8_t LOGI_BIT2 = 0;
volatile uint8_t LOGI_BIT3 = 0;

volatile uint8_t scan_line = 0;

static void exit_cb(void* unused);

/*
    K375s matrix lookup table
    Index range: 0-15, 255 for unavaliable keys.
    CHANGE ME: CURRENTLY ITS 1-INDEXED! chaneg to 0 index when have time otherwise code won't work
*/
static const LOGI_KEYPAIR LOGI_MATRIX[232] = {{255, 255}, {255, 255}, {255, 255}, {255, 255}, {1, 15},    {4, 23},    {11, 14},   {4, 19},    {6, 17},    {2, 15},    {10, 14},   {5, 21},    {9, 15},    {6, 21},    {7, 21},    {5, 19},    {5, 22},    {8, 17},    {3, 20},    {12, 15},   {3, 23},    {5, 15},    {3, 21},    {4, 16},    {255, 255}, {3, 18},    {7, 17},    {5, 23},    {4, 18},    {10, 15},   {6, 16},    {7, 16},    {4, 14},    {8, 21},    {7, 23},    {3, 22},    {8, 15},    {4, 20},    {6, 22},    {6, 20},    {3, 19},    {7, 22},    {3, 17},    {4, 22},    {13, 14},   {7, 20},    {7, 14},    {3, 15},    {6, 19},    {5, 20},    {255, 255}, {7, 15},    {255, 255}, {8, 23},    {4, 17},    {6, 15},    {5, 18},    {4, 15},    {5, 16},    {5, 17},    {11, 15},   {6, 14},    {12, 14},   {3, 14},    {2, 16},    {5, 14},    {8, 14},    {3, 16},    {6, 23},    {7, 18},    {12, 16}, {11, 16},   {10, 16},   {6, 18},    {2, 23},    {2, 22},    {4, 21},    {9, 23},
                                              {1, 22},    {8, 16},    {8, 18},    {8, 20},    {8, 22},    {9, 14},    {2, 19},    {2, 21},    {2, 20},    {1, 19},    {1, 16},    {2, 17},    {9, 22},    {9, 21},    {13, 15},   {1, 14},    {9, 20},    {1, 20},    {2, 14},    {7, 19},    {1, 17},    {1, 21},    {1, 18},    {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {9, 18},    {255, 255}, {9, 17},    {10, 21},   {10, 22},   {255, 255}, {255, 255}, {255, 255}, {255, 255}, {10, 18},   {255, 255}, {255, 255}, {255, 255}, {1, 23},  {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255},
                                              {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {255, 255}, {9, 19},    {13, 23},   {12, 21}, {10, 17},   {11, 18},   {13, 22},   {12, 20},   {255, 255}};

static void enable_exti_events(void) {
    palDisablePadEventI(LOGI_EXTI_PORT, LOGI_EXTI_PAD);
    palEnablePadEventI(LOGI_EXTI_PORT, LOGI_EXTI_PAD, PAL_EVENT_MODE_RISING_EDGE);
    palSetPadCallbackI(LOGI_EXTI_PORT, LOGI_EXTI_PAD, &exit_cb, 0);
}

static void exit_cb(void* unused) {
    (void)unused;
    chSysLockFromISR();

    LOGI_BIT0 = palReadPad(LOGI_BIT0_PORT, LOGI_BIT0_PAD);
    LOGI_BIT1 = palReadPad(LOGI_BIT1_PORT, LOGI_BIT1_PAD);
    LOGI_BIT2 = palReadPad(LOGI_BIT2_PORT, LOGI_BIT2_PAD);
    LOGI_BIT3 = palReadPad(LOGI_BIT3_PORT, LOGI_BIT3_PAD);
    scan_line = (LOGI_BIT3 << 3) | (LOGI_BIT2 << 2) | (LOGI_BIT1 << 1) | LOGI_BIT0;

    uint16_t expander_gpio = 0x00;
    uint8_t expander_gpioa = 0x00;
    uint8_t expander_gpiob = 0x00;


    /*
        Check modifier keys

        Bit 0 (0x01): Left Control
        Bit 1 (0x02): Left Shift
        Bit 2 (0x04): Left Alt
        Bit 3 (0x08): Left Window
        Bit 4 (0x10): Right Control
        Bit 5 (0x20): Right Shift
        Bit 6 (0x40): Right Alt
        Bit 7 (0x80): Right Window
    */
    if (scan_line == (CHECK_BIT(keyboard_report->mods, 0) ? LOGI_MATRIX[0xE0] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[0xE0].ROW);

    if (scan_line == (CHECK_BIT(keyboard_report->mods, 1) ? LOGI_MATRIX[0xE1] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[0xE1].ROW);

    if (scan_line == (CHECK_BIT(keyboard_report->mods, 2) ? LOGI_MATRIX[0xE2] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[0xE2].ROW);

    if (scan_line == (CHECK_BIT(keyboard_report->mods, 3) ? LOGI_MATRIX[0xE3] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[0xE3].ROW);

    if (scan_line == (CHECK_BIT(keyboard_report->mods, 4) ? LOGI_MATRIX[0xE4] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[0xE4].ROW);

    if (scan_line == (CHECK_BIT(keyboard_report->mods, 5) ? LOGI_MATRIX[0xE5] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[0xE5].ROW);

    if (scan_line == (CHECK_BIT(keyboard_report->mods, 6) ? LOGI_MATRIX[0xE6] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[0xE6].ROW);

    if (scan_line == (CHECK_BIT(keyboard_report->mods, 7) ? LOGI_MATRIX[0xE7] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[0xE7].ROW);

    /*
        Check key report
    */
    if (scan_line == (keyboard_report->keys[0] ? LOGI_MATRIX[keyboard_report->keys[0]] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[keyboard_report->keys[0]].ROW);

    if (scan_line == (keyboard_report->keys[1] ? LOGI_MATRIX[keyboard_report->keys[1]] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[keyboard_report->keys[1]].ROW);

    if (scan_line == (keyboard_report->keys[2] ? LOGI_MATRIX[keyboard_report->keys[2]] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[keyboard_report->keys[2]].ROW);

    if (scan_line == (keyboard_report->keys[3] ? LOGI_MATRIX[keyboard_report->keys[3]] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[keyboard_report->keys[3]].ROW);

    if (scan_line == (keyboard_report->keys[4] ? LOGI_MATRIX[keyboard_report->keys[4]] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[keyboard_report->keys[4]].ROW);

    if (scan_line == (keyboard_report->keys[5] ? LOGI_MATRIX[keyboard_report->keys[5]] : LOGI_MATRIX[0x00]).COL)
        expander_gpio |= (0x01 << LOGI_MATRIX[keyboard_report->keys[5]].ROW);


    //enable_exti_events(); //Disable further interrupts that might occur on same button press.
    expander_gpioa = expander_gpio;
    expander_gpiob = expander_gpio >> 8;

    i2c_start(EXPANDER_ADDR);
    expander_write(EXPANDER_REG_GPIOA, expander_gpioa);
    expander_write(EXPANDER_REG_GPIOB, expander_gpiob);
    chThdSleepMicroseconds(10);

    expander_write(EXPANDER_REG_GPIOA, 0x00);
    expander_write(EXPANDER_REG_GPIOB, 0x00);
    i2c_stop();

    chSysUnlockFromISR();
}

void keyboard_post_init_kb(void) {
    expander_init();
}

void matrix_init_kb(void) {
    setPinInput(A12);
    setPinInput(A13);
    setPinInput(A14);
    setPinInput(A15);

    osalSysLock();
    enable_exti_events();
    osalSysUnlock();
}

void matrix_scan_kb(void) {
    //update_key_lookup();
}
