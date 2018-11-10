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
 *      @abraxas3d
 *
 *****************************************************************************/
#include "system.h"

#define PIN_SEEKRIT		24

static bool m_interuptable = true;

void mbp_system_about() {
	char about[64];
	uint16_t serial = util_get_device_id();
	ble_version_t ble;
	sd_ble_version_get(&ble);
	memset(about, '\0', 64);
	sprintf(about, "Firmware:\n%s\nSoft Device:\n%#X\nDevice ID:\n%#X", build_timestamp, ble.subversion_number, serial);
	mbp_ui_popup("TRANS-IO", about);
}

void mbp_system_airplane_mode_select() {
	if (mbp_state_airplane_mode_get()) {
		bool enabled = mbp_ui_toggle_popup("Airplne Mode", 0, "Enable", "Disable", "Currently: ENABLED") == 0;
		mbp_state_airplane_mode_set(enabled);
		mbp_state_save();

		//Turn off
		if (!enabled) {
			util_ble_on();
		}
	} else {
		bool enabled = mbp_ui_toggle_popup("Airplne Mode", 1, "Enable", "Disable", "Currently: DISABLED") == 0;
		mbp_state_airplane_mode_set(enabled);
		mbp_state_save();

		//Turn on
		if (enabled) {
			util_ble_off();
		}
	}
}

void mbp_system_code() {
	char code[9];
	memset(code, 0, 9);
	mbp_ui_input("Code", "Enter Code:", code, 8, INPUT_FORMAT_SCROLL);

	//Master mode
	if (strcmp(code, "UP2RIGHT") == 0) {
		mbp_state_master_set(true);
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_MASTER_1 | UNLOCK_MASK_MASTER_2 | UNLOCK_MASK_MASTER_3 | UNLOCK_MASK_MASTER_4);
		mbp_state_save();
		mbp_ui_popup("Master", "Master Mode Engaged.");
	}
	//Wheaton
	else if (strcmp(code, "TABLETOP") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_WHEATON);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Wil Wheaton mode enabled.");
	}
	//Illusion
	else if (strcmp(code, "NUKACOLA") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_TWITTER);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Illusion Bling unlocked.");
	}

	//Hack Time
	else if (strcmp(code, "HAXXOR") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_DATE_TIME);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Hack Time Bling unlocked.");
	}

	//Defrag
	else if (strcmp(code, "DISKFULL") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_DEFRAG);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Defrag Bling unlocked.");
	}

	else if (strcmp(code, "CHEAT") == 0) {
		cheat_at_mastermind = true;		// not saved!
	}

	//Everything else
	else {
		mbp_ui_error(":(");
	}
}

uint16_t mbp_system_color_selection_menu(uint16_t current_color) {
	//first color will get replaced
	uint16_t colors[] = {
	COLOR_BLACK,
	COLOR_BLACK, COLOR_BROWN, COLOR_NAVY, COLOR_DARKBLUE, COLOR_DARKGREEN,
	COLOR_DARKCYAN, COLOR_MAROON, COLOR_PURPLE, COLOR_OLIVE, COLOR_LIGHTGREY,
	COLOR_DARKGREY, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN, COLOR_RED,
	COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE, COLOR_ORANGE, COLOR_GREENYELLOW,
	COLOR_PINK, COLOR_NEONPURPLE };
	char *color_names[] = {
			"Black",
			"Black", "Brown", "Navy", "Dark Blue", "Dark Green",
			"Dark Cyan", "Maroon", "Purple", "Olive", "Light Grey",
			"Dark Grey", "Blue", "Green", "Cyan", "Red",
			"Magenta", "Yellow", "White", "Orange", "Green Yellow",
			"Pink", "Neon Purple" };

	menu_t menu;
	menu.count = 0;
	menu.title = "Pick Color";
	menu_item_t items[23];
	menu.items = items;

	//First menu item should be the name of the current color
	for (uint8_t i = 0; i < 23; i++) {
		if (current_color == colors[i]) {
			items[menu.count++] = (menu_item_t ) { color_names[i], NULL, NULL, NULL, NULL };
			break;
		}
	}

	//All other colors add after that
	for (uint8_t i = 1; i < 23; i++) {
		items[menu.count++] = (menu_item_t ) { color_names[i], NULL, NULL, NULL, NULL };
	}

	//If they quit, return the current color
	if (mbp_submenu(&menu) == MENU_QUIT) {
		return current_color;
	}

	//If they picked something return the selected color
	return colors[menu.selected];
}

bool mbp_system_interuptable_get() {
	return m_interuptable;
}

//void mbp_system_interuptable_set(bool interuptable) {
//	m_interuptable = interuptable;
//}

