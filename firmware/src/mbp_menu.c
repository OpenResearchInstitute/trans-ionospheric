/*****************************************************************************
 * (C) Copyright 2017 AND!XOR LLC (http://andnxor.com/).
 * (C) Copyright 2018 Open Research Institute (http://openresearch.institute).
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
 * 	@andnxor
 * 	@zappbrandnxor
 * 	@hyr0n1
 * 	@andrewnriley
 * 	@lacosteaef
 * 	@bitstr3m
 *
 * Further modifications made by
 *      @sconklin
 *      @mustbeart
 *
 *****************************************************************************/
#include "system.h"

//New menu defines
#define MAX_ITEMS			4
#define MENU_COLOR			COLOR_WHITE
#define MENU_COLOR_BG			COLOR_BLACK
#define MENU_ICON_SIZE			20
#define MENU_INDICATOR_BG		COLOR_BLACK
#define MENU_INDICATOR_FG		COLOR_GREEN
#define MENU_INDICATOR_H		16
#define MENU_PADDING			2
#define MENU_SCROLL_DELAY		200
#define MENU_SELECTED_COLOR		COLOR_WHITE
#define MENU_SIZE			27
#define MENU_SELECTED_SIZE		114

#define SUBMENU_SELECTED_COLOR	COLOR_CYAN
#define SUBMENU_PADDING			2
#define SUBMENU_TITLE_BG		COLOR_DARKCYAN
#define SUBMENU_TITLE_FG		COLOR_WHITE
#define SUBMENU_TITLE_SIZE		15

uint8_t mbp_menu(menu_t *p_menu) {
	p_menu->selected = 0;
	p_menu->top = 0;
	util_gfx_set_font(FONT_LARGE);
	int16_t selected_button = 0;
	uint8_t max_visible_buttons = MIN(p_menu->count, MAX_ITEMS);

	uint32_t size = MENU_ICON_SIZE * MENU_ICON_SIZE * 2;
	uint8_t text_offset = 11;
	uint8_t icons[MAX_ITEMS][size];	// Only the ones on screen now

	//Preload icons for the first screenful of the menu
	for (uint8_t i = 0; i < p_menu->count && i < MAX_ITEMS; i++) {
		if (p_menu->items[i].icon != NULL) {
			util_sd_load_file(p_menu->items[i].icon, icons[i], size);
		}
	}

	while (1) {
		menu_item_t selected = p_menu->items[p_menu->selected];
		util_gfx_cursor_area_reset();

		//Clear the screen
		mbp_ui_cls();

		//Draw preview
		if (selected.preview == NULL) {
                    util_gfx_draw_raw_file("MENU/DEFAULT.PRV", MENU_SIZE, MENU_INDICATOR_H, GFX_WIDTH - MENU_SIZE, GFX_HEIGHT - MENU_INDICATOR_H, NULL, false,
                                           NULL);
		} else {
			util_gfx_draw_raw_file(selected.preview, MENU_SIZE, MENU_INDICATOR_H, GFX_WIDTH - MENU_SIZE, GFX_HEIGHT - MENU_INDICATOR_H, NULL,
			false, NULL);
		}

		//Draw indicator section
		char name[SETTING_NAME_LENGTH];
		mbp_state_name_get(name);
		util_gfx_set_font(FONT_LARGE);
		util_gfx_set_color(MENU_INDICATOR_FG);
		util_gfx_fill_rect(0, 0, GFX_WIDTH, MENU_INDICATOR_H, MENU_INDICATOR_BG);
		util_gfx_set_cursor(0, 5);
		util_gfx_print(name);

		if (mbp_state_airplane_mode_get()) {
			util_gfx_draw_raw_file("MENU/INDPLANE.RAW", 113, 1, 14, 14, NULL, false, NULL);
		}

		//Setup font again to ensure something else hasn't changed it to small
		util_gfx_set_font(FONT_LARGE);
		util_gfx_set_color(COLOR_WHITE);

		//Draws the menu
		for (uint8_t i = 0; i < max_visible_buttons; i++) {

			uint16_t x = 0;
			uint16_t y = MENU_INDICATOR_H + (i * MENU_SIZE) + (i);

			if (i == selected_button) {
				util_gfx_fill_rect(x, y, MENU_SELECTED_SIZE, MENU_SIZE, MENU_COLOR_BG);
				util_gfx_draw_rect(x, y, MENU_SELECTED_SIZE, MENU_SIZE, MENU_SELECTED_COLOR);
				util_gfx_set_cursor(x + (MENU_SIZE - 2) + MENU_PADDING, y + text_offset);
				util_gfx_print(selected.text);
			} else {
				util_gfx_fill_rect(x, y, MENU_SIZE + 1, MENU_SIZE, MENU_COLOR_BG);
				util_gfx_draw_rect(x, y, MENU_SIZE + 1, MENU_SIZE, MENU_COLOR);
			}

			if (p_menu->items[p_menu->top + i].icon != NULL) {
				util_gfx_draw_raw(x + MENU_PADDING + 1, y + MENU_PADDING + 1, MENU_ICON_SIZE, MENU_ICON_SIZE, icons[i]);
			}
		}

		//Track that screen is in a valid state
		util_gfx_validate();

		util_button_wait();

		if (util_button_down()) {
			//Move selected menu item down one if able
			if (p_menu->selected < p_menu->count - 1) {
				p_menu->selected++;

				//Move selected button down one
				if (selected_button < MAX_ITEMS - 1) {
					selected_button++;
				}
				//If already at bottom, scroll
				else {
					p_menu->top++;
					memmove(icons[0], icons[1], size * (MAX_ITEMS-1));
					if (p_menu->items[p_menu->selected].icon != NULL) {
						util_sd_load_file(p_menu->items[p_menu->selected].icon, icons[MAX_ITEMS-1], size);
					}
				}

				nrf_delay_ms(MENU_SCROLL_DELAY);
			}
		} else if (util_button_up()) {
			if (p_menu->selected > 0) {
				p_menu->selected--;

				//Move selection up one
				if (selected_button > 0) {
					selected_button--;
				}
				//If already at top scroll
				else {
					p_menu->top--;
					memmove(icons[1], icons[0], size * (MAX_ITEMS-1));
					if (p_menu->items[p_menu->selected].icon != NULL) {
						util_sd_load_file(p_menu->items[p_menu->selected].icon, icons[0], size);
					}
				}

				nrf_delay_ms(MENU_SCROLL_DELAY);
			}
		} else if (util_button_left()) {
			util_button_clear();
			return MENU_QUIT;
		} else if (util_button_action()) {
			util_button_clear();
			menu_item_t item = p_menu->items[p_menu->selected];
			if (item.callback != NULL) {
				item.callback(item.data);
				util_led_clear();
			}
		}
	}
}

