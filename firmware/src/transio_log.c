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

#define UI_MARGIN	3
#define SCREEN_WIDTH	20	// characters of small font
#define SCREEN_LINES	9	// rows of small font

static FIL m_logfile;
static bool m_logfile_is_open;
static uint16_t m_first_logfile_record;		// index of record at top of screen
static FSIZE_t m_first_logfile_seek;		// location in the log file

static bool __open_logfile(void) {
	FILINFO info;

	if (m_logfile_is_open) {
		return true;
	}

	if (f_stat(LOG_FILENAME, &info) != FR_OK) {
		return false;
	}

	if (f_open(&m_logfile, LOG_FILENAME, FA_READ | FA_OPEN_EXISTING) != FR_OK) {
		return false;
	}

	m_first_logfile_record = 0;
	m_first_logfile_seek = 0;
	m_logfile_is_open = true;
	return true;
}


static void __close_logfile(void) {
	f_close(&m_logfile);
}


static bool __get_line_from_file(char *buf) {
	char	char_read;
	UINT	count;

	for (uint8_t i=0; i < SCREEN_WIDTH+1; i++) {
		f_read(&m_logfile, &char_read, 1, &count);
		if (count == 1) {
			buf[i] = char_read;
			// detect end of line
			if (char_read == '\n') {
				buf[i+1] = '\0';
				return true;
			}
			// detect and replace non-printable characters
			else if (char_read < 0x20 || char_read > 0x7e) {
				buf[i] = '_';
			}
		} else {
			return false;
		}
	}

	// If we had a line longer than a screen width, terminate it
	// and eat up the rest of the line.
	buf[SCREEN_WIDTH] = '\n';
	buf[SCREEN_WIDTH+1] = 0;	// make certain it's null terminated
	while ((f_read(&m_logfile, &char_read, 1, &count) == FR_OK) &&
			(char_read != '\n')) {
		// do nothing with it.
	}

	return true;
}


void file_viewer(char *filename, char *title, char *eof_message, void empty_message(void)) {
    //buffer for formatting text
    char buf[SCREEN_WIDTH+2];	// room for CR and null termination

	m_logfile_is_open = false;

    bool redraw = true;
	bool at_eof = false;

    while (1) {
		if (redraw || !util_gfx_is_valid_state()) {

		    //Make sure there's no clipping
		    util_gfx_cursor_area_reset();

		    //Draw blank background
		    mbp_ui_cls();

		    //Print their name
		    util_gfx_set_font(FONT_LARGE);
		    util_gfx_set_color(COLOR_GREEN);
		    util_gfx_set_cursor(0, UI_MARGIN);
		    util_gfx_print(title);

			util_gfx_set_font(FONT_SMALL);
			util_gfx_set_color(COLOR_WHITE);
			util_gfx_set_cursor(0, 20);

			if (! __open_logfile()) {
				empty_message();
				at_eof = true;
			} else {
				// Draw or redraw the screen lines from the log file
				if (f_lseek(&m_logfile, m_first_logfile_seek) != FR_OK) {
					mbp_ui_error("seek failed");
					return;
				}

				at_eof = false;
				for (uint8_t row = 0; row < SCREEN_LINES; row++) {
					if (__get_line_from_file(buf)) {
						util_gfx_print(buf);
					} else {
						at_eof = true;
						util_gfx_print(eof_message);
						break;
					}
				}
			}

			redraw = false;
		}

		//validate screen state
		util_gfx_validate();

		util_button_wait();

		if (util_button_left()) {
			util_button_clear();

			__close_logfile();
			return;
		} else if (util_button_up() && m_first_logfile_record >= SCREEN_LINES) {
			// scroll up if possible
			m_first_logfile_record -= SCREEN_LINES;		// back up a screenful
			f_lseek(&m_logfile, 0);		// start at top of file again
			// discard lines until we're where we want to be
			for (uint16_t i = 0; i < m_first_logfile_record; i++) {
				if (!__get_line_from_file(buf)) {
					mbp_ui_error("re-read fail");
					return;
				}
			}
			m_first_logfile_seek = f_tell(&m_logfile);	// mark our new place
			redraw = true;
		} else if (util_button_down() && !at_eof) {
			// scroll down if possible
			m_first_logfile_seek = f_tell(&m_logfile);	// Move forward
			m_first_logfile_record += SCREEN_LINES;		// keep track of line number
			redraw = true;
		}

		util_button_clear();
    }
}


static void __explain_empty_logfile(void) {
	util_gfx_print("--- log empty ---\n\n");
	util_gfx_print("Select neighbors on\nthe Nearby list to\nmake QSOs!\n");
}


void transio_log_screen(void) {
	char call[SETTING_CALLSIGN_LENGTH];

	if (! mbp_state_callsign_get(call)) {
		if (0 == mbp_ui_toggle_popup("CALLSIGN", 0, "Enter", "Cancel", "First you must enter your callsign.")) {
			transio_callsign_edit();
			if (! mbp_state_callsign_get(call)) {
				return;
			}
		} else {
			return;
		}
	}

	file_viewer(LOG_FILENAME, "Your QSOs", "--- end of log ---", __explain_empty_logfile);
}


// Add a record to the logfile, creating it if needed.
void logfile_add_record(char *record) {
	FIL file;
	FRESULT result;
	UINT count;
	UINT record_length = strlen(record);

	//Write the data to SD
	result = f_open(&file, LOG_FILENAME, FA_OPEN_APPEND | FA_WRITE);
	if (result != FR_OK) {
		mbp_ui_error("Could not open logfile for writing.");
		return;
	}

	result = f_write(&file, record, record_length, &count);
	if (result != FR_OK || count != record_length) {
		mbp_ui_error("Could not write to logfile.");
	}

	result = f_close(&file);
	if (result != FR_OK) {
		mbp_ui_error("Could not close logfile.");
	}
}
