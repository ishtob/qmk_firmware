/* Copyright 2018 Jack Humbert
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BM6.h"

void matrix_init_kb(void) {
	matrix_init_user();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
  return process_record_user(keycode, record);
}

void matrix_scan_kb(void) {
  matrix_scan_user();
}

// void suspend_power_down_kb(void) {
//   rgb_matrix_set_suspend_state(true);
// }

// void suspend_wakeup_init_kb(void) {
//   rgb_matrix_set_suspend_state(false);
// }

const uint8_t music_map[MATRIX_ROWS][MATRIX_COLS] = LAYOUT(
    3, 4, 5,
    0, 1, 2
);

 const is31_led g_is31_leds[DRIVER_LED_TOTAL] = {
// /* Refer to IS31 manual for these locations
//  *   driver
//  *   |  R location
//  *   |  |      G location
//  *   |  |      |      B location
//  *   |  |      |      | */
       {1, C9_9,  C8_9,  C7_9},
       {1, C9_10, C8_10, C7_10},
       {1, C9_11, C8_11, C7_11},
       {1, C9_12, C8_12, C7_12},
       {1, C9_13, C8_13, C7_13},
       {1, C9_14, C8_14, C7_14}
 };

 const rgb_led g_rgb_leds[DRIVER_LED_TOTAL] = {

//     {row | col << 4}
//       |             {x=0..224, y=0..64}
//       |              |                 modifier
//       |              |                 |

    {{2|(9<<4)},   {20.36*9, 21.33*2}, 0},
    {{2|(10<<4)},  {20.36*10,21.33*2}, 0},
    {{2|(11<<4)},  {20.36*11,21.33*2}, 1},
    {{3|(9<<4)},   {20.36*9, 21.33*3}, 1},
    {{3|(10<<4)},  {20.36*10,21.33*3}, 1},
    {{3|(11<<4)},  {20.36*11,21.33*3}, 1}
 };
