/*****************************************************************************
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
 * 	@sconklin
 * 	@mustbeart
 * 	@abraxas3d
 *****************************************************************************/
#ifndef GAMEFILE_H_
#define GAMEFILE_H_

#define	GAMEFILE_FILENAME_LENGTH	12
#define GAMEFILE_QSO_DIRECTORY	"QSO"
#define GAMEFILE_QSO_DIRECTORY_LEN	3

// Make a filename string out of the BLE_ID
extern void gamefile_create_filename(uint8_t *BLE_ID, char *FILENAME);

// Add a record to the gamefile for a particular badge, creating it if needed.
extern void gamefile_add_record(uint8_t *BLE_ID, char *record);

#endif /* GAMEFILE_H_ */
