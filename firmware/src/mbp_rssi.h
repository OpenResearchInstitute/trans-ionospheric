/*****************************************************************************
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
 *      @sconklin
 *      @mustbeart
 *
 *****************************************************************************/
#ifndef MBP_RSSI_H_
#define MBP_RSSI_H_

extern void mbp_rssi_badge_heard(uint16_t device_id, uint8_t rssi);
extern void mbp_rssi_start(void);
extern void mbp_rssi_term_duration(unsigned int duration);

#endif /* MBP_RSSI_H_ */