/**
 * Sort menu items using bubble sort.
 * There are better algorithms but this is good enough
 */
void mbp_sort_menu(menu_t *p_menu) {
	for (uint8_t i = 0; i < p_menu->count; i++) {
		for (uint8_t j = 0; j < p_menu->count; j++) {
			if (strcasecmp(p_menu->items[i].text, p_menu->items[j].text) < 0) {
				//swap
				menu_item_t temp = p_menu->items[i];
				p_menu->items[i] = p_menu->items[j];
				p_menu->items[j] = temp;

			}
		}
	}
}

uint8_t mbp_submenu(menu_t *p_menu) {
	p_menu->selected = 0;
	p_menu->top = 0;

	//Setup font again in case something else has changed it
	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_color(COLOR_WHITE);

	uint8_t w = GFX_WIDTH;
	uint8_t font_height = util_gfx_font_height();
	uint8_t max_visible_items = MIN(p_menu->count, (GFX_HEIGHT - SUBMENU_TITLE_SIZE) / font_height);
	uint8_t selected_button = 0;
	bool draw_title = true;
	bool draw_menu_valid[max_visible_items];
	for (uint8_t i = 0; i < max_visible_items; i++) {
		draw_menu_valid[i] = false;
	}

	util_gfx_invalidate();

	while (1) {
		if (!util_gfx_is_valid_state()) {
			draw_title = true;
			mbp_ui_cls();
			draw_title = true;
			for (uint8_t i = 0; i < max_visible_items; i++) {
				draw_menu_valid[i] = false;
			}
		}

		util_gfx_cursor_area_reset();

		if (draw_title) {
			util_gfx_set_font(FONT_LARGE);
			util_gfx_set_cursor(0, 5);
			util_gfx_set_color(COLOR_WHITE);

			util_gfx_fill_rect(0, 0, GFX_WIDTH, SUBMENU_TITLE_SIZE, SUBMENU_TITLE_BG);
			util_gfx_draw_line(0, SUBMENU_TITLE_SIZE, GFX_WIDTH, SUBMENU_TITLE_SIZE, SUBMENU_TITLE_FG);
			util_gfx_print(p_menu->title);

			draw_title = false;
		}

		//Draws the menu
		for (uint8_t i = 0; i < max_visible_items; i++) {
			uint16_t y = (i * font_height) + SUBMENU_PADDING;

			if (!draw_menu_valid[i]) {
				util_gfx_fill_rect(SUBMENU_PADDING - 1, SUBMENU_TITLE_SIZE + y - 1, w - SUBMENU_PADDING, font_height, COLOR_BLACK);

				if (i == selected_button) {
					util_gfx_draw_rect(SUBMENU_PADDING - 1, SUBMENU_TITLE_SIZE + y - 1, w - SUBMENU_PADDING, font_height, SUBMENU_SELECTED_COLOR);
				}

				if (p_menu->items != NULL) {
					util_gfx_set_font(FONT_SMALL);
					util_gfx_set_color(COLOR_WHITE);
					util_gfx_set_cursor(SUBMENU_PADDING, SUBMENU_TITLE_SIZE + y);
					char title[32];
					sprintf(title, "%d) %s", p_menu->top + i + 1, p_menu->items[p_menu->top + i].text);
					util_gfx_print(title);
				} else {
					p_menu->draw_item(p_menu->top + i, SUBMENU_PADDING, SUBMENU_TITLE_SIZE + y, MENU_DRAW_EVERYTHING);
				}

				draw_menu_valid[i] = true;
			} else if (p_menu->items == NULL) {
				p_menu->draw_item(p_menu->top + i, SUBMENU_PADDING, SUBMENU_TITLE_SIZE + y, MENU_DRAW_UPDATES);
			}
		}

		//!!! isn't this redundant?
		//Highlight selected
		util_gfx_draw_rect(
		SUBMENU_PADDING - 1,
		SUBMENU_TITLE_SIZE + (selected_button * font_height) + SUBMENU_PADDING - 1,
				w - SUBMENU_PADDING,
				font_height,
				SUBMENU_SELECTED_COLOR);

		//Validate the gfx state since we're done drawing
		util_gfx_validate();

		//Wait for user interaction
		util_button_wait_timeout(500);

		if (util_button_down()) {
			//Move selected menu item down one if able
			if (p_menu->selected < p_menu->count - 1) {
				p_menu->selected++;

				//Move selected button down one
				if (selected_button < max_visible_items - 1) {
					draw_menu_valid[selected_button] = false;
					selected_button++;
					draw_menu_valid[selected_button] = false;
				}
				//If already at bottom, scroll
				else {
					p_menu->top++;

					for (uint8_t i = 0; i < max_visible_items; i++) {
						draw_menu_valid[i] = false;
					}
				}

				nrf_delay_ms(MENU_SCROLL_DELAY);
			}
		} else if (util_button_up()) {
			if (p_menu->selected > 0) {
				p_menu->selected--;

				//Move selection up one
				if (selected_button > 0) {
					draw_menu_valid[selected_button] = false;
					selected_button--;
					draw_menu_valid[selected_button] = false;
				}
				//If already at top scroll
				else {
					p_menu->top--;

					for (uint8_t i = 0; i < max_visible_items; i++) {
						draw_menu_valid[i] = false;
					}
				}

				nrf_delay_ms(MENU_SCROLL_DELAY);
			} else if (p_menu->items == NULL && p_menu->resorter != NULL) {
				// We tried to scroll up past the top. Re-survey the neighbors.
				p_menu->count = p_menu->resorter();
				max_visible_items = MIN(p_menu->count, (GFX_HEIGHT - SUBMENU_TITLE_SIZE) / font_height);

				for (uint8_t i = 0; i < max_visible_items; i++) {
					draw_menu_valid[i] = false;
				}
			}
		} else if (util_button_left()) {
			util_button_clear();
			return MENU_QUIT;
		} else if (util_button_action()) {
			util_button_clear();
			if (p_menu->items != NULL) {
				menu_item_t item = p_menu->items[p_menu->selected];
				if (item.callback != NULL) {
					item.callback(item.data);		// callback for this item
				}
			} else if (p_menu->callback != NULL) {
				p_menu->callback(p_menu->selected);	// callback for any item
			}
			return MENU_OK;

//			util_gfx_invalidate();
		}
	}
}

