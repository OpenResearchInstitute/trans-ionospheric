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
