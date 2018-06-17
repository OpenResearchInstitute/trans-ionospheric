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

// There were once two lists, but the so-called "active" list has been
// removed.
//
// The Seen list is fixed size LIFO. It holds all non-joco badges that we see,
// plus a cache of joco badges we've recently seen which we've already counted
// as visited for scoring.


void ble_lists_init();

//
// Seen list
//
#define SEEN_FLAG_MASK       0xF0
#define SEEN_FLAG_VISITED    0x10
#define SEEN_FLAG_SAID_HELLO 0x20
//#define SEEN_FLAG_FUTURE_USE 0x40
#define SEEN_FLAG_USED       0x80

#define SEEN_TYPE_MASK 0x0F
#define SEEN_TYPE_JOCO 0x01
#define SEEN_TYPE_PEER 0x02

uint8_t check_and_add_to_seen(uint8_t *address, uint16_t device_id, char *name, uint8_t type);
int set_seen_flags(uint8_t *address, uint16_t device_id, uint8_t flags);


//
// Prepare list of badges for the "Nearby badges" display
//
typedef struct {
    char text[20];
} ble_badge_list_menu_text_t;

// the number of badges we fetch to display in teh nearby menu option
#define NEARBY_BADGE_LIST_LEN 38

extern int get_nearby_badge_list(int size, ble_badge_list_menu_text_t *list); 
extern int get_nearby_badge_count(); 

#endif /* UTIL_BLE_LISTS_H_ */
