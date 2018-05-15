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
static volatile uint16_t m_s_conn_handle = BLE_CONN_HANDLE_INVALID;

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

	//Badge score characteristic
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
