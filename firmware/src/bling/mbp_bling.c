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
 *  @andnxor
 *  @zappbrandnxor
 *  @hyr0n1
 *  @andrewnriley
 *  @lacosteaef
 *  @bitstr3m
 *
 * Further modifications made by
 *      @sconklin
 *      @mustbeart
 *      @abraxas3d
 *
 *****************************************************************************/

#include "../system.h"

#define BACKGROUND_LED_TIME_MS     10
#define BG_TICKS_PER_SECOND		(1000/BACKGROUND_LED_TIME_MS)

#define SCROLL_CHAR_WIDTH           16
#define SCROLL_CHAR_HEIGHT          31
#define SCROLL_SPEED                -2
#define SCROLL_TIME_MS              20

APP_TIMER_DEF(m_background_led_timer);
APP_TIMER_DEF(m_scroll_led_timer);

static bool m_background_led_running = false;

typedef struct {
    uint8_t index;
    float hue;
} bling_defrag_state_t;

static void __rgb_file_callback(uint8_t frame, void *p_data) {
    util_led_play_rgb_frame((led_anim_t *) p_data);
}

/**
 * Generic call back that runs the leds in sparkle mode
 */
static void __led_sparkle(uint8_t frame, void *p_data) {
    uint8_t *data = (uint8_t *) p_data;
    uint8_t i = util_math_rand8_max(LED_COUNT);
    util_led_set_rgb(i, LED_COLOR_WHITE);
    util_led_show();

    nrf_delay_ms(20);

    //Unpack hue as a float
    float hue = ((float) *data / 10.0);

    util_led_set_rgb(i, util_led_hsv_to_rgb(hue, 1, 1));
    util_led_show();

    hue += 0.1;
    if (hue >= 1) {
        hue = 0;
    }

    //Pack the hue
    *data = (uint8_t) (hue * 10.0);
}

/*
static void __led_sparkle_single(uint8_t frame, void *p_data) {
    uint8_t *data = (uint8_t *) p_data;
    uint8_t i = util_math_rand8_max(LED_COUNT);
    util_led_clear();

    util_led_set(i, 255, 255, 255);
    util_led_show();

    nrf_delay_ms(20);

    //Unpack hue as a float
    float hue = ((float) *data / 10.0);

    util_led_set_rgb(i, util_led_hsv_to_rgb(hue, 1, 1));
    util_led_show();

    hue += 0.1;
    if (hue >= 1) {
        hue = 0;
    }

    //Pack the hue
    *data = (uint8_t) (hue * 10.0);
}
*/

void __led_hue_cycle(uint8_t frame, void *p_data) {
    uint8_t *p_hue = (uint8_t *) p_data;
    float hue = (float) (*p_hue) / 100.0;
    uint32_t color = util_led_hsv_to_rgb(hue, .6, .8);
    util_led_set_all_rgb(color);
    util_led_show();

    hue += .015;
    if (hue >= 1) {
        hue = 0;
    }
    *p_hue = (hue * 100.0);
}

static void __spectrum_analyzer_callback(uint8_t frame, void *p_data) {

    if ((frame % 2) == 0) {
        uint32_t green = util_led_to_rgb(0, 255, 0);
        uint32_t yellow = util_led_to_rgb(255, 255, 0);
        uint32_t red = util_led_to_rgb(255, 0, 0);

        uint32_t colors[] = { green, yellow, red };

        uint8_t rows[LED_MATRIX_H][LED_MATRIX_W] = LED_MATRIX_ROWS;

        util_led_set_all(0, 0, 0);
        for (uint8_t x = 0; x < LED_MATRIX_W; x++) {
            uint8_t h = util_math_rand8_max(LED_MATRIX_H) + 1;
            for (uint8_t y = 0; y < h; y++) {
                util_led_set_rgb(rows[LED_MATRIX_H - y - 1][x], colors[y]);
            }
        }
        util_led_show();
    }
}

static void __mbp_bling_rainbow_eye_callback(uint8_t frame, void *data) {
    uint8_t *p_data = (uint8_t *) data;
    float hue = ((float) *p_data) / 100.0;

    uint32_t rgb = util_led_hsv_to_rgb(hue, 1.0, 1.0);
//    util_led_set_rgb(LED_RIGHT_EYE_INDEX, rgb);
    util_led_set_rgb(6, rgb);
    util_led_show();

    hue -= .01;
    if (hue <= 0) {
        hue = .99;
    }

    *p_data = (uint8_t) (hue * 100.0);
}

