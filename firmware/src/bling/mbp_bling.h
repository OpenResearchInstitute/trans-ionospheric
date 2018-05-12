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
#ifndef MBP_ANIM_H_
#define MBP_ANIM_H_

extern void simple_filebased_bling(char *rawfile, char *rgbfile);
extern void mbp_bling_backer_abraxas3d(void *data);
extern void mbp_bling_backer_andnxor(void *data);
extern void mbp_bling_defrag();
extern void mbp_bling_flames(void *data);
extern void mbp_bling_illusion();
extern void mbp_bling_led_botnet(uint8_t frame, void *data);
extern void mbp_bling_led_rainbow_callback(uint8_t frame, void *p_data);
extern void mbp_bling_matrix();
extern void mbp_bling_nyan();
extern void mbp_bling_hack_time();
extern void mbp_bling_owl();
extern void mbp_bling_pirate();
extern void mbp_bling_rickroll();
extern uint8_t mbp_bling_scroll(char *text, bool loop);
extern void mbp_bling_scroll_cycle();
extern void mbp_bling_score_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_bender_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_joco_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_cpv_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_dc503_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_dc801_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_queercon_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_twitter();
//
extern void mbp_bling_twilight_zone();

extern void mbp_bling_16APSK();
extern void mbp_bling_ADA();
extern void mbp_bling_ANIME();
extern void mbp_bling_ARRL();
extern void mbp_bling_CRT1();
extern void mbp_bling_HOMER();
extern void mbp_bling_HORN();
extern void mbp_bling_KEY();
extern void mbp_bling_KUHL();
extern void mbp_bling_MICK();
extern void mbp_bling_MODEL();
extern void mbp_bling_NIXIE();
extern void mbp_bling_NOISE();
extern void mbp_bling_OSCOPE();
extern void mbp_bling_PATTERN();
extern void mbp_bling_PRISM();
extern void mbp_bling_RADIO();
extern void mbp_bling_RKO1();
extern void mbp_bling_RKO2();
extern void mbp_bling_ROTATE();
extern void mbp_bling_SAT1();
extern void mbp_bling_SAT2();
extern void mbp_bling_SBOB();
extern void mbp_bling_SDR();
extern void mbp_bling_SNOW();
extern void mbp_bling_SP1();
extern void mbp_bling_SP2();
extern void mbp_bling_SP3();
extern void mbp_bling_SPECT1();
extern void mbp_bling_SPECT2();
extern void mbp_bling_TESLA();
extern void mbp_bling_TOWER();
extern void mbp_bling_TREK1();
extern void mbp_bling_TREK2();
extern void mbp_bling_YAGI();

//
extern bool mbp_background_led_running();
extern void mbp_background_led_start();
extern void mbp_background_led_stop();

#endif /* MBP_ANIM_H_ */