static void mbp_menu_bling_ks() {
	char *backer_images[] = {
			"BLING/BACKERS/ABRAXAS.RAW",
			"BLING/BACKERS/BENDERL.RAW"
	};

	menu_callback_t (*backer_callbacks[]) = {
			&mbp_bling_backer_abraxas3d,
			&mbp_bling_backer_andnxor
	};

	uint8_t selected = 0;
	uint8_t count = 2;
	util_gfx_invalidate();

	while (1) {
		if (!util_gfx_is_valid_state()) {
			util_led_clear();
			util_gfx_draw_raw_file(backer_images[selected], 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);
		}
		util_gfx_validate();

		util_button_wait();
		if (util_button_down() > 0) {
			selected = (selected + 1) % count;
			util_gfx_invalidate();
		} else if (util_button_up() > 0) {
			if (selected == 0) {
				selected = count - 1;
			} else {
				selected--;
			}
			util_gfx_invalidate();
		} else if (util_button_action()) {
			util_button_clear();
			backer_callbacks[selected](NULL);
			util_gfx_invalidate();
		} else if (util_button_left()) {
			util_button_clear();
			return;
		}

		util_button_clear();
	}
}

static void mbp_menu_bling() {
	uint16_t unlock = mbp_state_unlock_get();

	menu_t menu;
	menu_item_t items[48];
	menu.items = items;
	menu.selected = 0;
	menu.top = 0;
	menu.title = "Bling";
	menu.count = 0;

	items[menu.count++] = (menu_item_t ) { "Backers", "MENU/KSLOGO.ICO", "MENU/BLACK.PRV", &mbp_menu_bling_ks, NULL };

        // Check for master encounter unlocks
// 	if ((unlock & UNLOCK_MASK_MASTER_1) > 0) {
//             items[menu.count++] = (menu_item_t ) { "FoSci", "MENU/FOSCI.ICO", "MENU/FOSCI.PRV", &mbp_bling_fallout_boy_science, NULL };
// 	}
// 	if ((unlock & UNLOCK_MASK_MASTER_2) > 0) {
//             items[menu.count++] = (menu_item_t ) { "MyHorse", "MENU/MYHORSE.ICO", "MENU/MYHORSE.PRV", &mbp_bling_get_on_my_horse, NULL };
// 	}
// 	if ((unlock & UNLOCK_MASK_MASTER_3) > 0) {
//             items[menu.count++] = (menu_item_t ) { "Mltipas", "MENU/MLTIPASS.ICO", "MENU/MLTIPASS.PRV", &mbp_bling_multipass_leelo, NULL };
// 	}
// 	if ((unlock & UNLOCK_MASK_MASTER_4) > 0) {
//             items[menu.count++] = (menu_item_t ) { "5thEl", "MENU/5THEL.ICO", "MENU/5THEL.PRV", &mbp_bling_5th_element_dance, NULL };
// 	}
        // if you've unlocked all four, you get a bonus
// 	if ((unlock & UNLOCK_MASK_ALLMASTERS) == UNLOCK_MASK_ALLMASTERS) {
//             items[menu.count++] = (menu_item_t ) { "FoDrink", "MENU/FODRINK.ICO", "MENU/FODRINK.PRV", &mbp_bling_fallout_boygirl_drinking, NULL };
// 	}

	items[menu.count++] = (menu_item_t ) { "Custom", "MENU/WRENCH.ICO", NULL, &mbp_bling_menu_custom, NULL };

	items[menu.count++] = (menu_item_t ) { "16APSK", "MENU/16APSK.ICO", "MENU/16APSK.PRV", &mbp_bling_16APSK, NULL };
	items[menu.count++] = (menu_item_t ) { "AdaPi", "MENU/ADA.ICO", "MENU/ADA.PRV", &mbp_bling_ADA, NULL };
	items[menu.count++] = (menu_item_t ) { "Anime", "MENU/ANIME.ICO", "MENU/ANIME.PRV", &mbp_bling_ANIME, NULL };
	items[menu.count++] = (menu_item_t ) { "ARRL", "MENU/ARRL.ICO", "MENU/ARRL.PRV", &mbp_bling_ARRL, NULL };
	items[menu.count++] = (menu_item_t ) { "CRT", "MENU/CRT1.ICO", "MENU/CRT1.PRV", &mbp_bling_CRT1, NULL };
	items[menu.count++] = (menu_item_t ) { "Homer", "MENU/HOMER.ICO", "MENU/HOMER.PRV", &mbp_bling_HOMER, NULL };
	items[menu.count++] = (menu_item_t ) { "Horn", "MENU/HORN.ICO", "MENU/HORN.PRV", &mbp_bling_HORN, NULL };
	items[menu.count++] = (menu_item_t ) { "Key", "MENU/KEY.ICO", "MENU/KEY.PRV", &mbp_bling_KEY, NULL };
	items[menu.count++] = (menu_item_t ) { "Kuhl", "MENU/KUHL.ICO", "MENU/KUHL.PRV", &mbp_bling_KUHL, NULL };
	items[menu.count++] = (menu_item_t ) { "Mickey", "MENU/MICK.ICO", "MENU/MICK.PRV", &mbp_bling_MICK, NULL };
	items[menu.count++] = (menu_item_t ) { "Model", "MENU/MODEL.ICO", "MENU/MODEL.PRV", &mbp_bling_MODEL, NULL };
	items[menu.count++] = (menu_item_t ) { "Nixie", "MENU/NIXIE.ICO", "MENU/NIXIE.PRV", &mbp_bling_NIXIE, NULL };
	items[menu.count++] = (menu_item_t ) { "Noise", "MENU/NOISE.ICO", "MENU/NOISE.PRV", &mbp_bling_NOISE, NULL };
	items[menu.count++] = (menu_item_t ) { "Oscope", "MENU/OSCOPE.ICO", "MENU/OSCOPE.PRV", &mbp_bling_OSCOPE, NULL };
	items[menu.count++] = (menu_item_t ) { "Pattern", "MENU/PATTERN.ICO", "MENU/PATTERN.PRV", &mbp_bling_PATTERN, NULL };
	items[menu.count++] = (menu_item_t ) { "Prism", "MENU/PRISM.ICO", "MENU/PRISM.PRV", &mbp_bling_PRISM, NULL };
	items[menu.count++] = (menu_item_t ) { "Radio", "MENU/RADIO.ICO", "MENU/RADIO.PRV", &mbp_bling_RADIO, NULL };
	items[menu.count++] = (menu_item_t ) { "RKO1", "MENU/RKO1.ICO", "MENU/RKO1.PRV", &mbp_bling_RKO1, NULL };
	items[menu.count++] = (menu_item_t ) { "RKO2", "MENU/RKO2.ICO", "MENU/RKO2.PRV", &mbp_bling_RKO2, NULL };
	items[menu.count++] = (menu_item_t ) { "Rotate", "MENU/ROTATE.ICO", "MENU/ROTATE.PRV", &mbp_bling_ROTATE, NULL };
	items[menu.count++] = (menu_item_t ) { "Sat1", "MENU/SAT1.ICO", "MENU/SAT1.PRV", &mbp_bling_SAT1, NULL };
	items[menu.count++] = (menu_item_t ) { "Sat2", "MENU/SAT2.ICO", "MENU/SAT2.PRV", &mbp_bling_SAT2, NULL };
	items[menu.count++] = (menu_item_t ) { "Sponge", "MENU/SBOB.ICO", "MENU/SBOB.PRV", &mbp_bling_SBOB, NULL };
	items[menu.count++] = (menu_item_t ) { "SDR", "MENU/SDR.ICO", "MENU/SDR.PRV", &mbp_bling_SDR, NULL };
	items[menu.count++] = (menu_item_t ) { "Snow", "MENU/SNOW.ICO", "MENU/SNOW.PRV", &mbp_bling_SNOW, NULL };
	items[menu.count++] = (menu_item_t ) { "SouthP1", "MENU/SP1.ICO", "MENU/SP1.PRV", &mbp_bling_SP1, NULL };
	items[menu.count++] = (menu_item_t ) { "SouthP2", "MENU/SP2.ICO", "MENU/SP2.PRV", &mbp_bling_SP2, NULL };
	items[menu.count++] = (menu_item_t ) { "Spctrm1", "MENU/SPECT1.ICO", "MENU/SPECT1.PRV", &mbp_bling_SPECT1, NULL };
	items[menu.count++] = (menu_item_t ) { "Spctrm2", "MENU/SPECT2.ICO", "MENU/SPECT2.PRV", &mbp_bling_SPECT2, NULL };
	items[menu.count++] = (menu_item_t ) { "Spctrm3", "MENU/SP3.ICO", "MENU/SP3.PRV", &mbp_bling_SP3, NULL };
	items[menu.count++] = (menu_item_t ) { "Tesla", "MENU/TESLA.ICO", "MENU/TESLA.PRV", &mbp_bling_TESLA, NULL };
	items[menu.count++] = (menu_item_t ) { "Tower", "MENU/TOWER.ICO", "MENU/TOWER.PRV", &mbp_bling_TOWER, NULL };
	items[menu.count++] = (menu_item_t ) { "Trek1", "MENU/TREK1.ICO", "MENU/TREK1.PRV", &mbp_bling_TREK1, NULL };
	items[menu.count++] = (menu_item_t ) { "Trek2", "MENU/TREK2.ICO", "MENU/TREK2.PRV", &mbp_bling_TREK2, NULL };
	items[menu.count++] = (menu_item_t ) { "Yagi", "MENU/YAGI.ICO", "MENU/YAGI.PRV", &mbp_bling_YAGI, NULL };

	items[menu.count++] = (menu_item_t ) { "Flames", "MENU/FLAMES.ICO", "MENU/FLAMES.PRV", &mbp_bling_flames, NULL };
	items[menu.count++] = (menu_item_t ) { "Twitter", "MENU/TWITTER.ICO", "MENU/TWITTER.PRV", &mbp_bling_twitter, NULL };
	items[menu.count++] = (menu_item_t ) { "Matrix", "MENU/MATRIX.ICO", "MENU/MATRIX.PRV", &mbp_bling_matrix, NULL };
	items[menu.count++] = (menu_item_t ) { "Nyan", "MENU/NYAN.ICO", "MENU/NYAN.PRV", &mbp_bling_nyan, NULL };
	items[menu.count++] = (menu_item_t ) { "Scroll", "MENU/SCROLL.ICO", "MENU/BLACK.PRV", &mbp_bling_scroll_cycle, NULL };
	items[menu.count++] = (menu_item_t ) { "Pirate", "MENU/PIRATES.ICO", "MENU/PIRATES.PRV", &mbp_bling_pirate, NULL };

	//Add illusion bling
	if ((unlock & UNLOCK_MASK_TWITTER) > 0) {
		items[menu.count++] = (menu_item_t ) { "Illusn", "MENU/ILLUSION.ICO", "MENU/ILLUSION.PRV", &mbp_bling_illusion, NULL };
	}

	//Defrag bling
	if ((unlock & UNLOCK_MASK_DEFRAG) > 0) {
		items[menu.count++] = (menu_item_t ) { "Defrag", "MENU/DEFRAG.ICO", "MENU/DEFRAG.PRV", &mbp_bling_defrag, NULL };
	}

	//Hack time bling
	if ((unlock & UNLOCK_MASK_DATE_TIME) > 0) {
		items[menu.count++] = (menu_item_t ) { "Time", "MENU/HACKTIME.ICO", "MENU/HACKTIME.PRV", &mbp_bling_hack_time, NULL };
	}

	mbp_background_led_stop();
	//clear out app_scheduler
	app_sched_execute();
	mbp_menu(&menu);
	mbp_background_led_start();
}

