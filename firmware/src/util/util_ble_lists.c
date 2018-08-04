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
#include "../system.h"

typedef struct {
	uint8_t 	ble_address[6];
	uint8_t 	name[SETTING_NAME_LENGTH];
	int8_t		rssi;
	uint32_t	last_heard_millis;
	uint8_t		flags;
} ble_lists_neighborlist_t;

#define	NEIGHBOR_LIST_SIZE	256
static ble_lists_neighborlist_t neighbor_list[NEIGHBOR_LIST_SIZE];
static uint8_t sorted_index[NEIGHBOR_LIST_SIZE];

#define	NEIGHBOR_NONE	255		// Unused entry in sorted_index

// Initialize the neighbor list system. Start with no neighbors.
void ble_lists_init(void) {
	memset(neighbor_list, 0, sizeof(neighbor_list));
	memset(sorted_index, NEIGHBOR_NONE, sizeof(sorted_index));
}


// Comparison function used for sorting the neighbor list.
// a and b are pointers to entries in the sorted_index.
// The entries in the sorted_index are indexes into neighbor_list.
// The entries in neighbor_list contain an integer field named rssi.
// We compare the values of rssi.
static int compare_neighbor_rssi(const void *a, const void *b) {

	int8_t a_rssi = neighbor_list[*(const uint8_t *)a].rssi;
	int8_t b_rssi = neighbor_list[*(const uint8_t *)b].rssi;

	if (a_rssi < b_rssi) {
		return -1;
	} else if (a_rssi > b_rssi) {
		return 1;
	} else {
		return 0;
	}
}


// Resort the neighbor list. This consists of populating the sorted_index
// with the indexes of all the valid neighbors, and then sorting them by
// RSSI.
int survey_and_sort_neighbors(void) {
	uint8_t count = 0;

	// Scan the neighbor list and write down the indices of the neighbors.
	for (int i=0; i < NEIGHBOR_LIST_SIZE; i++) {
		// Only look at ones with nonzero BLE addresses
		for (int j=0; j < 6; j++) {
			if (neighbor_list[i].ble_address[j] != 0x00) {
				sorted_index[count++] = i;
				break;
			}
		}
	}

	// No need to write the rest of sorted_index. Since the neighbor_list
	// never shrinks, the rest of the index still contains NEIGHBOR_NONE.

	// Now sort the entries according to RSSI
	if (count > 0) {
		// Sort only the full part of sorted_index.
		qsort(sorted_index, count, 1, compare_neighbor_rssi);
	}

	return count;
}


// Drawing function callback from menu handler for neighbor list menus
void ble_lists_draw_callback(uint8_t itemno, uint16_t x, uint16_t y, uint8_t menu_draw_method) {
//!!! write me
}


// Update the neighbor list based on a received BLE advertisement
void ble_lists_process_advertisement(uint8_t *ble_address,
									 char *name,
									 uint16_t appearance,
									 uint16_t mfg_code) {

//!!! write me

}