void mbp_system_game_menu() {
	menu_item_t items[] = {
			{ "Game Exit Hint", NULL, NULL, NULL, NULL },
			{ "Foreground Color", NULL, NULL, NULL, NULL },
			{ "Background Color", NULL, NULL, NULL, NULL },
			{ "LED Beep Toggle", NULL, NULL, NULL, NULL },
			{ "Accept Incoming", NULL, NULL, NULL, NULL },
			{ "Clear QSO Log", NULL, NULL, NULL, NULL },
	};
	menu_t menu;
	menu.count = 6;
	menu.items = items;
	menu.selected = 0;
	menu.title = "Game CFG";
	menu.top = 0;

	if (mbp_submenu(&menu) == MENU_QUIT) {
		return;
	}

	switch (menu.selected) {
	case 0:
		; //Toggle the Exit Game Hint
		if (mbp_state_game_exit_pop_up_get()) {
			mbp_state_game_exit_pop_up_set(mbp_ui_toggle_popup("Hint Pop Up", 0, "Enable", "Disable", "Currently: ENABLED") == 0);
			mbp_state_save();
		}
		else {
			mbp_state_game_exit_pop_up_set(mbp_ui_toggle_popup("Hint Pop Up", 1, "Enable", "Disable", "Currently: DISABLED") == 0);
			mbp_state_save();
		}
		break;

	case 1:
		; //Change the CHIP8 Foreground Color
		mbp_state_chip8_fg_color_set(mbp_system_color_selection_menu(mbp_state_chip8_fg_color_get()));
		mbp_state_save();
		break;

	case 2:
		; //Change the CHIP8 Background Color
		mbp_state_chip8_bg_color_set(mbp_system_color_selection_menu(mbp_state_chip8_bg_color_get()));
		mbp_state_save();
		break;

	case 3:
		; //Toggle the LED Blinks for Sound
		if (mbp_state_game_led_sound_get()) {
			mbp_state_game_led_sound_set(mbp_ui_toggle_popup("LED Beeps", 0, "Enable", "Disable", "Currently: ENABLED") == 0);
			mbp_state_save();
		}
		else {
			mbp_state_game_led_sound_set(mbp_ui_toggle_popup("LED Beeps", 1, "Enable", "Disable", "Currently: DISABLED") == 0);
			mbp_state_save();
		}
		break;

	case 4:
		; //Decide whether to accept incoming multiplayer game requests
		if (mbp_state_game_incoming_ok_get()) {
			mbp_state_game_incoming_ok_set(mbp_ui_toggle_popup("Incoming", 0, "Accept", "Reject", "Currently: Accept incoming game requests from other badges") == 0);
			mbp_state_save();
		}
		else {
			mbp_state_game_incoming_ok_set(mbp_ui_toggle_popup("Incoming", 1, "Accept", "Reject", "Currently: Reject incoming game requests from other badges") == 0);
			mbp_state_save();
		}
		break;

	case 5:
		if (mbp_ui_toggle_popup("Erase Log", 1, "Erase", "Cancel", "Do you want to erase the entire QSO log? Really?") == 0) {
			if (f_unlink(LOG_FILENAME) != FR_OK) {
				mbp_ui_error("Could not remove logfile");
			}
		}
		break;

	}

}

#define SPECIAL_STRING_LEN 4
void mbp_system_special_edit() {
    uint8_t special;
    special = mbp_state_special_get();
    char special_buf[SPECIAL_STRING_LEN];

//Ask if they want to clear
    char message[64];
    sprintf(message, "Value is '%03d' (MAX 255), edit existing or clear?", special);
    if (mbp_ui_toggle_popup("Value", 0, "Edit", "Clear", message) == 1) {
	special = 0;
    }

//Edit the special value
    sprintf(special_buf, "%03d", special);
    mbp_ui_input("Value", "Enter Value:", special_buf, SPECIAL_STRING_LEN - 1, INPUT_FORMAT_DIGITS);
    special = atoi(special_buf) % 256;

    sprintf(message, "Change value to: '%03d'?", special);
    if (mbp_ui_toggle_popup("Name", 0, "No", "Yes", message) == 1) {
	mbp_state_special_set(special);
	mbp_state_save();

	sprintf(message, "Value changed to '%03d'.", special);
	mbp_ui_popup("Value", message);
    }
    //Aborted
    else {
	mbp_ui_popup("Value", "Value not changed.");
    }
}

void mbp_system_name_edit() {
	char name[SETTING_NAME_LENGTH];
	mbp_state_name_get(name);

//Ask if they want to clear
	char message[64];
	sprintf(message, "Name is '%s', edit existing or clear?", name);
	if (mbp_ui_toggle_popup("Name", 0, "Edit", "Clear", message) == 1) {
		memset(name, 0, SETTING_NAME_LENGTH);
	}

//Edit the name
	mbp_ui_input("Name", "Enter Name:", name, SETTING_NAME_LENGTH - 1, INPUT_FORMAT_FREE);

	sprintf(message, "Change name to: '%s'?", name);
	if (mbp_ui_toggle_popup("Name", 0, "No", "Yes", message) == 1) {
		mbp_state_name_set(name);
		mbp_state_save();

		sprintf(message, "Name changed to '%s'.", name);
		mbp_ui_popup("Name", message);
	}
	//Aborted
	else {
		mbp_ui_popup("Name", "Name not changed.");
	}

}