static void mbp_menu_games() {
	menu_item_t items[] = {
			{ "Mastermind", NULL, NULL, &mastermind, NULL },
			{ "Ski Free", NULL, NULL, &ski, NULL },
			{ "CHIP-8", NULL, NULL, &chip8_menu, NULL },
	};

	menu_t menu;
	menu.items = items;
	menu.count = 3;
	menu.selected = 0;
	menu.title = "Games";
	menu.top = 0;
	mbp_submenu(&menu);
}


// Action handler for all entries on the nearby menu
static void __nearby_callback(uint8_t index) {
	//!!! write me
}


static void mbp_menu_nearby() {
	menu_t menu;

	menu.items = NULL;
	menu.count = survey_and_sort_neighbors();
	menu.title = "Nearby";
	menu.callback = __nearby_callback;
	menu.draw_item = ble_lists_draw_callback;
	menu.resorter = survey_and_sort_neighbors;

	if (menu.count == 0) {
		mbp_ui_popup("Nearby", "Sorry no neighbors :(");
		return;
	}

	//!!!vile hack for testing
	// ble_gap_addr_t hack_address;
	// uint8_t raw_addr[] = { 0, 0x74, 0x50, 0xb9, 0xe1, 0x74, 0xc5};
	// memcpy((uint8_t *)&hack_address, raw_addr, 7);

	//for (uint8_t i = 0; i < badge_list_size; i++) {
	//	items[menu.count++] = (menu_item_t ) { list[i].text, NULL, NULL, &transio_qso_attempt, NULL };
//		items[menu.count++] = (menu_item_t ) { list[i].text, NULL, NULL, &mbp_medea_hack, &hack_address };
	//}

	mbp_submenu(&menu);
}

