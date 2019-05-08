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
 * 	@sconklin
 * 	@mustbeart
 * 	@abraxas3d
 *****************************************************************************/
#ifndef TRANSIO_RADIO_H_
#define TRANSIO_RADIO_H_


#define TRANSIO_RADIO_MSD_LEN   16

extern void transio_radio_screen();
extern void transio_radio_process_advertisement(uint8_t address[],
										 char name[],
										 uint16_t appearance,
                                         uint8_t msd_length,
										 uint8_t mfg_specific_data[],
										 int8_t rssi);

#endif /* TRANSIO_RADIO_H_ */
