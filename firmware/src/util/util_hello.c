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


// Callback passed to util_gfx_draw_raw_file() to stop playback after one frame.
static void __play_1_frame_only(uint8_t frame, void *p_data) {
	util_gfx_draw_raw_file_stop();
}


static void __do_hello(char *name, char *lcd_file) {
    uint16_t w, h;
    app_sched_pause();
    bool background_was_running = mbp_background_led_running();
    mbp_background_led_stop();

    //Pick colors
    float h1 = ((float) util_math_rand8_max(100) / 100.0);
    float h2 = h1 + 0.5;
    if (h2 >= 1.0) {
        h2 -= 1.0;
    }
    uint32_t color_1 = util_led_hsv_to_rgb(h1, 1, 1);
    uint32_t color_2 = util_led_hsv_to_rgb(h2, 1, 1);

    util_gfx_draw_raw_file(lcd_file, 0, 0, GFX_WIDTH, GFX_HEIGHT, __play_1_frame_only, false, NULL);
    uint16_t fg = util_gfx_rgb_to_565(color_1);
    uint16_t bg = util_gfx_rgb_to_565(color_2);

    //Compute name coords
    util_gfx_set_font(FONT_LARGE);
    util_gfx_get_text_bounds(name, 0, 0, &w, &h);
    uint16_t y = (GFX_HEIGHT / 2) + 4;
    uint16_t x = (GFX_WIDTH - w) / 2;

    //Print background behind text
	util_gfx_fill_rect(x-3, y-7, w+7, h+8, bg);

    //Print name
    util_gfx_set_color(fg);
    util_gfx_set_cursor(x, y);
    util_gfx_print(name);

    //Compute hello coords
    char hello[] = "Hello";
    util_gfx_get_text_bounds(hello, 0, 0, &w, &h);
    x = (GFX_WIDTH - w) / 2;
    y = (GFX_HEIGHT / 2) - 4 - h;

    //Print background behind text
	util_gfx_fill_rect(x-3, y-7, w+7, h+8, bg);

    //Print hello
    util_gfx_set_color(fg);
    util_gfx_set_cursor(x, y);
    util_gfx_print(hello);

    //Set all LEDs
    util_led_set_all_rgb(color_1);
    util_led_show();
    nrf_delay_ms(2000);

	// This made sense on Bender DC25. It still looks cool, so leave it for now.
    uint8_t cols[5][4] = {
            { 8, 4, 0, 12 },
            { 9, 5, 1, 255 },
            { 10, 6, 2, 255 },
            { 11, 7, 3, 13 },
            { 255, 255, 255, 14 }
    };
    uint8_t height[] = { 4, 4, 4, 4, 4 };

    while (1) {
        //pick a random column to lower
        uint8_t col = util_math_rand8_max(5);
        if (height[col] > 0) {
            height[col]--;
            uint8_t index = cols[col][height[col]];

            if (index < LED_COUNT) {
                util_led_set_rgb(index, color_2);
                util_led_show();
                nrf_delay_ms(40);
                util_led_set(index, 0, 0, 0);
                util_led_show();
            }
        }

        nrf_delay_ms(30);

        bool done = true;
        for (uint8_t i = 0; i < 5; i++) {
            if (height[i] > 0) {
                done = false;
                break;
            }
        }

        if (done) {
            break;
        }
    }

    //Cleanup and give control back to user
    util_gfx_invalidate();
    if (background_was_running) {
        mbp_background_led_start();
    }
    app_sched_resume();
    util_button_clear();
}


// Called from the LED background bling once per cycle, so as to schedule
// Hello blings at an appropriate time.
void hello_background_handler(void * p_event_data, uint16_t event_size) {
	uint16_t hello_company_id;

	// Name needs to hang around for the bling schedule handler to use it.
	static char hello_name[SETTING_NAME_LENGTH];

	// Search the neighbor list for the most hello-worthy neighbor
	if (ble_lists_choose_hello_neighbor(&hello_company_id, hello_name)) {

		switch (hello_company_id) {
			case COMPANY_ID_TRANSIO:
			case COMPANY_ID_TRANSIO_TMP:
				__do_hello(hello_name, "JINTRO.RAW");
				break;

			case COMPANY_ID_JOCO:
				__do_hello(hello_name, "B2B/PM_HELLO.RAW");
				break;

			case COMPANY_ID_ANDNXOR:
				__do_hello(hello_name, "B2B/BENDER.RAW");
				break;

			case COMPANY_ID_CPV:
				__do_hello(hello_name, "B2B/CPV.RAW");
				break;

			case COMPANY_ID_DC503:
				__do_hello(hello_name, "B2B/DC503.RAW");
				break;

			case COMPANY_ID_DC801:
				__do_hello(hello_name, "B2B/DC801.RAW");
				break;

			case COMPANY_ID_QUEERCON:
				__do_hello(hello_name, "B2B/QUEERCON.RAW");
				break;

			case COMPANY_ID_DCDARKNET:
			case COMPANY_ID_DCZIA:
			case COMPANY_ID_FoB1un7:
			case COMPANY_ID_TDI:
			case COMPANY_ID_DCFURS:
			case COMPANY_ID_BLINKYBLING:
			default:
				__do_hello(hello_name, "B2B/GHELLO.RAW");
				break;
		}
	}
}
