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
 *      @mustbeart
 *      @abraxas3d
 *
 *****************************************************************************/

#include "system.h"

#define	RSSI_NOTHING_HEARD	(-127)

#define MIN_BARGRAPH_RSSI	(-106)
#define DB_PER_BAR 3

static int8_t m_rssi_summary = RSSI_NOTHING_HEARD;
static uint32_t m_rssi_term_end_time = 0;

APP_TIMER_DEF(m_rssi_timer);

void mbp_rssi_badge_heard(uint16_t device_id, int8_t rssi) {

	// RSSI summary is the strongest signal heard this period.
	if (rssi > m_rssi_summary) {
		m_rssi_summary = rssi;
	}

	// If RSSI monitoring on the terminal is active, monitor this one
	if (m_rssi_term_end_time > 0) {
		mbp_term_display_rssi(device_id, rssi);
	}
}

static void report_rssi(int8_t rssi) {
	uint8_t level;

	if (rssi < MIN_BARGRAPH_RSSI) {
		level = 0;
	} else {
		level = (rssi - MIN_BARGRAPH_RSSI) / DB_PER_BAR;
	}

	// send RSSI summary to the bargraph display
	util_i2c_smeter_write(level);
}

static void __rssi_timer_handler(void *p_data) {

	// Report the RSSI summary for this tick
	report_rssi(m_rssi_summary);
	m_rssi_summary = RSSI_NOTHING_HEARD;

	// Check for the end of Terminal RSSI monitoring
	if (m_rssi_term_end_time > 0 && util_millis() > m_rssi_term_end_time) {
		m_rssi_term_end_time = 0;
		mbp_term_end_rssi();
	}
}

void mbp_rssi_start(void) {
	uint32_t err_code;

	m_rssi_summary = RSSI_NOTHING_HEARD;

	if (util_i2c_smeter_start()) {
		err_code = app_timer_create(&m_rssi_timer, APP_TIMER_MODE_REPEATED, __rssi_timer_handler);
		APP_ERROR_CHECK(err_code);
		err_code = app_timer_start(m_rssi_timer, APP_TIMER_TICKS(100), NULL);
		APP_ERROR_CHECK(err_code);
	}
}

void mbp_rssi_stop(void) {
	app_timer_stop(m_rssi_timer);
}

void mbp_rssi_term_duration(unsigned int duration) {
	if (duration == 0) {
		if (m_rssi_term_end_time != 0) {
			m_rssi_term_end_time = 1;	// non-zero but in the past (expired)
		}
	} else {
		m_rssi_term_end_time = util_millis() + 1000 * (uint32_t) duration;
	}
}