#define GLITTER_HANG_TIME 10
static uint8_t hang_time = GLITTER_HANG_TIME;

static void __mbp_bling_glitter_callback(uint8_t frame, void *p_data) {
    uint8_t *p_index = (uint8_t *) p_data;

    // If the stored data is a valid index, set that LED to blue or black
    if (*p_index < LED_COUNT) {
    // half the time
    if (hang_time == 0) {
        hang_time = GLITTER_HANG_TIME;
        //Pack invalid data so next time around a new LED is picked
        *p_index = 0xFF;
    } else {
        util_led_set_rgb(*p_index, LED_COLOR_BLACK);
        hang_time--;
    }

    //Otherwise if it's an invalid index randomly pick one to flash
    } else {
    *p_index = util_math_rand8_max(LED_COUNT);
    util_led_set_rgb(*p_index, LED_COLOR_WHITE);
    }

    util_led_show();
}

static void __mbp_bling_backer_abraxas3d_callback(uint8_t frame, void *p_data) {
    uint32_t indigo = LED_COLOR_INDIGO;
    uint32_t coral = LED_COLOR_CORAL;
    uint32_t gold = LED_COLOR_GOLD;
    uint32_t yellow = LED_COLOR_YELLOW;
    uint32_t white = LED_COLOR_WHITE;

    //Unpack cycle
    uint8_t *data = (uint8_t *) p_data;
    uint8_t cycle = (uint8_t) *data;

    if (cycle == 0) {
        //Move Column 1 & 4 Up Vertically
        util_led_set_rgb(13, indigo);
        util_led_set_rgb(0, gold);
        util_led_set_rgb(4, coral);
        util_led_set_rgb(8, yellow);
        util_led_set_rgb(12, indigo);
        util_led_set_rgb(3, gold);
        util_led_set_rgb(7, coral);
        util_led_set_rgb(11, yellow);

        //1-5-9 / 2-6-10
        util_led_set(1, 0, 0, 0); //CLEAR
        util_led_set(2, 0, 0, 0); //CLEAR
        util_led_set(6, 0, 0, 0); //CLEAR
        util_led_set(10, 0, 0, 0); //CLEAR
        util_led_set(9, 0, 0, 0); //CLEAR
        util_led_set(5, 0, 0, 0); //CLEAR
        util_led_set_rgb(1, indigo);
        util_led_set_rgb(5, indigo);
        util_led_set_rgb(9, indigo);
        util_led_set_rgb(14, indigo); //Set the Cig
    }

    else if (cycle == 1) {
        //Move Column 1 & 4 Up Vertically
        util_led_set_rgb(13, gold);
        util_led_set_rgb(0, coral);
        util_led_set_rgb(4, yellow);
        util_led_set_rgb(8, indigo);
        util_led_set_rgb(12, gold);
        util_led_set_rgb(3, coral);
        util_led_set_rgb(7, yellow);
        util_led_set_rgb(11, indigo);

        //1-5-9 / 2-6-10
        util_led_set(1, 0, 0, 0); //CLEAR
        util_led_set(2, 0, 0, 0); //CLEAR
        util_led_set(6, 0, 0, 0); //CLEAR
        util_led_set(10, 0, 0, 0); //CLEAR
        util_led_set(9, 0, 0, 0); //CLEAR
        util_led_set(5, 0, 0, 0); //CLEAR
        util_led_set_rgb(1, white);
        util_led_set_rgb(5, white);
        util_led_set_rgb(9, white);

        util_led_set_rgb(14, white); //Set the Cig
    }

    else if (cycle == 2) {
        //Move Column 1 & 4 Up Vertically
        util_led_set_rgb(13, coral);
        util_led_set_rgb(0, yellow);
        util_led_set_rgb(4, indigo);
        util_led_set_rgb(8, gold);
        util_led_set_rgb(12, coral);
        util_led_set_rgb(3, yellow);
        util_led_set_rgb(7, indigo);
        util_led_set_rgb(11, gold);

        //1-5-9 / 2-6-10
        util_led_set(1, 0, 0, 0); //CLEAR
        util_led_set(2, 0, 0, 0); //CLEAR
        util_led_set(6, 0, 0, 0); //CLEAR
        util_led_set(10, 0, 0, 0); //CLEAR
        util_led_set(9, 0, 0, 0); //CLEAR
        util_led_set(5, 0, 0, 0); //CLEAR
        util_led_set_rgb(2, indigo);
        util_led_set_rgb(6, indigo);
        util_led_set_rgb(10, indigo);
        util_led_set_rgb(14, indigo); //Set the Cig
    }

    else {
        //Move Column 1 & 4 Up Vertically
        util_led_set_rgb(13, yellow);
        util_led_set_rgb(0, indigo);
        util_led_set_rgb(4, gold);
        util_led_set_rgb(8, coral);
        util_led_set_rgb(12, yellow);
        util_led_set_rgb(3, indigo);
        util_led_set_rgb(7, gold);
        util_led_set_rgb(11, coral);

        //1-5-9 / 2-6-10
        util_led_set(1, 0, 0, 0); //CLEAR
        util_led_set(2, 0, 0, 0); //CLEAR
        util_led_set(6, 0, 0, 0); //CLEAR
        util_led_set(10, 0, 0, 0); //CLEAR
        util_led_set(9, 0, 0, 0); //CLEAR
        util_led_set(5, 0, 0, 0); //CLEAR
        util_led_set_rgb(2, white);
        util_led_set_rgb(6, white);
        util_led_set_rgb(10, white);
        util_led_set_rgb(14, white); //Set the Cig
    }

    util_led_show();
    nrf_delay_ms(30);

    //Increment the Cycle
    if (cycle < 3) {
        cycle++;
    }
    else {
        cycle = 0;
    }

    //Pack the cycle
    *data = (uint8_t) cycle;
}

