/*****************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 * 	@sconklin
 * 	@mustbeart
 * 	@abraxas3d
 *****************************************************************************/
#ifndef UTIL_BLE_LISTS_H_
#define UTIL_BLE_LISTS_H_


// Function to initialize the data structures.
extern void ble_lists_init(void);

// Update the neighbor list based on a received BLE advertisement
extern void ble_lists_process_advertisement(uint8_t *ble_address, char *name, uint16_t appearance, uint16_t mfg_code);

// Create a sorted list of all the nearby badges we've heard since powerup.
// Return the count.
extern int survey_and_sort_neighbors(void);

// Drawing function callback from menu handler for neighbor list menus
extern void ble_lists_draw_callback(uint8_t itemno, uint16_t x, uint16_t y, uint8_t menu_draw_method);

#endif /* UTIL_BLE_LISTS_H_ */
