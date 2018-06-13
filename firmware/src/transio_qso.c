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

#define QSO_SVC_UUID			0xf264
#define	QSO_CHAR_CALLSIGN_UUID	0xc91d	// read the badge's callsign

static uint8_t m_qso_callsign[SETTING_CALLSIGN_LENGTH] = "NOTSET";

static uint16_t m_qso_service_handle;
static volatile uint16_t m2_s_conn_handle = BLE_CONN_HANDLE_INVALID;
static volatile uint16_t m2_c_conn_handle = BLE_CONN_HANDLE_INVALID;
static volatile uint16_t m2_c_callsign_handle;
static uint8_t m2_c_callsign_result[SETTING_CALLSIGN_LENGTH];
static volatile bool m2_connecting = false;
static volatile bool m2_waiting = false;
static volatile bool m_callsign_read_ok = false;

void transio_qso_callsign_set(char *callsign) {
	memcpy(m_qso_callsign, callsign, SETTING_CALLSIGN_LENGTH);
}


/**
 * Initialize the callsign characteristic
 * This reads the badge's callsign. No security here, we trust hams.
 */
static uint32_t __init_char_callsign() {
	ble_gatts_char_md_t char_md;
	ble_gatts_attr_t attr_char_value;
	ble_uuid_t ble_uuid;
	ble_gatts_attr_md_t attr_md;
	ble_gatts_char_handles_t char_handles;

	memset(&char_md, 0, sizeof(char_md));

	//Badge callsign characteristic
	char_md.char_props.write = 0;			//Writing not allowed
	char_md.char_props.write_wo_resp = 0;	//Writing not allowed
	char_md.char_props.read = 1;	//Read the data
	char_md.p_char_user_desc = NULL;
	char_md.p_char_pf = NULL;
	char_md.p_user_desc_md = NULL;
	char_md.p_cccd_md = NULL;
	char_md.p_sccd_md = NULL;

	BLE_UUID_BLE_ASSIGN(ble_uuid, QSO_CHAR_CALLSIGN_UUID);

	memset(&attr_md, 0, sizeof(attr_md));

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
	attr_md.vloc = BLE_GATTS_VLOC_USER;
	attr_md.rd_auth = 0;
	attr_md.wr_auth = 0;
	attr_md.vlen = 0;

	memset(&attr_char_value, 0, sizeof(attr_char_value));

	mbp_state_callsign_get((char *)m_qso_callsign);

	attr_char_value.p_uuid = &ble_uuid;
	attr_char_value.p_attr_md = &attr_md;
	attr_char_value.init_len = SETTING_CALLSIGN_LENGTH;
	attr_char_value.init_offs = 0;
	attr_char_value.max_len = SETTING_CALLSIGN_LENGTH;
	attr_char_value.p_value = m_qso_callsign;

	uint32_t result = sd_ble_gatts_characteristic_add(m_qso_service_handle, &char_md, &attr_char_value, &char_handles);
	return result;
}


/**
 * Initialization for QSO services under BLE
 */
uint32_t transio_qso_ble_init(void) {
	uint32_t err_code;
	ble_uuid_t ble_uuid;

	// Add service
	BLE_UUID_BLE_ASSIGN(ble_uuid, QSO_SVC_UUID);
	err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &m_qso_service_handle);
	if (err_code != NRF_SUCCESS) {
		return err_code;
	}

	//Register with discovery db so we can talk to GATT
	APP_ERROR_CHECK(ble_db_discovery_evt_register(&ble_uuid));

	__init_char_callsign();

	return err_code;
}


static void __on_read_response(const ble_evt_t *p_ble_evt) {
	const ble_gattc_evt_read_rsp_t * p_response;

	// Check if the event if on the link for this instance
	if (m2_c_conn_handle != p_ble_evt->evt.gattc_evt.conn_handle) {
		return;
	}

	p_response = &p_ble_evt->evt.gattc_evt.params.read_rsp;

	//Check the handle of the response and pull the data out
	if (p_response->handle == m2_c_callsign_handle) {
		memcpy(m2_c_callsign_result, p_response->data, SETTING_CALLSIGN_LENGTH);
		m_callsign_read_ok = true;
		m2_waiting = false;
	}
}



bool transio_qso_callsign_get(void) {
	m2_waiting = true;
	m_callsign_read_ok = false;

	//Fire off a read request
	sd_ble_gattc_read(m2_c_conn_handle, m2_c_callsign_handle, 0);

	uint32_t end_time = util_millis() + 5000;
	while (m2_waiting) {
		APP_ERROR_CHECK(sd_app_evt_wait());
		if (util_millis() > end_time) {
			m2_waiting = false;
		}
	}

	return m_callsign_read_ok;
}


