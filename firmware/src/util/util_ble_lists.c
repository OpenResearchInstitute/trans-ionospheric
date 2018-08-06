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

#define	NLFLAGS_EMPTY	0x00	// means this record is empty

#define NLFLAGS_VALID	0x80	// set for all non-empty records
								// Must be different from any flags in advertisements

// The rest of the flags are identical to flags in advertisements.
#define NLFLAGS_UPDATE_MASK		0x7F
#define NLFLAGS_GAMES_ACCEPTED	BLE_DATA_FLAGS_MASK_GAMES
#define NLFLAGS_QSO_GAME		BLE_DATA_FLAGS_MASK_QSO
#define NLFLAGS_MM_GAME			BLE_DATA_FLAGS_MASK_MM

#define NLFLAGS_DEFAULT	(NLFLAGS_VALID)	// start with these flags for new entries

#define NEIGHBOR_REUSE_AGE	(3*60000L)	// 3 minutes in milliseconds

#define MAX_SCREEN_LINES	9

#define SUBMENU_PADDING		2
#define SUBMENU_TITLE_SIZE	15

typedef struct {
	uint8_t		flags;			// all zero means this entry is empty
								// This is first for premature optimization purposes
	uint8_t 	ble_address[BLE_GAP_ADDR_LEN];
	char	 	name[SETTING_NAME_LENGTH];
	int8_t		rssi;
	uint32_t	last_heard_millis;
	uint16_t	company_id;
} ble_lists_neighborlist_t;

typedef struct {
	nlindex_t	itemno;
	char		name[SETTING_NAME_LENGTH];
	int8_t		rssi;
} ble_lists_displayed_info_t;

static ble_lists_neighborlist_t neighbor_list[NEIGHBOR_LIST_SIZE];
static nlindex_t sorted_index[NEIGHBOR_LIST_SIZE];
static ble_lists_displayed_info_t displayed[MAX_SCREEN_LINES];
static bool updates_frozen = false;


// Initialize the neighbor list system. Start with no neighbors.
void ble_lists_init(void) {
	memset(neighbor_list, 0, sizeof(neighbor_list));
	memset(sorted_index, NEIGHBOR_NONE, sizeof(sorted_index));
	for (uint8_t i = 0; i < MAX_SCREEN_LINES; i++) {
		displayed[i].itemno = NEIGHBOR_NONE;
	}
}


