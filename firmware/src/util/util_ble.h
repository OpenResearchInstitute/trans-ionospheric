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
#ifndef UTIL_UTIL_BLE_H_
#define UTIL_UTIL_BLE_H_

#define COMPANY_ID_TRANSIO_TMP				0x0858	// Fake ID used at Hamvention 2018
#define COMPANY_ID_TRANSIO					0x064A	// Open Research Institute, Inc
#define COMPANY_ID_ANDNXOR					0x049E	// assigned to AND!XOR LLC :-)
#define COMPANY_ID_JOCO						0x0B25	// Fake ID used for JoCo 2018
#define COMPANY_ID_CPV						0x0C97	// Fake; Crypto Privacy Village
#define COMPANY_ID_DC503					0x0503	// assigned to Locoroll, Inc
#define COMPANY_ID_DC801					0x0801	// Fake, DC801??
#define COMPANY_ID_QUEERCON					0x04D3	// assigned to Queercon, Inc
#define COMPANY_ID_DCDARKNET				0x444E	// Fake
#define COMPANY_ID_DCZIA					0x5050	// Fake
#define COMPANY_ID_FoB1un7					0x0069	// Fake; Friends of B1un7
#define COMPANY_ID_TDI						0x1010	// Fake; The Diana Initiative
#define COMPANY_ID_DCFURS					0x71FF	// Fake
#define COMPANY_ID_BLINKYBLING				0x4242	// Fake

#define APPEARANCE_ID_ANDNXOR_DC25				0x19DC
#define APPEARANCE_ID_STANDARD_DC26				0x26DC

extern void util_ble_advertising_start();
extern void util_ble_score_update();
extern uint32_t util_ble_connect(ble_gap_addr_t *p_address);
extern uint32_t util_ble_disconnect();
extern void util_ble_c2_set(master_c2_t *p_c2);
extern void util_ble_init();
extern void util_ble_name_get(char *name);
extern void util_ble_name_set(char *name);
extern void util_ble_flags_set(void);
extern uint32_t util_ble_nus_send(char *p_string, uint16_t length);
extern void util_ble_off();
extern void util_ble_on();
extern void util_ble_scan_start();

#endif /* UTIL_UTIL_BLE_H_ */