void mbp_system_name_select() {
	char *names[] = {
		"Ori", "Hiram", "Percy", "Maxim", "Wouff",
		"DeForest", "Marconi", "Zenith", "Phase4", "Extra"
	};
	uint8_t name_count = 10;

	menu_item_t items[name_count];
	menu_t menu;
	menu.count = name_count;
	menu.items = items;
	menu.title = "Name";

	for (uint8_t i = 0; i < name_count; i++) {
		items[i] = (menu_item_t ) { names[i], NULL, NULL, NULL, NULL };
	}

	while (mbp_submenu(&menu) != MENU_OK)
		;

	mbp_state_name_set(names[menu.selected]);

	char message[32];
	sprintf(message, "Name set to '%s'", names[menu.selected]);
	mbp_ui_popup("Name", message);
}

void mbp_system_reset() {
	if (mbp_ui_toggle_popup("Reset", 1, "Yes", "Cancel", "Progress will be lost. Are you sure?") == 0) {
		// uint16_t contacted_count;
		mbp_state_new();
		// Give credit for all contacts in the db
		// contacted_count = count_db_entries();
		// mbp_state_score_set(contacted_count * POINTS_4_VISIT);
		mbp_state_save();
		mbp_ui_popup("Reset", "Badge is a n00b again.");
		mbp_system_name_select();
	}
}

bool mbp_system_seekrit_get() {
	nrf_gpio_cfg_input(PIN_SEEKRIT, NRF_GPIO_PIN_PULLUP);
	return nrf_gpio_pin_read(PIN_SEEKRIT) == 0;
}

void mbp_system_shouts() {
	mbp_ui_popup(
			"Shouts",
			"@ANDnXOR\n"
			"Partners\n"
			"ORI badge team\n"
			"#badgelife\n"
			"... and you!\n");
}

void mbp_system_bugreports() {
	mbp_ui_popup(
			"BugReports",
			"Found bugs?\n"
			"Find us at\n"
			"Open Research\n"
			"Institute in\n"
			"DC26 Wireless\n"
			"Village");
}

