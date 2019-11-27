/*****************************************************************************
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

#define UI_MARGIN	3
#define SCREEN_WIDTH	20	// characters of small font
#define SCREEN_LINES	9	// rows of small font

#define	MAX_AD_FIELD_LEN	26

#define TRANSIO_RADIO_INTERVAL_MS	 1000
#define TRANSIO_RADIO_TIMEOUT_MS	15000
APP_TIMER_DEF(transio_radio_timer);

uint32_t transio_radio_timestamp = 0;           // timestamp of last received radio info
uint8_t transio_radio_address[BLE_GAP_ADDR_LEN];// BLE address of the radio
int8_t transio_radio_rssi = INT8_MIN;           // received signal strength of the radio
char transio_radio_name[SETTING_NAME_LENGTH];   // name of the radio
uint8_t transio_radio_msd[MAX_AD_FIELD_LEN];    // manufacturer-specific data from the radio
uint8_t transio_radio_msd_length;               // length of manufacturer-specific data

bool transio_radio_redraw;
bool transio_radio_update;


// Age out radio if we haven't seen it for a while
static void __transio_radio_timer_handler(void *p_data) {
	uint32_t timenow = util_millis();

	if (timenow - transio_radio_timestamp > TRANSIO_RADIO_TIMEOUT_MS) {
		transio_radio_rssi = INT8_MIN;			// flag the radio as unheard
		memset(transio_radio_address, 0, BLE_GAP_ADDR_LEN);
		app_timer_stop(transio_radio_timer);	// and stop checking for now

		transio_radio_redraw = true;			// screen needs updating

	}

}


// Run a timer for radio aging
static void __transio_radio_timer_start(void) {
	uint32_t err_code;
	static bool setup = false;

	if (!setup) {
		err_code = app_timer_create(&transio_radio_timer, APP_TIMER_MODE_REPEATED, __transio_radio_timer_handler);
		APP_ERROR_CHECK(err_code);
	}

	uint32_t ticks = APP_TIMER_TICKS(TRANSIO_RADIO_INTERVAL_MS, UTIL_TIMER_PRESCALER);
	err_code = app_timer_start(transio_radio_timer, ticks, NULL);
	APP_ERROR_CHECK(err_code);

}


void transio_radio_screen() {
 	char buffer[32];

	mbp_background_led_stop();
	//clear out app_scheduler
	app_sched_execute();

	bool smeter_was_on = m_smeter_available;
	if (!smeter_was_on) {
		util_i2c_smeter_start();
	}
	mbp_rssi_stop();

	util_gfx_invalidate();

    transio_radio_redraw = true;

    while (1) {
		if (transio_radio_redraw || !util_gfx_is_valid_state()) {
		    //Make sure there's no clipping
		    util_gfx_cursor_area_reset();

		    //Draw blank background
		    mbp_ui_cls();

		    //Print screen banner
		    util_gfx_set_font(FONT_LARGE);
		    util_gfx_set_color(COLOR_GREEN);
		    util_gfx_set_cursor(0, UI_MARGIN);
		    util_gfx_print("P4G Radio");

			util_gfx_set_font(FONT_SMALL);
			util_gfx_set_color(COLOR_WHITE);
			util_gfx_set_cursor(0, 20);

            if (transio_radio_rssi == INT8_MIN) {
                util_gfx_print("None detected");
            } else {
                util_gfx_set_cursor(0, 20);
                util_gfx_print("Name: ");
                util_gfx_print(transio_radio_name);
				transio_radio_update = true;
            }

			transio_radio_redraw = false;
		}

		if (transio_radio_update) {
			util_gfx_fill_rect(6*5, 32, 100, 24, COLOR_BLACK);

			util_gfx_set_cursor(0, 32);
			util_gfx_print("RSSI: ");
			sprintf(buffer, "%d", transio_radio_rssi);
			util_gfx_print(buffer);

			util_gfx_set_cursor(0, 44);
			util_gfx_print("Data: ");
			util_hex_encode((uint8_t *)buffer, transio_radio_msd, transio_radio_msd_length);
			util_gfx_print(buffer);

			transio_radio_update = false;
		}

		//validate screen state
		util_gfx_validate();

		//util_button_wait();

		if (util_button_left() || util_button_action()) {
			util_button_clear();

			break;
		}

		util_button_clear();

		app_sched_execute();
		nrf_delay_ms(300);
	}

	app_timer_stop(transio_radio_timer);
	if (!smeter_was_on) {
		util_i2c_smeter_stop();
	}
	mbp_rssi_start();
	util_led_clear();
	util_button_clear();
	mbp_background_led_start();
}


// Process a new incoming advertisement from a P4G radio
void transio_radio_process_advertisement(uint8_t address[],
										 char name[],
										 uint16_t appearance,
                                         uint8_t msd_length,
										 uint8_t mfg_specific_data[],
										 int8_t rssi) {

    uint32_t timestamp = util_millis();

    if (appearance != APPEARANCE_ID_ORI_P4GRADIO) {
        // Insist on the appearance matching exactly. We can get fancier later.
        return;
    }

    // Is it a different radio than we've been displaying?
    if (memcmp(address, transio_radio_address, BLE_GAP_ADDR_LEN) != 0) {
        if (rssi > transio_radio_rssi) {
            // switch to monitoring the new, stronger radio
            memcpy(transio_radio_address, address, BLE_GAP_ADDR_LEN);
            memcpy(transio_radio_name, name, SETTING_NAME_LENGTH);
			transio_radio_redraw = true;
			__transio_radio_timer_start();	// start checking radio's age
        } else {
            return;     // disregard a radio that's new and weaker than the old one
        }
    }
    
    // Same radio or different one, we update the current info.
    transio_radio_rssi = rssi;
	transio_radio_msd_length = msd_length;
    memcpy(transio_radio_msd, mfg_specific_data, msd_length);
    transio_radio_timestamp = timestamp;
    transio_radio_update = true;
}