static void mbp_menu_system() {
	menu_t menu;
	menu_item_t items[10];
	menu.items = items;
	menu.count = 0;

	items[menu.count++] = (menu_item_t ) { "Name", "MENU/NAME.ICO", NULL, &mbp_system_name_edit, NULL };
	if (mbp_state_master_get())
	    items[menu.count++] = (menu_item_t ) { "Special", "MENU/NAME.ICO", NULL, &mbp_system_special_edit, NULL };
	items[menu.count++] = (menu_item_t ) { "About", "MENU/ABOUT.ICO", NULL, &mbp_system_about, NULL };
	items[menu.count++] = (menu_item_t ) { "Shouts", "MENU/SHOUTS.ICO", NULL, &mbp_system_shouts, NULL };
	items[menu.count++] = (menu_item_t ) { "Games", "MENU/CONTROL.ICO", NULL, &mbp_system_game_menu, NULL };
	items[menu.count++] = (menu_item_t ) { "Plane", "MENU/AIRPLANE.ICO", NULL, &mbp_system_airplane_mode_select, NULL };
	items[menu.count++] = (menu_item_t ) { "Test", "MENU/TEST.ICO", NULL, &mbp_system_test, NULL };
	items[menu.count++] = (menu_item_t ) { "Tilt", "MENU/TILT.ICO", NULL, &mbp_system_tilt_mode_select, NULL };
	items[menu.count++] = (menu_item_t ) { "Reset", "MENU/RESET.ICO", NULL, &mbp_system_reset, NULL };

	menu.selected = 0;
	menu.title = "System";
	menu.top = 0;
	mbp_menu(&menu);
}

