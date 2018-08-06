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
#ifndef TRANSIO_QSO_H_
#define TRANSIO_QSO_H_


extern void transio_callsign_edit();
extern void transio_qso_callsign_set(char *callsign);
extern void transio_qso_callsign_update(void);
extern uint32_t transio_qso_ble_init(void);
extern void transio_qso_on_ble_evt(const ble_evt_t * p_ble_evt);
extern void transio_qso_on_db_disc_evt(const ble_db_discovery_evt_t * p_evt);

// Attempt a QSO with a neighbor badge, by index from the sorted_list.
extern void transio_qso_attempt(uint8_t index);

#endif /* TRANSIO_QSO_H_ */
