#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "hal.h"
#include "timer.h"
#include "wait.h"
#include "printf.h"
#include "backlight.h"
#include "matrix.h"
#include "action.h"
#include "keycode.h"
#include <string.h>

/*
 *     COL_PINS { A10, B2, B11, A0, A1, A2, B5 }
 *     ROW_PINS { A3, B10, A9, A8, A6, A7, B0 }
 */
/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_COLS];
static bool debouncing = false;
static uint16_t debouncing_time = 0;

static uint8_t encoder_state = 0;
static int8_t encoder_value = 0;
static int8_t encoder_LUT[] = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0 };

__attribute__ ((weak))
void matrix_init_user(void) {}

__attribute__ ((weak))
void matrix_scan_user(void) {}

__attribute__ ((weak))
void matrix_init_kb(void) {
  matrix_init_user();
}

__attribute__ ((weak))
void matrix_scan_kb(void) {
  matrix_scan_user();
}

void matrix_init(void) {
    printf("matrix init\n");
    //debug_matrix = true;

    // encoder setup
    palSetPadMode(GPIOB, 12, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(GPIOB, 13, PAL_MODE_INPUT_PULLUP);

    encoder_state = (palReadPad(GPIOB, 12) << 0) | (palReadPad(GPIOB, 13) << 1);

    // actual matrix setup
    //col
    palSetPadMode(GPIOA, 10, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOB, 2, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOB, 11,  PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, 0,  PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, 1,  PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, 2,  PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOB, 5,  PAL_MODE_OUTPUT_PUSHPULL);
    //row
    palSetPadMode(GPIOA, 3, PAL_MODE_INPUT_PULLDOWN);
    palSetPadMode(GPIOB, 10,  PAL_MODE_INPUT_PULLDOWN);
    palSetPadMode(GPIOA, 9,  PAL_MODE_INPUT_PULLDOWN);
    palSetPadMode(GPIOA, 8, PAL_MODE_INPUT_PULLDOWN);
    palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_PULLDOWN);
    palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_PULLDOWN);
    palSetPadMode(GPIOB, 0, PAL_MODE_INPUT_PULLDOWN);


    memset(matrix, 0, MATRIX_ROWS * sizeof(matrix_row_t));
    memset(matrix_debouncing, 0, MATRIX_COLS * sizeof(matrix_row_t));


    matrix_init_quantum();
}

__attribute__ ((weak))
void encoder_update(bool clockwise) { }

bool last_dip_switch[4] = {0};

#ifndef ENCODER_RESOLUTION
  #define ENCODER_RESOLUTION 4
#endif

uint8_t matrix_scan(void) {

    // encoder on B12 and B13
    encoder_state <<= 2;
    encoder_state |= (palReadPad(GPIOB, 12) << 0) | (palReadPad(GPIOB, 13) << 1);
    encoder_value += encoder_LUT[encoder_state & 0xF];
    if (encoder_value >= ENCODER_RESOLUTION) {
        encoder_update(0);
    }
    if (encoder_value <= -ENCODER_RESOLUTION) { // direction is arbitrary here, but this clockwise
        encoder_update(1);
    }
    encoder_value %= ENCODER_RESOLUTION;

    // actual matrix
    for (int col = 0; col < MATRIX_COLS; col++) {
        matrix_row_t data = 0;

        // strobe col { A10, B2, B11, A0, A1, A2, B5 }
        switch (col) {
            case 0: palSetPad(GPIOA, 10); break;
            case 1: palSetPad(GPIOB, 2); break;
            case 2: palSetPad(GPIOB, 11); break;
            case 3: palSetPad(GPIOA, 0); break;
            case 4: palSetPad(GPIOA, 1); break;
            case 5: palSetPad(GPIOA, 2); break;
            case 6: palSetPad(GPIOA, 5); break;
        }

        // need wait to settle pin state
        wait_us(20);

        // read row data { A3, B10, A9, A8, A6, A7, B0 }
        data = (
            (palReadPad(GPIOA, 3) << 0 ) |
            (palReadPad(GPIOB, 10)  << 1 ) |
            (palReadPad(GPIOA, 9)  << 2 ) |
            (palReadPad(GPIOA, 8) << 3 ) |
            (palReadPad(GPIOA, 6) << 4 ) |
            (palReadPad(GPIOA, 7) << 5 ) |
            (palReadPad(GPIOB, 0) << 6 )
        );

        // unstrobe  col { A10, B2, B11, A0, A1, A2, B5 }
        switch (col) {
            case 0: palClearPad(GPIOA, 10); break;
            case 1: palClearPad(GPIOB, 2); break;
            case 2: palClearPad(GPIOB, 11); break;
            case 3: palClearPad(GPIOA, 0); break;
            case 4: palClearPad(GPIOA, 1); break;
            case 5: palClearPad(GPIOA, 2); break;
            case 6: palClearPad(GPIOB, 5); break;
        }

        if (matrix_debouncing[col] != data) {
            matrix_debouncing[col] = data;
            debouncing = true;
            debouncing_time = timer_read();
        }
    }

    if (debouncing && timer_elapsed(debouncing_time) > DEBOUNCE) {
        for (int row = 0; row < MATRIX_ROWS; row++) {
            matrix[row] = 0;
            for (int col = 0; col < MATRIX_COLS; col++) {
                matrix[row] |= ((matrix_debouncing[col] & (1 << row) ? 1 : 0) << col);
            }
        }
        debouncing = false;
    }

    matrix_scan_quantum();

    return 1;
}

bool matrix_is_on(uint8_t row, uint8_t col) {
    return (matrix[row] & (1<<col));
}

matrix_row_t matrix_get_row(uint8_t row) {
    return matrix[row];
}

void matrix_print(void) {
    printf("\nr/c 01234567\n");
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        printf("%X0: ", row);
        matrix_row_t data = matrix_get_row(row);
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (data & (1<<col))
                printf("1");
            else
                printf("0");
        }
        printf("\n");
    }
}
