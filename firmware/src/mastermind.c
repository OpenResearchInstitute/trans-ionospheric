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
 *  @sconklin
 *  @mustbeart
 *  @abraxas3d
 *****************************************************************************/

#include "system.h"

static ble_badge_list_menu_text_t m_opponent;

static void __set_opponent(void *p_data) {
	memcpy(m_opponent.text, ((ble_badge_list_menu_text_t *)p_data)->text, 20);
}

static bool mbp_mastermind_players() {
	int badge_list_size;
	ble_badge_list_menu_text_t *list;
	uint8_t status;

	menu_t menu;
	menu_item_t items[NEARBY_BADGE_LIST_LEN];
	menu.items = items;
	menu.count = 0;
	menu.title = "Players";

	list = malloc(NEARBY_BADGE_LIST_LEN * sizeof( ble_badge_list_menu_text_t));
	if (!list) {
		mbp_ui_error("players malloc fail.");
		return false;
	}

	badge_list_size = get_nearby_badge_list(NEARBY_BADGE_LIST_LEN, list);

	if (badge_list_size == 0) {
		mbp_ui_popup("BUMMER!", "No other badges are nearby to play with you");
		return false;
	}

	for (uint8_t i = 0; i < badge_list_size; i++) {
		items[menu.count++] = (menu_item_t ) { list[i].text, NULL, NULL, __set_opponent, &list[i].text };
	}

	status = mbp_submenu(&menu);

	free(list);

	if (status == MENU_QUIT) {
		return false;
	} else {
		return true;
	}
}

// Entry point from the Games menu.
void mastermind() {

	// Try to pick an opponent from the available players
	if (mbp_mastermind_players()) {
		char buf[40];

		sprintf(buf, "Invite %s to play?", m_opponent.text);
		uint8_t invite = mbp_ui_toggle_popup("PLAYER", 0, "Invite", "Cancel", buf);

		if (invite == 0) {
			mbp_ui_popup("TRYING", "Inviting player to join the game");
		}
	}
}
