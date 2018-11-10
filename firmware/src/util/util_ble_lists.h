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

typedef uint8_t nlindex_t;

#define	NEIGHBOR_LIST_SIZE	255	// must fit in nlindex_t and not use NEIGHBOR_NONE
#define	NEIGHBOR_NONE	255		// Unused entry in sorted_index

#define NEIGHBOR_FILTER_NONE	0xFF			// any valid neighbor will do
#define NEIGHBOR_FILTER_MM		BLE_DATA_FLAGS_MASK_MM

// Function to initialize the data structures.
extern void ble_lists_init(void);

// Update the neighbor list based on a received BLE advertisement
extern void ble_lists_process_advertisement(uint8_t *ble_address,
											char *name,
											uint16_t appearance,
											uint16_t mfg_code,
											uint8_t flags,
											int8_t rssi);

// Create a sorted list of all the nearby badges we've heard since powerup.
// Return the count.
extern int survey_and_sort_neighbors(uint8_t filter_flags);

// Drawing function callback from menu handler for neighbor list menus
extern void ble_lists_draw_callback(nlindex_t itemno, uint16_t x, uint16_t y, uint8_t menu_draw_method);

// Get various parameters for a selected neighbor.
extern void ble_lists_get_neighbor_address(uint8_t index, uint8_t *buf);
extern int8_t ble_lists_get_neighbor_rssi(uint8_t index);
extern char *ble_lists_get_neighbor_name(uint8_t index);

// Determine whether a selected neighbor plays the QSO game
extern bool neighbor_allows_qso_game(uint8_t index);

// Generate a text report about a selected neighbor.
// Returns number of characters added to buf.
extern int neighbor_get_info(uint8_t index, char *name, char *buf);

// Find a suitable neighbor for sending a Hello to.
// Returns true if it found one.
extern bool ble_lists_choose_hello_neighbor(uint16_t *hello_company_id, char *hello_name);

// Randomly mark a neighbor as un-Helloed
extern void ble_lists_randomly_unhello_neighbor(void);

#endif /* UTIL_BLE_LISTS_H_ */