static void __mbp_bling_backer_andnxor_callback(uint8_t frame, void *p_data) {
    if (frame < 128) {
        util_led_set_all(0, frame * 2, 0);
    } else if (frame < 200) {
        util_led_set(util_math_rand8_max(LED_COUNT), 255, 255, 255);
    } else {
        uint8_t b = (222 - frame) * 11;
        util_led_set_all(b, b, b);
    }

    util_led_show();
}

void simple_filebased_bling(char *rawfile, char *rgbfile) {
    util_led_clear();
    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file(rgbfile, &anim);
    util_gfx_draw_raw_file(rawfile, 0, 0, 128, 128, &__rgb_file_callback, true, (void *) &anim);
}

void mbp_bling_backer_abraxas3d(void *data) {
    uint8_t hue = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/BACKERS/KSABRAXA.RAW", 0, 0, 128, 128, &__mbp_bling_backer_abraxas3d_callback, true, &hue);
}

void mbp_bling_backer_andnxor(void *data) {
    uint8_t hue = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/BACKERS/ANDNXOR.RAW", 0, 0, 128, 128, &__mbp_bling_backer_andnxor_callback, true, &hue);
}

void mbp_bling_led_rainbow_callback(uint8_t frame, void *p_data) {
    //Unpack the data
    uint8_t *data = (uint8_t *) p_data;
    float hue = (float) *data / 100.0;

    //Define led matrix
    uint8_t led_map[LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT] = LED_MATRIX_FULL_MAPPING
    ;

    for (uint8_t row = 0; row < LED_MATRIX_FULL_ROW_COUNT; row++) {
        float rowhue = hue + (row * .08);
        if (rowhue >= 1)
            rowhue--;

        uint32_t color = util_led_hsv_to_rgb(rowhue, 1, .7);
        for (uint8_t i = 0; i < LED_MATRIX_FULL_COL_COUNT; i++) {
            uint8_t index = (row * LED_MATRIX_FULL_COL_COUNT) + i;
            if (led_map[index] < LED_COUNT) {
                util_led_set_rgb(led_map[index], color);
            }
        }

        //Increment row and color and loop around
        hue += .01;
        if (hue >= 1)
            hue = 0;
    }

    util_led_show();

    //Pack the data and store for next time
    *data = (int) (hue * 100);
}

// template
//void mbp_bling_fillthisin(void *data) { simple_filebased_bling("BLING/JOCO/DRWHOTIM.RAW", "BLING/TUNNEL.RGB");}

void mbp_bling_flames(void *data) { simple_filebased_bling("BLING/AND!XOR/FLAMES.RAW", "BLING/FLAMES.RGB"); }

void mbp_bling_hack_time(void *data) { simple_filebased_bling("BLING/AND!XOR/HACKTIME.RAW", "BLING/PINKBLUE.RGB"); }

