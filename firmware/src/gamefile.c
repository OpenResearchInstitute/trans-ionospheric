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
#include "../system.h"


//Make a filename string out of the BLE_ID
void gamefile_create_filename(uint8_t *BLE_ID, char *filename)
{
	char alphabet[36] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
	int letter[GAMEFILE_FILENAME_LENGTH];

	//create indexes into the alphabet from the BLE_ID
	//BLE_ID is divided into 5 bit chunks
	letter[0] = (BLE_ID[0]&0xF8)>>3;

	letter[1] = (BLE_ID[0]&0x07) + ((BLE_ID[1]&0xC0)>>3);

	letter[2] = (BLE_ID[1]&0x3E)>>1;

	letter[3] = ((BLE_ID[1]&0x01)<<4) + ((BLE_ID[2]&0xF0)>>4);

	letter[4] = ((BLE_ID[2]&0x0F)<<1) + ((BLE_ID[3]&0x80)>>7);

	letter[5] = (BLE_ID[3]&0x7C)>>2;

	letter[6] = ((BLE_ID[3]&0x03)<<3) + ((BLE_ID[4]&0xE0)>>5);

	letter[7] = 0;

	letter[8] = (BLE_ID[4]&0x1F);

	letter[9] = (BLE_ID[5]&0xF8)>>3;\

	letter[10] = (BLE_ID[5]&0x07); //0b0000 0111 becomes 0b00111

	//create a filename array with a dot.
	for (int i = 0; i < 7; i++) {
		filename[i] = alphabet[letter[i]];
	}

	filename[7] = '.';

	for (int i = 8; i < 11; i++) {
		filename[i] = alphabet[letter[i]];
	}

	filename[11] = '\0';
}


// Add a record to the gamefile for a particular badge, creating it if needed.
void gamefile_add_record(uint8_t *BLE_ID, char *record) {
	FIL file;
	FRESULT result;
	UINT count;
	UINT record_length = strlen(record);
	char filename[GAMEFILE_FILENAME_LENGTH+1];
	char path[1+GAMEFILE_QSO_DIRECTORY_LEN+1+GAMEFILE_FILENAME_LENGTH+1];

	sprintf(path, "/%s", GAMEFILE_QSO_DIRECTORY);
	result = f_mkdir(path);
	if (result != FR_OK && result != FR_EXIST) {
		mbp_ui_error("Could not create game folder");
		return;
	}
	
	gamefile_create_filename(BLE_ID, filename);
	sprintf(path, "/%s/%s", GAMEFILE_QSO_DIRECTORY, filename);

	//Write the data to SD
	result = f_open(&file, path, FA_OPEN_APPEND | FA_WRITE);
	if (result != FR_OK) {
		mbp_ui_error("Could not open gamefile for writing.");
		return;
	}

	result = f_write(&file, record, record_length, &count);
	if (result != FR_OK || count != record_length) {
		mbp_ui_error("Could not write to gamefile.");
	}

	result = f_close(&file);
	if (result != FR_OK) {
		mbp_ui_error("Could not close gamefile.");
	}
}