void mbp_system_test() {
	uint8_t i;

	mbp_background_led_stop();
	//clear out app_scheduler
	app_sched_execute();
	util_gfx_set_font(FONT_SMALL);
	char buffer[32];

	uint16_t i2c_test_data = 0xff00;
	bool smeter_was_on = m_smeter_available;
	if (!smeter_was_on) {
		util_i2c_smeter_start();
	}
	mbp_rssi_stop();

	util_gfx_invalidate();
	uint8_t color = 0;

	while (1) {
		//Full redraw if invalidated
		if (!util_gfx_is_valid_state()) {
			mbp_ui_cls();
			util_gfx_set_font(FONT_SMALL);
			util_gfx_set_color(COLOR_WHITE);

			util_gfx_set_cursor(0, 0);
			util_gfx_print("Micro SD");

			util_gfx_set_cursor(0, 12);
			util_gfx_print("I2C");

			util_gfx_set_cursor(0, 24);
			util_gfx_print("Up");

			util_gfx_set_cursor(0, 36);
			util_gfx_print("Down");

			util_gfx_set_cursor(0, 48);
			util_gfx_print("Left");

			util_gfx_set_cursor(0, 60);
			util_gfx_print("Right");

			util_gfx_set_cursor(0, 72);
			util_gfx_print("Action");

			util_gfx_set_cursor(0, 84);
			util_gfx_print("Nearby");

			util_gfx_set_cursor(0, 96);
			util_gfx_print("Time");

			util_gfx_set_cursor(0, 108);
			util_gfx_print("Build");

			//FW Version (this won't change!)
			util_gfx_set_cursor(36, 108);
			util_gfx_print(build_timestamp);
		}

		//Clear values
		util_gfx_fill_rect(90, 0, 38, 108, COLOR_BLACK);
		util_gfx_fill_rect(36, 12, 92, 12, COLOR_BLACK);	// for I2C wider data

		//Test for microsd
		util_gfx_set_cursor(90, 0);
		if (util_sd_file_size("VERSION") > 0) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("Yes");
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("No");
		}

		//Check that I2C is even working
		if (! m_i2c_available) {
			util_gfx_set_cursor(66, 12);
			util_gfx_print("Bus stuck");
		} else {
			//Discover I2C devices and display them
			uint8_t num_i2c = util_i2c_count();
			// Desired output depending on number of I2C devices found:
			// I2C            None
			// I2C            01
			// I2C            01 02
			// I2C         01 02 03
			// I2C      01 02 03 04
			// I2C   01 02 03 04 05
			// I2C        123 found
			static const uint8_t hex_columns[] = {90, 90, 90, 72, 54, 36, 66};
			util_gfx_set_cursor(hex_columns[num_i2c < 6 ? num_i2c : 6], 12);

			if (num_i2c == 0) {
				util_gfx_set_color(COLOR_WHITE);
				util_gfx_print("None");
			} else if (num_i2c > 5) {
				util_gfx_set_color(COLOR_RED);
				sprintf(buffer, "%3u found", num_i2c);
				util_gfx_print(buffer);
			} else {
				uint8_t scan_addr7 = UTIL_I2C_FIRST_NORMAL_I2C_ADDRESS7;
				uint8_t result;

				util_gfx_set_color(COLOR_YELLOW);
				for (i=0; i < num_i2c-1; i++) {
					result = util_i2c_get_next_device_address(scan_addr7);
					if (result != UTIL_I2C_INVALID_I2C_ADDRESS7) {
						sprintf(buffer, "%02x ", result);
						util_gfx_print(buffer);
						scan_addr7 = result + 1;
					}
				}
				sprintf(buffer, "%02x", util_i2c_get_next_device_address(scan_addr7));
				util_gfx_print(buffer);
			}
		}

		//up button
		util_gfx_set_cursor(90, 24);
		if (util_button_up()) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("true");
			util_i2c_ioexp_out(i2c_test_data);
			util_i2c_smeter_write(i2c_test_data & 0x1F);
			i2c_test_data += 0xFF01;	// increment LSbyte, decrement MSbyte
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("false");
		}

		//down button
		util_gfx_set_cursor(90, 36);
		if (util_button_down()) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("true");
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("false");
		}

		//left button
		util_gfx_set_cursor(90, 48);
		if (util_button_left()) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("true");
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("false");
		}

		//right button
		util_gfx_set_cursor(90, 60);
		if (util_button_right()) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("true");
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("false");
		}

		//action button
		util_gfx_set_cursor(90, 72);
		if (util_button_action()) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("true");
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("false");
		}

		//Show badge db count
		util_gfx_set_color(COLOR_WHITE);
		util_gfx_set_cursor(90, 84);
		sprintf(buffer, "%d", survey_and_sort_neighbors(NEIGHBOR_FILTER_NONE));
		util_gfx_print(buffer);

		//Current time
		util_gfx_set_cursor(90, 96);
		sprintf(buffer, "%lu", util_millis() / 1000);
		util_gfx_print(buffer);

		util_gfx_validate();

		if (util_tilt_inverted()) {
                    util_led_set_all(255, 255, 255);
                    util_led_show();
                } else {
                    //Test LEDs
                    switch (color) {
                    case 0:
                            util_led_set_all(255, 0, 0);
                            break;
                    case 1:
                            util_led_set_all(0, 255, 0);
                            break;
                    case 2:
                            util_led_set_all(0, 0, 255);
                            break;
                    }
                    util_led_show();
                    color = (color + 1) % 3;
                }

                util_gfx_set_color(COLOR_WHITE);
		//Allow user to quit
		if (util_button_left() > 0) {
			break;
		}

		app_sched_execute();
		nrf_delay_ms(300);
	}

	if (!smeter_was_on) {
		util_i2c_smeter_stop();
	}
	mbp_rssi_start();
	util_led_clear();
	util_button_clear();
	mbp_background_led_start();
}

void mbp_system_tilt_mode_select() {
	if (mbp_state_tilt_get()) {
		mbp_state_tilt_set(mbp_ui_toggle_popup("Tilt", 0, "Enable", "Disable", "Tilt sensor is enabled.") == 0);
	} else {
		mbp_state_tilt_set(mbp_ui_toggle_popup("Tilt", 1, "Enable", "Disable", "Tilt sensor is disabled.") == 0);
	}
	mbp_state_save();
}

void mbp_system_unlock_state() {
	uint16_t unlock = mbp_state_unlock_get();

	mbp_background_led_stop();
	//Clear out scheduler of any background LED events
	app_sched_execute();

	util_led_clear();

	//If they've unlocked all 16 set all LEDs to blue
	if (unlock == 0xFFFF) {
		util_led_set_all(0, 0, 255);
	} else {
		for (uint8_t i = 0; i <= LED_COUNT; i++) {
			if ((unlock & (1 << i)) > 0) {
				util_led_set_rgb(i, LED_COLOR_ORANGE);
			}
		}
	}

	util_led_show();
	util_gfx_draw_raw_file("BLING/TRANSIO/MAGICEYE.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, true, NULL);
	util_led_clear();

	mbp_background_led_start();
}