void mbp_menu_main() {
	menu_t menu;
	menu_item_t items[10];
	menu.count = 0;
	items[menu.count++] = (menu_item_t ) { "Bling!", "MENU/BLING.ICO", NULL, &mbp_menu_bling, NULL };
	items[menu.count++] = (menu_item_t ) { "ViewLog", "MENU/SCORE.ICO", NULL, &transio_log_screen, NULL };
	items[menu.count++] = (menu_item_t ) { "Games", "MENU/CONTROL.ICO", NULL, &mbp_menu_games, NULL };
	items[menu.count++] = (menu_item_t ) { "Nearby", "MENU/NEARBY.ICO", NULL, &mbp_menu_nearby, NULL };
        items[menu.count++] = (menu_item_t ) { "TCL", "MENU/TCL.ICO", NULL, &mbp_tcl_menu, NULL };
	items[menu.count++] = (menu_item_t ) { "Code", "MENU/CODE.ICO", NULL, &mbp_system_code, NULL };
	items[menu.count++] = (menu_item_t ) { "System", "MENU/GEAR.ICO", NULL, &mbp_menu_system, NULL };

// JOCO TODO we should put back in a master control functionality
//	if (mbp_state_master_get()) {
//		items[menu.count++] = (menu_item_t ) { "Master", "MENU/MASTER.ICO", NULL, &mbp_master_menu_main, NULL };
//	}

	menu.items = items;
	menu.title = "TRANSIO";

	mbp_background_led_start();
	mbp_menu(&menu);
}