void transio_qso_on_ble_evt(const ble_evt_t * p_ble_evt) {
	if (p_ble_evt == NULL) {
		return;
	}

	switch (p_ble_evt->header.evt_id) {
	//Connected to as a service
		case BLE_GAP_EVT_CONNECTED:
			m2_s_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
			break;
		case BLE_GAP_EVT_DISCONNECTED:
			m2_connecting = false;
			m2_s_conn_handle = BLE_CONN_HANDLE_INVALID;
			m2_c_conn_handle = BLE_CONN_HANDLE_INVALID;
			break;
		case BLE_GAP_EVT_TIMEOUT:
			m2_connecting = false;
			m2_s_conn_handle = BLE_CONN_HANDLE_INVALID;
			m2_c_conn_handle = BLE_CONN_HANDLE_INVALID;
			break;

		case BLE_GATTC_EVT_HVX:
			break;

		case BLE_GATTC_EVT_WRITE_RSP:
			break;

		case BLE_GATTC_EVT_READ_RSP:
			__on_read_response(p_ble_evt);
			break;

		case BLE_GATTS_EVT_WRITE:
			break;

		case BLE_EVT_USER_MEM_REQUEST:
			break;

		case BLE_EVT_USER_MEM_RELEASE:
			break;

		default:
			break;
	}
}


/**
 * BLE discovery event handler
 */
void transio_qso_on_db_disc_evt(const ble_db_discovery_evt_t * p_evt) {
	// Check if the callsign Service was discovered.
	if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE
			&& p_evt->params.discovered_db.srv_uuid.uuid == QSO_SVC_UUID
			&& p_evt->params.discovered_db.srv_uuid.type == BLE_UUID_TYPE_BLE) {

		m2_c_conn_handle = p_evt->conn_handle;

		for (uint8_t i = 0; i < p_evt->params.discovered_db.char_count; i++) {

			if (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid == QSO_CHAR_CALLSIGN_UUID) {
				// Found callsign characteristic
				m2_c_callsign_handle = p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
			}
		}

		APP_ERROR_CHECK(pm_conn_secure(m2_c_conn_handle, false));

		//all done
		m2_connecting = false;
	}
}


bool transio_qso_connect_blocking(ble_gap_addr_t *address) {
	m2_connecting = true;
	m2_c_conn_handle = BLE_CONN_HANDLE_INVALID;
	util_ble_connect(address);
	while (m2_connecting) {
		APP_ERROR_CHECK(sd_app_evt_wait());
	}

	return m2_c_conn_handle != BLE_CONN_HANDLE_INVALID;
}


void transio_qso_attempt() {
	ble_gap_addr_t address;
	// Hard coded address to use, pending improvements to the neighbor list.
	// Note: byte order is the opposite order displayed in nRF Connect, etc.
//	uint8_t raw_addr[] = { 0x74, 0x50, 0xb9, 0xe1, 0x74, 0xc5};	// some badge
	uint8_t raw_addr[] = { 0x80, 0xe7, 0x2b, 0x10, 0x9c, 0xc7};	// Proto 7
//	uint8_t raw_addr[] = { 0x06, 0x26, 0x79, 0x86, 0x50, 0xe5};	// Abraxas3D Bender
	memcpy(address.addr, raw_addr, 6);
	address.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;

	char buf[32];

	//Connect to the selected badge
	mbp_ui_cls();
	util_gfx_set_cursor(0, 0);
	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_color(COLOR_WHITE);
	util_gfx_print("Calling the badge...\ncome in, please!\n");

	//Connect
	if (transio_qso_connect_blocking(&address)) {
		util_gfx_print("*** CONNECTED\n");

		if (transio_qso_callsign_get()) {
			util_gfx_print("UR 599 TNX QSO\n");
			//!!! update neighbor list entry with callsign
			//!!! make logfile entry
			sprintf(buf, "His callsign: %s\n", m2_c_callsign_result);
		} else {
			util_gfx_print("Nothing heard!\nTry again later.\n");
		}
	}
	util_ble_disconnect();
	nrf_delay_ms(1500);
	util_gfx_print("*** DISCONNECTED\n\nAny key to continue.");

	util_button_wait();
	util_button_clear();
}


void transio_callsign_edit(void) {
	char call[SETTING_CALLSIGN_LENGTH];
	bool already_set = mbp_state_callsign_get(call);
	char message[64];

	if (already_set) {
		//Ask if they want to clear
		sprintf(message, "Callsign is '%s', edit existing or clear?", call);
		if (mbp_ui_toggle_popup("Callsign", 0, "Edit", "Clear", message) == 1) {
			memset(call, 0, SETTING_CALLSIGN_LENGTH);
		}
	} else {		// first entry; start editing from a blank string.
		memset(call, 0, SETTING_CALLSIGN_LENGTH);
	}

//Edit the name
	mbp_ui_input("Callsign", "Enter Callsign:", call, SETTING_CALLSIGN_LENGTH - 1, false);

	sprintf(message, "Change callsign to: '%s'?", call);
	if (mbp_ui_toggle_popup("Callsign", 0, "No", "Yes", message) == 1) {
		mbp_state_callsign_set(call);
		mbp_state_save();

		sprintf(message, "Callsign changed to '%s'.", call);
		mbp_ui_popup("Callsign", message);
	}
	//Aborted
	else {
		mbp_ui_popup("Callsign", "Callsign not changed.");
	}

}
