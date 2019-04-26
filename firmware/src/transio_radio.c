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


void transio_radio_screen() {
 
	mbp_background_led_stop();
	//clear out app_scheduler
	app_sched_execute();

	bool smeter_was_on = m_smeter_available;
	if (!smeter_was_on) {
		util_i2c_smeter_start();
	}
	mbp_rssi_stop();

	util_gfx_invalidate();

    bool redraw = true;

    while (1) {
		if (redraw || !util_gfx_is_valid_state()) {

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

            util_gfx_print("None detected");

			redraw = false;
		}

		//validate screen state
		util_gfx_validate();

		util_button_wait();

		if (util_button_left() || util_button_action()) {
			util_button_clear();

			break;
		}

		util_button_clear();

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