void mbp_bling_16APSK(void *data) { simple_filebased_bling("BLING/TRANSIO/16APSK.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_ADA(void *data) { simple_filebased_bling("BLING/TRANSIO/ADA.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_ANIME(void *data) { simple_filebased_bling("BLING/TRANSIO/ANIME.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_ARRL(void *data) { simple_filebased_bling("BLING/TRANSIO/ARRL.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_CRT1(void *data) { simple_filebased_bling("BLING/TRANSIO/CRT1.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_HOMER(void *data) { simple_filebased_bling("BLING/TRANSIO/HOMER.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_HORN(void *data) { simple_filebased_bling("BLING/TRANSIO/HORN.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_KEY(void *data) { simple_filebased_bling("BLING/TRANSIO/KEY.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_KUHL(void *data) { simple_filebased_bling("BLING/TRANSIO/KUHL.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_MICK(void *data) { simple_filebased_bling("BLING/TRANSIO/MICK.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_MODEL(void *data) { simple_filebased_bling("BLING/TRANSIO/MODEL.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_NIXIE(void *data) { simple_filebased_bling("BLING/TRANSIO/NIXIE.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_NOISE(void *data) { simple_filebased_bling("BLING/TRANSIO/NOISE.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_OSCOPE(void *data) { simple_filebased_bling("BLING/TRANSIO/OSCOPE.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_PATTERN(void *data) { simple_filebased_bling("BLING/TRANSIO/PATTERN.RAW", "BLING/PINKBLUE.RGB"); }

//void mbp_bling_PRISM(void *data) { simple_filebased_bling("BLING/TRANSIO/PRISM.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_PRISM() {
    uint8_t hue = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/TRANSIO/PRISM.RAW", 0, 0, 128, 128, &__mbp_bling_glitter_callback, true, &hue);
}

void mbp_bling_RADIO(void *data) { simple_filebased_bling("BLING/TRANSIO/RADIO.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_RKO1(void *data) { simple_filebased_bling("BLING/TRANSIO/RKO1.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_RKO2(void *data) { simple_filebased_bling("BLING/TRANSIO/RKO2.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_ROTATE(void *data) { simple_filebased_bling("BLING/TRANSIO/ROTATE.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_SAT1(void *data) { simple_filebased_bling("BLING/TRANSIO/SAT1.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_SAT2(void *data) { simple_filebased_bling("BLING/TRANSIO/SAT2.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_SBOB(void *data) { simple_filebased_bling("BLING/TRANSIO/SBOB.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_SDR(void *data) { simple_filebased_bling("BLING/TRANSIO/SDR.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_SNOW(void *data) { simple_filebased_bling("BLING/TRANSIO/SNOW.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_SP1(void *data) { simple_filebased_bling("BLING/TRANSIO/SP1.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_SP2(void *data) { simple_filebased_bling("BLING/TRANSIO/SP2.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_SP3(void *data) { simple_filebased_bling("BLING/TRANSIO/SP3.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_SPECT1(void *data) { simple_filebased_bling("BLING/TRANSIO/SPECT1.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_SPECT2(void *data) { simple_filebased_bling("BLING/TRANSIO/SPECT2.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_TESLA(void *data) { simple_filebased_bling("BLING/TRANSIO/TESLA.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_TOWER(void *data) { simple_filebased_bling("BLING/TRANSIO/TOWER.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_TREK1(void *data) { simple_filebased_bling("BLING/TRANSIO/TREK1.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_TREK2(void *data) { simple_filebased_bling("BLING/TRANSIO/TREK2.RAW", "BLING/PINKBLUE.RGB"); }
void mbp_bling_YAGI(void *data) { simple_filebased_bling("BLING/TRANSIO/YAGI.RAW", "BLING/PINKBLUE.RGB"); }

void mbp_bling_illusion() {
    uint8_t index = 0;
    uint8_t count = 6;

    char *modes[] = { "BLING/AND!XOR/ILLUS3.RAW", "BLING/AND!XOR/ILLUS2.RAW", "BLING/AND!XOR/ILLUS1.RAW", "BLING/AND!XOR/ILLUS4.RAW",
            "BLING/AND!XOR/ILLUS5.RAW", "BLING/AND!XOR/ILLUS6.RAW" };
    uint8_t button = 0;

    util_led_clear();

    //If anything other than left button is pressed cycle modes
    while ((button & BUTTON_MASK_LEFT) == 0) {
        uint8_t hue = 0;
        button = util_gfx_draw_raw_file(modes[index], 0, 0, 128, 128, &__led_hue_cycle, true, &hue);
        index = (index + 1) % count;
    }
}

void mbp_bling_led_botnet(uint8_t frame, void *data) {
//
//  for (uint8_t i = 0; i < LED_COUNT; i++) {
//      led_rgb_t rgb = util_led_get(i);
//      if (rgb.red > 0) {
//          util_led_set(i, (rgb.red - 1), 0, 0);
//      }
//  }
    //Turn off an led
    util_led_set(util_math_rand8_max(LED_COUNT), 0, 0, 0);

    //Turn on an led
    uint8_t red = util_math_rand8_max(100) + 155;
    uint8_t i = util_math_rand8_max(LED_COUNT);
    util_led_set(i, red, 0, 0);
    util_led_show();
}

static void __matrix_callback(uint8_t frame, void *p_data) {
    uint8_t brightness[5][4];
    memcpy(brightness, p_data, 5 * 4);
    if ((frame % 5) == 0) {

        //Move drops down
        for (uint8_t x = 0; x < 5; x++) {
            for (uint8_t y = 0; y < 4; y++) {
                if (brightness[x][y] == 240) {
                    brightness[x][y] = 190;
                    brightness[x][(y + 1) % 4] = 240;
                    brightness[x][(y + 2) % 4] = 0;
                    brightness[x][(y + 3) % 4] = 100;
                    break;
                }
            }
        }

        //Map XY to LED indices
        uint8_t mapping[LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT] = LED_MATRIX_FULL_MAPPING;
        for (uint8_t x = 0; x < LED_MATRIX_FULL_COL_COUNT; x++) {
            for (uint8_t y = 0; y < LED_MATRIX_FULL_ROW_COUNT; y++) {
                uint8_t index = mapping[(y * LED_MATRIX_FULL_COL_COUNT) + x];
                if (index < LED_COUNT) {
                    util_led_set(index, 0, brightness[x][y], 0);
                }
            }
        }

        util_led_show();
    }

    memcpy(p_data, brightness, 5 * 4);
}

void mbp_bling_matrix() {
    util_led_clear();
    uint8_t brightness[5][4];
    for (uint8_t x = 0; x < 5; x++) {
        uint8_t r = util_math_rand8_max(4);
        brightness[x][r] = 240;
    }
    util_gfx_draw_raw_file("BLING/AND!XOR/MATRIX.RAW", 0, 0, 128, 128, &__matrix_callback, true, (void *) &brightness);
}

void mbp_bling_nyan() {
    util_led_clear();
    uint8_t hue = 0; //hue is normally a float 0 to 1, pack it in an 8 bit int
    util_gfx_draw_raw_file("BLING/AND!XOR/NAYAN.RAW", 0, 0, 128, 128, &__led_sparkle, true, &hue);
}

void mbp_bling_owl() {
    uint8_t hue = 0;
    uint8_t index = 0;
    uint8_t count = 3;

    char *modes[] = { "BLING/AND!XOR/OWL1.RAW", "BLING/AND!XOR/OWL2.RAW", "BLING/AND!XOR/OWL3.RAW" };
    uint8_t button = 0;

    util_led_clear();

    //If anything other than left button is pressed cycle modes
    while ((button & BUTTON_MASK_LEFT) == 0) {
        button = util_gfx_draw_raw_file(modes[index], 0, 0, 128, 128, &__mbp_bling_rainbow_eye_callback, true, &hue);
        index = (index + 1) % count;
    }
}

void mbp_bling_pirate() {
    util_led_clear();
    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file("BLING/GOLD.RGB", &anim);

    uint8_t button = 0;
    //Prevent escape from bling (so we can catch action button)
    while ((button & BUTTON_MASK_LEFT) == 0) {
        button = util_gfx_draw_raw_file("BLING/AND!XOR/PIRATES.RAW", 0, 0, 128, 128, &__rgb_file_callback, true, (void *) &anim);
    }
}

void mbp_bling_rickroll() {
    util_led_clear();
    util_gfx_draw_raw_file("/BLING/AND!XOR/RICKROLL.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, &__spectrum_analyzer_callback, true, NULL);
}

uint8_t mbp_bling_scroll(char *text, bool loop) {
    util_gfx_invalidate();

    //Make sure all scroll text is upper case
    for (uint8_t i = 0; i < strlen(text); i++) {
        text[i] = toupper((int)text[i]);
    }
    int16_t y = (GFX_HEIGHT - SCROLL_CHAR_HEIGHT) / 2;
    int16_t x = GFX_WIDTH;

    //" ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.!?,()[]{}<>/\\|;:&^%$#@*-_+"
    char *files[] = {
            "SCROLL/SPACE.FNT",
            "SCROLL/A.FNT",
            "SCROLL/B.FNT",
            "SCROLL/C.FNT",
            "SCROLL/D.FNT",
            "SCROLL/E.FNT",
            "SCROLL/F.FNT",
            "SCROLL/G.FNT",
            "SCROLL/H.FNT",
            "SCROLL/I.FNT",
            "SCROLL/J.FNT",
            "SCROLL/K.FNT",
            "SCROLL/L.FNT",
            "SCROLL/M.FNT",
            "SCROLL/N.FNT",
            "SCROLL/O.FNT",
            "SCROLL/P.FNT",
            "SCROLL/Q.FNT",
            "SCROLL/R.FNT",
            "SCROLL/S.FNT",
            "SCROLL/T.FNT",
            "SCROLL/U.FNT",
            "SCROLL/V.FNT",
            "SCROLL/W.FNT",
            "SCROLL/X.FNT",
            "SCROLL/Y.FNT",
            "SCROLL/Z.FNT",
            "SCROLL/0.FNT",
            "SCROLL/1.FNT",
            "SCROLL/2.FNT",
            "SCROLL/3.FNT",
            "SCROLL/4.FNT",
            "SCROLL/5.FNT",
            "SCROLL/6.FNT",
            "SCROLL/7.FNT",
            "SCROLL/8.FNT",
            "SCROLL/9.FNT",
            "SCROLL/PERIOD.FNT",
            "SCROLL/EXCL.FNT",
            "SCROLL/QUESTION.FNT",
            "SCROLL/COMMA.FNT",
            "SCROLL/LPAREN.FNT",
            "SCROLL/RPAREN.FNT",
            "SCROLL/LBRACKET.FNT",
            "SCROLL/RBRACKET.FNT",
            "SCROLL/LBRACE.FNT",
            "SCROLL/RBRACE.FNT",
            "SCROLL/LT.FNT",
            "SCROLL/GT.FNT",
            "SCROLL/FSLASH.FNT",
            "SCROLL/BSLASH.FNT",
            "SCROLL/PIPE.FNT",
            "SCROLL/SEMI.FNT",
            "SCROLL/COLON.FNT",
            "SCROLL/AMP.FNT",
            "SCROLL/CARROT.FNT",
            "SCROLL/PCT.FNT",
            "SCROLL/DOLLAR.FNT",
            "SCROLL/HASH.FNT",
            "SCROLL/AT.FNT",
            "SCROLL/STAR.FNT",
            "SCROLL/DASH.FNT",
            "SCROLL/USCORE.FNT",
            "SCROLL/PLUS.FNT"
    };

    while (1) {
        if (!util_gfx_is_valid_state()) {
            mbp_ui_cls();
        }

        util_gfx_fill_rect(0, y, SCROLL_CHAR_WIDTH, SCROLL_CHAR_HEIGHT, COLOR_BLACK);

        for (uint8_t i = 0; i < strlen(text); i++) {
            const char *ptr = strchr(INPUT_CHARS, text[i]);

            if (ptr) {
                uint16_t xx = x + (i * SCROLL_CHAR_WIDTH);
                int index = ptr - INPUT_CHARS;

                if (xx > 0 && xx < (GFX_WIDTH - SCROLL_CHAR_WIDTH))
                    util_gfx_draw_raw_file(files[index], xx, y, SCROLL_CHAR_WIDTH, SCROLL_CHAR_HEIGHT, NULL, false, NULL);
            }
        }
        util_gfx_validate();

        x += SCROLL_SPEED;

        int16_t x_min = 0 - (SCROLL_CHAR_WIDTH * strlen(text));
        //If we run off the left edge, loop around maybe
        if (x < x_min) {
            if (!loop)
                break;
            x = GFX_WIDTH;
        }

        if ((util_button_action() && loop) || util_button_left()) {
            uint8_t button = util_button_state();
            util_button_clear();
            return button;
        }

        app_sched_execute();
        nrf_delay_ms(SCROLL_TIME_MS);
    }

    util_gfx_invalidate();
    uint8_t button = util_button_state();
    util_button_clear();
    return button;
}

static void __scroll_callback(void *p_data) {
    led_anim_t *p_anim = (led_anim_t *) p_data;
    util_led_play_rgb_frame(p_anim);
}

void mbp_bling_scroll_cycle() {
    util_led_clear();
    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file("BLING/KIT.RGB", &anim);

    //Get the usesrname
    char name[SETTING_NAME_LENGTH];
    mbp_state_name_get(name);

    uint8_t index = 0;
    uint8_t count = 3;
    char *messages[] = { "TRANS-IONOSPHERIC ", "STINKING BADGES ", name };

    //Start up led timer for scroll
    APP_ERROR_CHECK(app_timer_create(&m_scroll_led_timer, APP_TIMER_MODE_REPEATED, __scroll_callback));
    APP_ERROR_CHECK(app_timer_start(m_scroll_led_timer, APP_TIMER_TICKS(1000/20, UTIL_TIMER_PRESCALER), &anim));

    while (1) {
        uint8_t button = mbp_bling_scroll(messages[index], false);
        index = (index + 1) % count;

        if ((button & BUTTON_MASK_LEFT) > 0) {
            break;
        }
    }

    app_timer_stop(m_scroll_led_timer);

    util_gfx_invalidate();
    util_button_clear();
}

// void mbp_bling_score_schedule_handler(void * p_event_data, uint16_t event_size) {
//     char *name = (char *) p_event_data;
//     uint16_t w, h;
//     app_sched_pause();
//     bool background_was_running = mbp_background_led_running();
//     mbp_background_led_stop();
//
//     UTIL_LED_ANIM_INIT(anim);
//     util_led_load_rgb_file("BLING/GRNBLUE.RGB", &anim);
//     util_gfx_draw_raw_file("MBSCORE.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);
//
//     uint16_t fg = util_gfx_rgb_to_565(COLOR_BLACK);
//
//     //Compute name coords
//     util_gfx_set_font(FONT_LARGE);
//     util_gfx_get_text_bounds(name, 0, 0, &w, &h);
//     uint16_t y = 80;
//     uint16_t x = (GFX_WIDTH - w) / 2;
//
//     //Print name
//     util_gfx_set_color(fg);
//     util_gfx_set_cursor(x, y);
//     util_gfx_print(name);
//
//     // 6 second score time (20 FPS)
//     for (uint16_t i = 0; i < (20 * SCORE_DISPLAY_TIME); i++) {
//         util_led_play_rgb_frame(&anim);
//         nrf_delay_ms(50);
//     }
//
//     //Cleanup and give control back to user
//     util_gfx_invalidate();
//     if (background_was_running) {
//         mbp_background_led_start();
//     }
//     app_sched_resume();
//     util_button_clear();
// }

static void __mbp_bling_twitter_callback(uint8_t frame, void *p_data) {
    uint8_t *p_index = (uint8_t *) p_data;

    //If the stored data is a valid index, set that LED to blue or black
    if (*p_index < LED_COUNT) {
        if (util_math_rand8_max(2) == 0) {
            util_led_set(*p_index, 29, 161, 243);
        } else {
            util_led_set_rgb(*p_index, LED_COLOR_BLACK);
        }

        //Pack invalid data so next time around a new LED is picked
        *p_index = 0xFF;
    }
    //Otherwise if it's an invalid index randomly pick one to flash
    else {
        *p_index = util_math_rand8_max(LED_COUNT);
        util_led_set_rgb(*p_index, LED_COLOR_WHITE);
    }

    util_led_show();
}

void mbp_bling_twitter() {
    util_led_clear();
    uint8_t index = 0xFF; //set index out of bounds of led count
    util_gfx_draw_raw_file("BLING/TWITTER/TWITTER.RAW", 0, 0, 128, 128, &__mbp_bling_twitter_callback, true, &index);
}

static void __mbp_defrag_callback(uint8_t frame, void *p_data) {
    bling_defrag_state_t *p_defrag = (bling_defrag_state_t *) p_data;
    uint8_t count = (LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT);
    uint8_t led_mapping[LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT] = LED_MATRIX_FULL_MAPPING
    ;
    uint32_t rgb = util_led_hsv_to_rgb(p_defrag->hue, 1, .8);
    uint8_t index = led_mapping[p_defrag->index];

    if (index < LED_COUNT) {
        util_led_set_rgb(index, rgb);
        util_led_show();
    }

    p_defrag->index++;
    if (p_defrag->index >= count) {
        p_defrag->index = 0;
        p_defrag->hue += .05;

        if (p_defrag->hue >= 1) {
            p_defrag->hue = 0;
        }
    }
}

void mbp_bling_defrag() {
    bling_defrag_state_t defrag;
    defrag.hue = 0;
    defrag.index = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/AND!XOR/DEFRAG.RAW", 0, 0, 128, 128, &__mbp_defrag_callback, true, &defrag);
}

// called every BACKGROUND_LED_TIME_MS
static void __background_led_sch_handler(void * p_event_data, uint16_t event_size) {
	static int bg_cycle_count = 0;

	// Background LEDs intended to simulate Phase 4 Ground status display:
	if (bg_cycle_count == 3 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(8, LED_COLOR_GREEN);
	} else if (bg_cycle_count == 4 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(9, LED_COLOR_RED);
		util_led_set_rgb(11, LED_COLOR_RED);
	} else if (bg_cycle_count == 7* BG_TICKS_PER_SECOND) {
		util_led_set_rgb(9, LED_COLOR_LIGHTBLUE);
		util_led_set_rgb(11, LED_COLOR_RED);
	} else if (bg_cycle_count == 8 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(7, LED_COLOR_GREEN);
	} else if (bg_cycle_count == 10 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(12, LED_COLOR_GREEN);
	} else if (bg_cycle_count == 11 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(9, LED_COLOR_GREEN);
		util_led_set_rgb(11, LED_COLOR_GREEN);
	} else if (bg_cycle_count == 14 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(13, LED_COLOR_ORANGE);
	} else if (bg_cycle_count == 15 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(6, LED_COLOR_GREEN);
	} else if (bg_cycle_count == 16 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(5, LED_COLOR_RED);
	} else if (bg_cycle_count == 17 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(14, LED_COLOR_RED);
	} else if (bg_cycle_count == 19 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(5, LED_COLOR_BLACK);
		util_led_set_rgb(6, LED_COLOR_BLACK);
		util_led_set_rgb(13, LED_COLOR_GREEN);
		util_led_set_rgb(14, LED_COLOR_BLACK);
	} else if (bg_cycle_count == 20 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(6, LED_COLOR_GREEN);
		util_led_set_rgb(13, LED_COLOR_GREEN);
	} else if (bg_cycle_count == 22 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(5, LED_COLOR_GREEN);
	} else if (bg_cycle_count == 23 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(14, LED_COLOR_GREEN);
	} else if (bg_cycle_count == 25 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(0, LED_COLOR_BLUE);
		util_led_set_rgb(5, LED_COLOR_BLACK);
		util_led_set_rgb(6, LED_COLOR_BLACK);
		util_led_set_rgb(7, LED_COLOR_BLACK);
		util_led_set_rgb(10, LED_COLOR_BLUE);
		util_led_set_rgb(13, LED_COLOR_BLACK);
		util_led_set_rgb(14, LED_COLOR_BLACK);
	} else if (bg_cycle_count == 28 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(1, LED_COLOR_PURPLE);
	} else if (bg_cycle_count == 29 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(2, LED_COLOR_PURPLE);
	} else if (bg_cycle_count == 30 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(3, LED_COLOR_PURPLE);
	} else if (bg_cycle_count == 31 * BG_TICKS_PER_SECOND) {
		util_led_set_rgb(4, LED_COLOR_PURPLE);
	} else if (bg_cycle_count == 35 * BG_TICKS_PER_SECOND) {
		util_led_set_all_rgb(LED_COLOR_BLACK);

		// Simulated radio cycle now complete. Good time to HELLO neighbors!
		app_sched_event_put(NULL, 0, hello_background_handler);

	} else if (bg_cycle_count >= 38 * BG_TICKS_PER_SECOND) {
		bg_cycle_count = 0;
	}

    util_led_show();
	bg_cycle_count++;

	// If computations are extensive, do them here, not before
	// updating the display.

}

static void __background_led_timer_handler(void *p_data) {
    app_sched_event_put(NULL, 0, __background_led_sch_handler);
}

bool mbp_background_led_running() {
    return m_background_led_running;
}

void mbp_background_led_start() {
    if (!m_background_led_running) {
        //Start up timer for updating the background LED display
        APP_ERROR_CHECK(app_timer_create(&m_background_led_timer, APP_TIMER_MODE_REPEATED, __background_led_timer_handler));
        APP_ERROR_CHECK(app_timer_start(m_background_led_timer, APP_TIMER_TICKS(BACKGROUND_LED_TIME_MS, UTIL_TIMER_PRESCALER), NULL));

        m_background_led_running = true;
    }
}

void mbp_background_led_stop() {
    if (m_background_led_running) {
        util_led_set_all_rgb(LED_COLOR_BLACK);
        util_led_show();
        APP_ERROR_CHECK(app_timer_stop(m_background_led_timer));

        m_background_led_running = false;
    }
}