// Comparison function used for sorting the neighbor list.
// a and b are pointers to entries in the sorted_index.
// The entries in the sorted_index are indexes into neighbor_list.
// The entries in neighbor_list contain an integer field named rssi.
// We compare the values of rssi.
static int compare_neighbor_rssi(const void *a, const void *b) {

	int8_t a_rssi = neighbor_list[*(const nlindex_t *)a].rssi;
	int8_t b_rssi = neighbor_list[*(const nlindex_t *)b].rssi;

	if (a_rssi > b_rssi) {
		return -1;
	} else if (a_rssi < b_rssi) {
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

	updates_frozen = true;

	// Scan the neighbor list and write down the indices of the neighbors.
	for (int i=0; i < NEIGHBOR_LIST_SIZE; i++) {
		if (neighbor_list[i].flags != NLFLAGS_EMPTY) {
			sorted_index[count++] = i;
		}
	}

	// No need to write the rest of sorted_index. Since the neighbor_list
	// never shrinks, the rest of the index still contains NEIGHBOR_NONE.

	// Now sort the entries according to RSSI
	if (count > 0) {
		// Sort only the full part of sorted_index.
		qsort(sorted_index, count, 1, compare_neighbor_rssi);
	}

	updates_frozen = false;

	// Invalidate our saved info about what's on the screen
	for (uint8_t i = 0; i < MAX_SCREEN_LINES; i++) {
		displayed[i].itemno = NEIGHBOR_NONE;
	}

	return count;
}


// Drawing function callback from menu handler for neighbor list menus
void ble_lists_draw_callback(nlindex_t itemno, uint16_t x, uint16_t y, uint8_t menu_draw_method) {
	char title[22];

	switch (menu_draw_method) {
		case MENU_DRAW_UPDATES:
			if (displayed[itemno % MAX_SCREEN_LINES].itemno == itemno) {
				util_gfx_set_font(FONT_SMALL);
				util_gfx_set_color(COLOR_WHITE);
				uint8_t font_width = util_gfx_font_width();
				uint8_t font_height = util_gfx_font_height();

				if (0 != strcmp(displayed[itemno % MAX_SCREEN_LINES].name,
								neighbor_list[sorted_index[itemno]].name)) {
					util_gfx_fill_rect(SUBMENU_PADDING + 4 * font_width,
									   y,
									   (SETTING_NAME_LENGTH - 1) * font_width,
									   font_height,
									   COLOR_BLACK);
					util_gfx_set_cursor(x + (4 * font_width), y);
					sprintf(title, "%-*s", SETTING_NAME_LENGTH-1,
										   neighbor_list[sorted_index[itemno]].name);
					util_gfx_print(title);
					strcpy(displayed[itemno % MAX_SCREEN_LINES].name, neighbor_list[sorted_index[itemno]].name);
				}

				if (displayed[itemno % MAX_SCREEN_LINES].rssi != neighbor_list[sorted_index[itemno]].rssi) {
					util_gfx_fill_rect(SUBMENU_PADDING + (4 + SETTING_NAME_LENGTH-1) * font_width,
									   y,
									   4 * font_width,
									   font_height,
									   COLOR_BLACK);
					util_gfx_set_cursor(x + ((4 + SETTING_NAME_LENGTH-1) * font_width), y);
					sprintf(title, "%4d", neighbor_list[sorted_index[itemno]].rssi);
					util_gfx_print(title);
					displayed[itemno % MAX_SCREEN_LINES].rssi = neighbor_list[sorted_index[itemno]].rssi;
				}

				return;
			}
		// We didn't have saved info on the screen contents. Treat as DRAW_EVERYTHING.
		// FALLTHROUGH

		case MENU_DRAW_EVERYTHING:
			util_gfx_set_font(FONT_SMALL);
			util_gfx_set_color(COLOR_WHITE);
			util_gfx_set_cursor(x, y);

			sprintf(title, "%3d %-*s%4d", itemno+1, SETTING_NAME_LENGTH-1,
										neighbor_list[sorted_index[itemno]].name,
										neighbor_list[sorted_index[itemno]].rssi);
			util_gfx_print(title);

			// update our saved copy of what's displayed on screen
			displayed[itemno % MAX_SCREEN_LINES].itemno = itemno;
			strcpy(displayed[itemno % MAX_SCREEN_LINES].name, neighbor_list[sorted_index[itemno]].name);
			displayed[itemno % MAX_SCREEN_LINES].rssi = neighbor_list[sorted_index[itemno]].rssi;

			break;

		default:
			mbp_ui_error("Bad redraw");
			break;
	}
}


// Get the BLE address for a selected neighbor.
void ble_lists_get_neighbor_address(uint8_t itemno, uint8_t *buf) {
	memcpy(buf, neighbor_list[sorted_index[itemno]].ble_address, BLE_GAP_ADDR_LEN);
}


// Utility function to compare two GAP addresses for equality
static bool __ble_address_equal(uint8_t *a, uint8_t *b) {
	for (uint8_t i = 0; i < BLE_GAP_ADDR_LEN; i++) {
		if (a[i] != b[i]) {
			return false;
		}
	}

	return true;
}


// Update the neighbor list based on a received BLE advertisement.
// We are guaranteed that this is a badge with a valid badgestdcom appearance.
void ble_lists_process_advertisement(uint8_t *ble_address,
									 char *name,
									 uint16_t appearance,
									 uint16_t company_id,
									 uint8_t flags,
									 int8_t rssi) {

	// In the case where there are very many badges nearby, the neighbor list
	// can fill up. When that happens, we can overwrite the oldest badges on
	// the neighbor list, but we don't want this to happen continuously so that
	// badges are constantly rotating on and off the list. So, when we have to
	// look for an oldest neighbor, and it's too new to expire yet, we write
	// down the future time when we might next be able to expire a neighbor.
	// This will save us rescanning the list for elderly neighbors when we
	// already know it's too soon to expire any.
	static uint32_t neighbor_list_next_expiration = 0;


	// If the list is being sorted (for example) we can't safely update it,
	// so just discard advertisements until that's all done.
	if (updates_frozen) {
		return;		// discard this one, sorry!
	}

	// We'll need the current time.
	uint32_t timenow = util_millis();

	// neighbor_list is a trivial hash table, using one byte of the BLE address
	// as the hash. We start by using the hash as the index. If that's not a
	// matching entry or an empty entry, we proceed linearly through the array
	// until we find one that is, or come back to the starting point.
	// (Adjust this if nlindex_t ever changes from uint8_t!)
	nlindex_t initial_index = ble_address[0];
	if (initial_index >= NEIGHBOR_LIST_SIZE) {
		initial_index = 0;	// don't use NEIGHBOR_NONE (255), just skip ahead to 0.
	}

	// Now we'll scan for the place in neighbor_list where this badge will be recorded.
	nlindex_t index = initial_index;
	while (true) {
		if (neighbor_list[index].flags == NLFLAGS_EMPTY) {
			// We found an empty entry before we found a match. Put it here.
			memcpy(neighbor_list[index].ble_address, ble_address, BLE_GAP_ADDR_LEN);
			neighbor_list[index].flags = NLFLAGS_DEFAULT;
			break;
		} else if (__ble_address_equal(ble_address, neighbor_list[index].ble_address)) {
			// We found the matching entry.
			break;
		}

		// increment the index, wrapping around
		if (++index == NEIGHBOR_LIST_SIZE) {
			index = 0;
		}

		// check if we've looked at every entry without finding a place
		if (index == initial_index) {
			// Uh oh, the list is full.
			if (neighbor_list_next_expiration > timenow) {
				// We're waiting for the oldest neighbor to reach a certain age.
				// Until that happens, we won't overwrite any neighbors.
				// (This is a performance hack for the list-full case.)
				return;		// discard this one, sorry!
			}

			// Now look for the oldest entry, so we might overwrite it
			uint32_t oldest = UINT32_MAX;
			nlindex_t oldest_index;
			for (nlindex_t i = 0; i < NEIGHBOR_LIST_SIZE; i++) {
				if (neighbor_list[i].last_heard_millis < oldest) {
					oldest = neighbor_list[i].last_heard_millis;
					oldest_index = i;
				}
			}
			// See if the oldest one is too new to expire and re-use.
			if (oldest + NEIGHBOR_REUSE_AGE > timenow) {
				// Too new. Write down when we can next consider re-use.
				neighbor_list_next_expiration = oldest + NEIGHBOR_REUSE_AGE;
				return;		// discard this one, sorry!
			}

			// OK, let's go ahead and re-use this neighbor list entry
			index = oldest_index;
			memcpy(neighbor_list[index].ble_address, ble_address, BLE_GAP_ADDR_LEN);
			neighbor_list[index].flags = NLFLAGS_DEFAULT;
			break;
		}
	}

	// We've found where to store the badge info. Store it.
	neighbor_list[index].rssi = rssi;
	strncpy(neighbor_list[index].name, name, SETTING_NAME_LENGTH);
	neighbor_list[index].last_heard_millis = timenow;
	neighbor_list[index].flags = (neighbor_list[index].flags & ~NLFLAGS_UPDATE_MASK)
							   | (flags & NLFLAGS_UPDATE_MASK);
	neighbor_list[index].company_id = company_id;
}


// Determine whether a selected neighbor plays the QSO game
bool neighbor_allows_qso_game(uint8_t index) {
	ble_lists_neighborlist_t neighbor = neighbor_list[sorted_index[index]];

	if (   (neighbor.flags & NLFLAGS_GAMES_ACCEPTED)
		&& (neighbor.flags & NLFLAGS_QSO_GAME)) {
		return true;
	} else {
		return false;
	}
}


// Generate a text report about a selected neighbor.
// Returns number of characters added to buf.
int neighbor_get_info(uint8_t index, char *name, char *buf) {
	ble_lists_neighborlist_t neighbor = neighbor_list[sorted_index[index]];
	char when[20];
	uint32_t how_long = util_millis() - neighbor.last_heard_millis;

	strcpy(name, neighbor.name);

	if (how_long < 20000) {
		sprintf(when, "Heard just now");
	} else if (how_long < 120000) {
		sprintf(when, "%ld secs ago", how_long/1000);
	} else if (how_long < 60000L*120) {
		sprintf(when, "%ld mins ago", how_long/60000);
	} else {
		sprintf(when, "%ld hours ago", how_long/(60000L*60));
	}

	return sprintf(buf, "%s\nBLE:%02x%02x%02x%02x%02x%02x\nRSSI:   %-4ddBm\n%s\n",
		util_ble_company_id_to_string(neighbor.company_id),
		neighbor.ble_address[0],
		neighbor.ble_address[1],
		neighbor.ble_address[2],
		neighbor.ble_address[3],
		neighbor.ble_address[4],
		neighbor.ble_address[5],
		neighbor.rssi,
		when
	);
}
