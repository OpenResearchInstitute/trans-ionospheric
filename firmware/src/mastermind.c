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

#define MM_NUM_COLUMNS	4
#define	MM_NUM_COLORS	6
#define MM_NO_COLOR		(MM_NUM_COLORS)
#define MM_ANSWER_MATCH	(MM_NUM_COLORS+1)
#define MM_ANSWER_WHITE	(MM_NUM_COLORS+2)	// right color, wrong column
#define MM_MAX_TURNS	12

#define MM_PLAYING		0
#define MM_USER_QUIT	1
#define MM_REMOTE_QUIT	2
#define MM_VICTORY		3
#define MM_DEFEAT		4

#define SUBMENU_TITLE_BG		COLOR_DARKCYAN
#define SUBMENU_TITLE_FG		COLOR_WHITE
#define SUBMENU_TITLE_SIZE		15

#define MM_PEG_KERNING			1
#define MM_LEADING				2
#define MM_LEFT_MARGIN			7
#define MM_GUTTER				10
#define MM_PEG_WIDTH	((GFX_WIDTH - 2*MM_LEFT_MARGIN - MM_GUTTER)/(2*MM_NUM_COLUMNS)-MM_PEG_KERNING)
#define MM_GO_X_POSITION		60
#define MM_FONT_HEIGHT_TO_BASELINE	9
#define MM_ARROW_HEIGHT			8
#define MM_ARROW_HALF_WIDTH		4

#define MM_BACKGROUND_COLOR		0x2081	// very very yellow
#define MM_DIMMED_COLOR			0x4208	// very dark grey
#define MM_ENABLED_COLOR		COLOR_LIGHTGREY

#define MM_REMOTE			17
#define MM_AUTO_OPPONENT	18

#define MM_ANIMATION_MS		100
APP_TIMER_DEF(m_mm_animation_timer);

// Structure that holds a single row of color choices,
// which may be guesses or answers.
// Each entry should be a color index from 0 to MM_NUM_COLORS for guesses,
// or MM_ANSWER_MATCH or MM_ANSWER_WHITE for answers,
// or MM_NO_COLOR if the value is invalid or unknown.
typedef struct {
	uint8_t	peg[MM_NUM_COLUMNS];
} mm_color_list_t;

// Structure that holds the information from a single game turn.
typedef struct {
	bool	played;
	bool	answered;
	mm_color_list_t		guess;
	mm_color_list_t		answer;
} mm_game_turn_t;

// Structure that holds the information for an entire game (one direction)
typedef struct {
	bool	won;
	bool	lost;
	mm_game_turn_t turn[MM_MAX_TURNS];
} mm_game_t;

// This is the game in which the local user is Codebreaker.
static mm_game_t game;


// Structure that holds the information about our worthy opponent.
typedef struct {
	bool	remote;
	char	name[20];
} mm_opponent_t;

// Identity of our worthy opponent.
//!!! right now the badge identity is text; soon it will need to contain either a
//!!! Bluetooth address or a flag that play is to proceed locally.
static mm_opponent_t m_opponent;

// Secret code for local play against the badge
static mm_color_list_t		m_their_code;

// Secret code we create for remote player to guess
static mm_color_list_t		m_our_code;

// Key pegs answer we have received from the codemaker (auto or remote)
static mm_color_list_t		m_received_answer;

// These are the graphical colors used to display the various color indexes.
const uint16_t mm_colors[] = {
	COLOR_YELLOW, COLOR_RED, COLOR_MAGENTA, COLOR_BLUE, COLOR_GREEN, COLOR_WHITE,
	MM_BACKGROUND_COLOR,		// MM_NO_COLOR
	COLOR_PINK, COLOR_WHITE		// Colors for answers
};

// The row height on the graphical display varies depending on how many
// rows need to be displayed.
static uint8_t m_row_height;


// Current coordinates for the "Go" menu item. This item is updated on
// the fly (to show whether all guesses have been entered, making it valid
// to proceed) so it's useful to save these.
static uint8_t m_go_x, m_go_y;

// Set the current coordinates for the "Go" menu item.
static void __set_go_xy(uint8_t x, uint8_t y) {
	m_go_x = x;
	m_go_y = y;
}


// Display the "Go" menu item in a specified color.
static void __display_go(uint16_t color) {
	util_gfx_set_cursor(m_go_x, m_go_y);
	util_gfx_set_color(color);
	util_gfx_print("Go ");
}


// Display the "Go" menu item in the disabled state.
static void __disable_go(void) {
	__display_go(MM_DIMMED_COLOR);
}


// Display the "Go" menu item in the enabled state.
static void __enable_go(void) {
	__display_go(MM_ENABLED_COLOR);
}


// Update a single row of the graphical display with color boxes
// for all of the guesses. This is used when redrawing the screen
// after a change in m_row_height, but not when actually entering guesses.
// It assumes the guesses are all valid colors.
static void __draw_guesses(uint8_t row) {
	uint8_t y = SUBMENU_TITLE_SIZE + 1 + row * m_row_height + 2;
	uint8_t x = MM_LEFT_MARGIN;

	for (uint8_t peg = 0; peg < MM_NUM_COLUMNS; peg++) {
		uint16_t color = mm_colors[game.turn[row].guess.peg[peg]];
		util_gfx_fill_rect(x, y, MM_PEG_WIDTH, m_row_height - MM_LEADING, color);
		x += MM_PEG_WIDTH + MM_PEG_KERNING;
	}
}


// Update a single row of the graphical display with color boxes
// for all of the answers. This is used both when the answers first
// arrive and when redrawing the screen after a change in m_row_height.
static void __draw_answers(uint8_t row) {
	uint8_t y = SUBMENU_TITLE_SIZE + 1 + row * m_row_height + 2;
	uint8_t x = 69;

	for (uint8_t peg = 0; peg < MM_NUM_COLUMNS; peg++) {
		uint16_t color = mm_colors[game.turn[row].answer.peg[peg]];
		util_gfx_fill_rect(x, y, MM_PEG_WIDTH, m_row_height - MM_LEADING, color);
		x += MM_PEG_WIDTH + MM_PEG_KERNING;
	}
}


// Draw a single blank color box, consisting of an outline in dark grey
// around a box filled with black. Used to initialize the guesses, and
// also used when the user chooses to enter MM_NO_COLOR temporarily.
static void __draw_peg_outline(uint8_t y, uint8_t column) {
	uint8_t x = MM_LEFT_MARGIN + column * (MM_PEG_WIDTH + MM_PEG_KERNING);

	util_gfx_fill_rect(x, y, MM_PEG_WIDTH, m_row_height - MM_LEADING, COLOR_BLACK);
	util_gfx_draw_rect(x, y, MM_PEG_WIDTH, m_row_height - MM_LEADING, COLOR_DARKGREY);
}


// Draw a single color box in the specified color.
static void __draw_peg_colorbox(uint8_t y, uint8_t column, uint8_t color) {
	uint8_t x = MM_LEFT_MARGIN + column * (MM_PEG_WIDTH + MM_PEG_KERNING);

	util_gfx_fill_rect(x, y, MM_PEG_WIDTH, m_row_height - MM_LEADING, mm_colors[color]);
}


// Check whether all the columns in this guess are set to a valid color.
static bool __guess_is_complete(mm_color_list_t *guess, uint8_t row) {
	for (uint8_t i=0; i < MM_NUM_COLUMNS; i++) {
		if (guess->peg[i] >= MM_NUM_COLORS) {
			return false;
		}
	}

	return true;
}


// Update a single peg box according to game state and button press.
// Cycle up or down through the list of colors (0 to MM_NUM_COLORS-1) and
// a final value representing no color choice (MM_NUM_COLORS == MM_NO_COLOR).
static void __change_peg_box(mm_color_list_t *guess, uint8_t y, uint8_t row, uint8_t column, bool up) {
	uint8_t peg = guess->peg[column];
	if (up) {
		peg++;
		if (peg > MM_NUM_COLORS) {
			peg = 0;
		}
	} else if (peg == 0) {
		peg = MM_NO_COLOR;
	} else {
		peg--;
	}
	guess->peg[column] = peg;

	if (peg == MM_NO_COLOR) {
		__draw_peg_outline(y, column);
	} else {
		__draw_peg_colorbox(y, column, peg);
	}

	if (__guess_is_complete(guess, row)) {
		__enable_go();
	} else {
		__disable_go();
	}
}


// Clear the arrow pointer.
static void __mm_arrow_clear(uint8_t y) {
	util_gfx_fill_rect( 0, y+m_row_height, GFX_WIDTH, MM_ARROW_HEIGHT+1, MM_BACKGROUND_COLOR);
}


// Display the arrow pointer.
static void __mm_arrow(uint8_t y, uint8_t which) {
	// Erase any previous arrows
	__mm_arrow_clear(y);

	// compute the location of the top vertex
	y += m_row_height;
	uint8_t x = MM_LEFT_MARGIN + MM_PEG_WIDTH/2 + which*(MM_PEG_WIDTH+MM_PEG_KERNING);
	if (which == MM_NUM_COLUMNS) {
		// Move the Go arrow over a bit to look centered
		x += 1;
	} else if (which == MM_NUM_COLUMNS+1) {
		// Move the Quit arrow over a bit to look centered
		x += 12;
	}

	// draw the arrow
	util_gfx_draw_triangle(	x, y,	// Top vertex
							x - MM_ARROW_HALF_WIDTH, y + MM_ARROW_HEIGHT,
							x + MM_ARROW_HALF_WIDTH, y + MM_ARROW_HEIGHT,
							COLOR_WHITE);
}


// Collect guesses from the local user, in place on the graphic display
// Returns: true if the user guessed and hit Go
//			false if the user hit Quit
static bool __collect_guesses(mm_color_list_t *guess, uint8_t row) {
	uint8_t y = SUBMENU_TITLE_SIZE		// room for the column headers
	 			+ 1 					// room for the line under the headers
				+ row * m_row_height	// room for the boxes above this row
				+ MM_LEADING;			// room for a bit of space between rows

	// Draw blank boxes where the guesses go
	for (uint8_t peg = 0; peg < MM_NUM_COLUMNS; peg++) {
		__draw_peg_outline(y, peg);
	}

	// Draw other menu choices
	__set_go_xy(MM_GO_X_POSITION,
				y + m_row_height - MM_FONT_HEIGHT_TO_BASELINE); // bottom-justify text
	util_gfx_set_cursor(m_go_x, m_go_y);
	util_gfx_set_color(MM_ENABLED_COLOR);	// Quit is always an option
	util_gfx_print("   Quit");	// leaving room for "Go" at the beginning
	__disable_go();

	// Handle buttons until the user selects Go or Quit
	uint8_t column = 0;
	__mm_arrow(y, column);
	while (1) {
		util_button_wait();

		if (util_button_up()) {
			if (column < MM_NUM_COLUMNS) {
				__change_peg_box(guess, y, row, column, true);
			}
		} else if (util_button_down() > 0) {
			if (column < MM_NUM_COLUMNS) {
				__change_peg_box(guess, y, row, column, false);
			}
		} else if (util_button_action()) {
			if (column == MM_NUM_COLUMNS) {	// Go menu item
				if (__guess_is_complete(guess, row)) {
					// Erase other menu choices
					util_gfx_fill_rect(MM_GO_X_POSITION,
										y-2, 							// allow for ascender height
										GFX_WIDTH - MM_GO_X_POSITION,	// to right screen edge
										m_row_height+2,					// to the very bottom
										MM_BACKGROUND_COLOR);
					__mm_arrow_clear(y);
					return true;
				}
			} else if (column == MM_NUM_COLUMNS+1) {	// Quit menu item
				// No need to clean up display, we'll be abandoning it anyway.
				return false;
			}
		} else if (util_button_left()) {
			if (column > 0) {
				column--;
				__mm_arrow(y, column);
			}
		} else if (util_button_right()) {
			if (column < MM_NUM_COLUMNS+1) {
				column++;
				__mm_arrow(y, column);
			}
		}

		util_button_clear();
	}
}


// shuffle the array of key pegs
static void __shuffle(uint8_t* array, long NumberOfElements)
{
	if (NumberOfElements > 1)
	{
		long i;
		for (i = 0; i < NumberOfElements - 1; i++)
		{
			long j = i + rand() / (RAND_MAX / (NumberOfElements - i) + 1);
			uint8_t t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}


// Score a guess against a secret code.
// Results go in m_received_answer
// This implementation only works for FOUR COLUMNS
static void __mm_score_guess(mm_color_list_t *guess_struct, mm_color_list_t *code) {

	// Adapt arguments to existing code for scoring
	// int testCode(int* color, int* guess, int* pegs)
	uint8_t guess[4];
	uint8_t color[4];
	uint8_t pegs[4];
	for (uint8_t i=0; i < 4; i++) {
		guess[i] = guess_struct->peg[i] + 1;		// color can't be 0 here.
		color[i] = code->peg[i] + 1;
	}

	//key peg result of 2 is a match in both position and color
	//key peg result of 1 is a match in color, but not position
	//punch out all the perfect matches first
	//Check for exact matches and mark corresponding key peg
	if(guess[0] == color[0]){
		// printf("correct1\n");
		pegs[0] = 2;
	}
	if(guess[1] == color[1]){
		// printf("correct2\n");
		pegs[1] = 2;
	}
	if(guess[2] == color[2]){
		// printf("correct3\n");
		pegs[2] = 2;
	}
	if(guess[3] == color[3]){
		// printf("correct4\n");
		pegs[3] = 2;
	}

	//then check remaining guesses to see if the color appears in remaining color code
	//found another bug.

	uint8_t color_flag[4] = {1, 1, 1, 1};  //trust me

	if(pegs[0]==0){
		// printf("test guess[0] for color out of place\n");
		//we only work with guess[0] if it wasn't an exact match
		//it's either 0 or 2 at this point of the tests
		//this might work more generally if it was != 2
		//but I don't know that yet
		if((guess[0] == color[1])&&(pegs[1]!=2)){
			//guess[0] matches color[1], but we can't claim it if guess[1] matched color[1] already
			//since pegs[1] = 2 in that condition, we can work this into the test
			// printf("guess[0] at color[1] so peg[0] = 1 and color_flag[1] = 0 to remove it from further testing\n");
			pegs[0] = 1;
			color_flag[1] = 0; //we matched with color[1], remove it
		}else{//only execute this if the above condition was false
			if((guess[0] == color[2])&&(pegs[2]!=2)){
				//guess[0] matches color[2], but we can't claim it if guess[2] matched color[2] already
				//since pegs[2] = 2 in that condition, we can work this into the test
				// printf("guess[0] at color[2] so pegs[0]=1 and color_flag[2] = 0 to remove it from further testing\n");
				pegs[0] = 1;
				color_flag[2] = 0; //we matched with color[2], remove it
			}else{//only execute this if the above condition was false
				if((guess[0] == color[3])&&(pegs[3]!=2)){
					//guess[0] matches color[3], but we can't claim it if guess[3] matched color[3] already
					//since pegs[3] = 2 in that condition, we can work this into the test
					// printf("guess[0] at color[3] so pegs[0]=1 and color_flag[3] = 0 to remove it from further testing\n");
					pegs[0] = 1;
					color_flag[3] = 0; //we matched with color[3], remove it


				}
			}
		}
	}


	if(pegs[1]==0){
		// printf("test guess[1] for color out of place\n");
		//we only work with guess[1] if it wasn't an exact match
		//it's either 0 or 2 at this point of the tests
		//this might work more generally if it was != 2
		//but I don't know that yet
		if((guess[1] == color_flag[0]*color[0])&&(pegs[0]!=2)){
			//guess[1] matches color[0], but we can't claim it if guess[0] matched color[0] already
			//since pegs[0] = 2 in that condition, we can work this into the test
			// printf("guess[1] at color[0] so peg[1] = 1 and color_flag[0] = 0 to remove it from further testing\n");
			pegs[1] = 1;
			color_flag[0] = 0; //we matched with color[0], remove it


		}else{//only execute this if the above condition was false
			if((guess[1] == color_flag[2]*color[2])&&(pegs[2]!=2)){
				//guess[1] matches color[2], but we can't claim it if guess[2] matched color[2] already
				//since pegs[2] = 2 in that condition, we can work this into the test
				// printf("guess[1] at color[2] so pegs[1]=1 and color_flag[2] = 0 to remove it from further testing\n");
				pegs[1] = 1;
				color_flag[2] = 0; //we matched with color[2], remove it


			}else{//only execute this if the above condition was false
				if((guess[1] == color_flag[3]*color[3])&&(pegs[3]!=2)){
					//guess[1] matches color[3], but we can't claim it if guess[3] matched color[3] already
					//since pegs[3] = 2 in that condition, we can work this into the test
					// printf("guess[1] at color[3] so pegs[1]=1 and color_flag[3] = 0 to remove it from further testing\n");
					pegs[1] = 1;
					color_flag[3] = 0; //we matched with color[3], remove it


				}
			}
		}
	}

	if(pegs[2]==0){
		// printf("test guess[2] for color out of place\n");
		//we only work with guess[2] if it wasn't an exact match
		//it's either 0 or 2 at this point of the tests
		//this might work more generally if it was != 2
		//but I don't know that yet
		if((guess[2] == color_flag[0]*color[0])&&(pegs[0]!=2)){
			//guess[2] matches color[0], but we can't claim it if guess[0] matched color[0] already
			//since pegs[0] = 2 in that condition, we can work this into the test
			// printf("guess[2] at color[0] so peg[2] = 1 and color_flag[0] = 0 to remove it from further testing\n");
			pegs[2] = 1;
			color_flag[0] = 0; //we matched with color[0], remove it


		}else{//only execute this if the above condition was false
			if((guess[2] == color_flag[1]*color[1])&&(pegs[1]!=2)){
				//guess[2] matches color[1], but we can't claim it if guess[1] matched color[1] already
				//since pegs[1] = 2 in that condition, we can work this into the test
				// printf("guess[2] at color[1] so pegs[2]=1 and color_flag[1] = 0 to remove it from further testing\n");
				pegs[2] = 1;
				color_flag[1] = 0; //we matched with color[1], remove it


			}else{//only execute this if the above condition was false
				if((guess[2] == color_flag[3]*color[3])&&(pegs[3]!=2)){
					//guess[2] matches color[3], but we can't claim it if guess[3] matched color[3] already
					//since pegs[3] = 2 in that condition, we can work this into the test
					// printf("guess[2] at color[3] so pegs[2]=1 and color_flag[3] = 0 to remove it from further testing\n");
					pegs[2] = 1;
					color_flag[3] = 0; //we matched with color[3], remove it


				}
			}
		}
	}

	if(pegs[3]==0){
		// printf("test guess[3] for color out of place\n");
		//we only work with guess[3] if it wasn't an exact match
		//it's either 0 or 2 at this point of the tests
		//this might work more generally if it was != 2
		//but I don't know that yet
		if((guess[3] == color_flag[0]*color[0])&&(pegs[0]!=2)){
			//guess[3] matches color[0], but we can't claim it if guess[0] matched color[0] already
			//since pegs[0] = 2 in that condition, we can work this into the test
			// printf("guess[3] at color[0] so peg[3] = 1 and color_flag[0] = 0 to remove it from further testing\n");
			pegs[3] = 1;
			color_flag[0] = 0; //we matched with color[0], remove it


		}else{//only execute this if the above condition was false
			if((guess[3] == color_flag[2]*color[2])&&(pegs[2]!=2)){
				//guess[3] matches color[2], but we can't claim it if guess[2] matched color[2] already
				//since pegs[2] = 2 in that condition, we can work this into the test
				// printf("guess[3] at color[2] so pegs[3]=1 and color_flag[2] = 0 to remove it from further testing\n");
				pegs[3] = 1;
				color_flag[2] = 0; //we matched with color[2], remove it


			}else{//only execute this if the above condition was false
				if((guess[3] == color_flag[1]*color[1])&&(pegs[1]!=2)){
					//guess[3] matches color[1], but we can't claim it if guess[1] matched color[1] already
					//since pegs[1] = 2 in that condition, we can work this into the test
					// printf("guess[3] at color[1] so pegs[3]=1 and color_flag[1] = 0 to remove it from further testing\n");
					pegs[3] = 1;
					color_flag[1] = 0; //we matched with color[1], remove it


				}
			}
		}
	}
	// printf("inside knuth-o-matic, counter is now %d\n", pegs[0]+pegs[1]+pegs[2]+pegs[3]);
	// return pegs[0]+pegs[1]+pegs[2]+pegs[3];

	// Key pegs need to be scrambled up to hide position information
	__shuffle(pegs, 4);

	// Convert output of existing code to our color convention
	// and copy it into m_received_answer.
	for (uint8_t i=0; i < MM_NUM_COLUMNS; i++) {
		switch (pegs[i]) {
			case 0:
				m_received_answer.peg[i] = MM_NO_COLOR;
				break;
			case 1:
				m_received_answer.peg[i] = MM_ANSWER_WHITE;
				break;
			case 2:
				m_received_answer.peg[i] = MM_ANSWER_MATCH;
				break;
			default:
				mbp_ui_error("unknown key peg value");
				break;
		}
	}
}


// Alternate untested unintegrated scoring routine.
// WIP but it can score any number of columns.
// static void __score_guess(uint8_t *guess, uint8_t *code, uint8_t *keypegs) {
// 	uint8_t exact_matches = 0;
// 	uint8_t color_matches = 0;
//
// 	// Make scratch copies of both guess and code
// 	for (int8_t i=0; i < MM_NUM_COLUMNS; i++) {
// 		s_guess[i] = guess[i];
// 		s_code[i] = code[i];
// 	}
//
// 	// Check for exact matches (color and position); mark them in both scratch arrays
// 	for (int8_t i=0; i < MM_NUM_COLUMNS; i++) {
// 		if (s_guess[i] == s_code[i]) {
// 			exact_matches++;
// 			s_guess[i] = MM_NO_COLOR_1;
// 			s_code[i] == MM_NO_COLOR_2;
// 		}
// 	}
//
// 	// For each peg in the guess, scan the code array for a color match.
// 	for (uint8_t g=0; g < MM_NUM_COLUMNS; g++) {
// 		// We're not skipping guess that are exact matches; they won't match again.
//
// 		for (uint8_t c=0; c < MM_NUM_COLUMNS; c++) {
// 			// We're not skipping codes that already matched; they won't match again.
//
// 			if (s_guess[g] == s_code[c]) {
// 				color_matches++;
// 				s_code[c] = MM_NO_COLOR_2;
// 				break;		// Don't look for any more matches of this guess peg
// 			}
// 		}
// 	}
//
//
// }


// Animation timer for waiting for our guess to be scored.
static uint8_t m_animation_row;
static void __mm_animation_timer_handler(void *p_data) {

	// select random colors for the answers in the row
	for (uint8_t i=0; i < MM_NUM_COLUMNS; i++) {
		game.turn[m_animation_row].answer.peg[i] = util_math_rand16_max(0xFFFF);
	}
	__draw_answers(m_animation_row);
}


// Control the animation for waiting for our guess to be scored.
static void __animate_answers(uint8_t row, bool run) {
	uint32_t err_code;

	if (run) {
		m_animation_row = row;

		// start the animation
		err_code = app_timer_create(&m_mm_animation_timer, APP_TIMER_MODE_REPEATED, __mm_animation_timer_handler);
		APP_ERROR_CHECK(err_code);

		uint32_t ticks = APP_TIMER_TICKS(MM_ANIMATION_MS, UTIL_TIMER_PRESCALER);
		err_code = app_timer_start(m_mm_animation_timer, ticks, NULL);
		APP_ERROR_CHECK(err_code);

	} else {
		// stop the animation
		app_timer_stop(m_mm_animation_timer);
	}
}


// Send the user's guess to the code maker (remote or automatic) and obtain answers
// Returns: true if we successfully retrieved answers
//			false if not
static bool __collect_answers(uint8_t row) {
	bool status = false;

	// for debug let's see the receive answers all magenta if not updated.
	// for (uint8_t i=0; i < MM_NUM_COLUMNS; i++) {
	// 	m_received_answer.peg[i] = 2;
	// }

	// scribble on the answers wildly to signal that thinking is taking place
	__animate_answers(row, true);

	if (m_opponent.remote) {
		//!!! TODO

		status = false;
	} else {	// non-remote, automatic, opponent
		// call the scoring algorithm locally
		__mm_score_guess(&(game.turn[row].guess), &m_their_code);

		nrf_delay_ms(2000);	// make it seem like hard work!
		status = true;		// local scoring always succeeds
	}

	// stop the animation
	__animate_answers(row, false);

	// replace the animation with answers collected
	for (uint8_t i=0; i < MM_NUM_COLUMNS; i++) {
		game.turn[row].answer.peg[i] = m_received_answer.peg[i];
	}
	__draw_answers(row);

	return status;
}


// Examine the answers and decide if this was a winning guess.
// That is, if all the answers are Matches.
// Returns: true if we got all Matches
//			false if not
static bool __check_for_victory(uint8_t row) {
	bool victory = true;

	for (uint8_t peg=0; peg < MM_NUM_COLUMNS; peg++) {
		victory &= (MM_ANSWER_MATCH == game.turn[row].answer.peg[peg]);
	}

	return victory;
}


// Implements the game of Mastermind for the local code breaker.
static void __mm_codebreaker(void) {
	uint8_t turn = 0;
	uint8_t status = MM_PLAYING;

	m_row_height = 16;	// nice big boxes to start with (height in pixels)

	// Initialize the game.
	game.won = false;
	game.lost = false;
	for (uint8_t row = 0; row < MM_MAX_TURNS; row++) {
		game.turn[row].played = false;
		game.turn[row].answered = false;
		for (uint8_t column = 0; column < MM_NUM_COLUMNS; column++) {
			game.turn[row].guess.peg[column] = MM_NO_COLOR;
			game.turn[row].answer.peg[column] = MM_NO_COLOR;
		}
	}

	// Initial screen draw
	util_gfx_cursor_area_reset();
	mbp_ui_cls();
	util_gfx_fill_screen(MM_BACKGROUND_COLOR);

	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_cursor(0, 5);
	util_gfx_set_color(COLOR_GREEN);
	util_gfx_print("   GUESS    ANSWER");

	util_gfx_draw_line(0, SUBMENU_TITLE_SIZE, GFX_WIDTH, SUBMENU_TITLE_SIZE, SUBMENU_TITLE_FG);

	while (1) {
		uint8_t row;

		if (turn == 6) {	// time to squeeze the display vertically
			m_row_height = 8;
			util_gfx_fill_rect(0,						// left screen edge
							   SUBMENU_TITLE_SIZE+1,	// just below the line
							   GFX_WIDTH, GFX_HEIGHT,	// to bottom right corner
							   MM_BACKGROUND_COLOR);
			for (row = 0; row < 6; row++) {	// redraw the old rows at new height
				__draw_guesses(row);
				__draw_answers(row);
			}
		}

		if ( ! __collect_guesses(&(game.turn[turn].guess), turn)) {
			status = MM_USER_QUIT;
			break;
		}

		if ( ! __collect_answers(turn)) {
			status = MM_REMOTE_QUIT;
			break;
		}

		__draw_answers(turn);

		if (__check_for_victory(turn)) {
			status = MM_VICTORY;
			break;
		}

		// Next turn
		turn++;
		if (turn >= MM_MAX_TURNS) {	// Oops, used up the turn limit!
			status = MM_DEFEAT;
			nrf_delay_ms(2000);		// time to ponder your failure
			break;
		}
	}

	char msg_buf[60];
	switch (status) {
		case MM_USER_QUIT:
				sprintf(msg_buf, "You quit the game!");
				break;
		case MM_REMOTE_QUIT:
				sprintf(msg_buf, "Couldn't get an answer from the other player!");
				break;
		case MM_VICTORY:
				sprintf(msg_buf, "You got it in %d turn%c!", turn+1, turn == 0 ? '!' : 's');
				break;
		case MM_DEFEAT:
				sprintf(msg_buf, "You had %d tries but still didn't get it!", MM_MAX_TURNS);
				break;
		default:
				sprintf(msg_buf, "Why is the game over?");
				break;
	}
	mbp_ui_popup("GAMEOVER", msg_buf);
}


// Prompts the user for a secret code
// Returns: true if the user entered a complete code and hit Go
//			false if the user hit Quit
static bool __mm_make_code(void) {

	m_row_height = 16;	// nice big boxes for single-row code entry

	// Initial screen draw.
	util_gfx_cursor_area_reset();
	mbp_ui_cls();
	util_gfx_fill_screen(MM_BACKGROUND_COLOR);

	util_gfx_set_font(FONT_LARGE);
	util_gfx_set_cursor(0, 5);
	util_gfx_set_color(COLOR_WHITE);
	util_gfx_fill_rect(0, 0, GFX_WIDTH, SUBMENU_TITLE_SIZE, SUBMENU_TITLE_BG);
	util_gfx_draw_line(0, SUBMENU_TITLE_SIZE, GFX_WIDTH, SUBMENU_TITLE_SIZE, SUBMENU_TITLE_FG);
	util_gfx_print("MASTERMIND");

	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_cursor(0, 2 + SUBMENU_TITLE_SIZE);
	util_gfx_set_color(COLOR_GREEN);
	util_gfx_print("Enter your secret    code for the other   player to guess:");

	// util_gfx_draw_line(0, SUBMENU_TITLE_SIZE, GFX_WIDTH, SUBMENU_TITLE_SIZE, SUBMENU_TITLE_FG);

	for (uint8_t i=0; i < MM_NUM_COLUMNS; i++) {
		m_our_code.peg[i] = MM_NO_COLOR;
	}

	return __collect_guesses(&m_our_code, 3);	// Use row 3 for a nice vertical location
}


// Handle the action of selecting an opponent from the menu.
static void __set_remote_opponent(void *p_data) {
	memcpy(m_opponent.name, ((ble_badge_list_menu_text_t *)p_data)->text, 20);
	m_opponent.remote = true;
}


static void __set_auto_opponent(void *p_data) {
	strcpy(m_opponent.name, "the Badge");
	m_opponent.remote = false;
}


// Present a menu of available opponents to choose from.
//!!! This currently uses the text-only listing from JoCo's badge list.
//!!! It needs to be updated to use a complete list with Bluetooth addresses.
// Returns: MM_REMOTE if the user picked a remote badge opponent.
//			MM_AUTO_OPPONENT if the user chose to play against the badge
//			MM_USER_QUIT if the user canceled.
static uint8_t __mbp_mastermind_players() {
	int badge_list_size;
	ble_badge_list_menu_text_t *list;
	uint8_t status;

	menu_t menu;
	menu_item_t items[NEARBY_BADGE_LIST_LEN];
	menu.items = items;
	menu.count = 0;
	menu.title = "Players";

	list = malloc(NEARBY_BADGE_LIST_LEN * sizeof( ble_badge_list_menu_text_t));
	if (!list) {
		mbp_ui_error("players malloc fail.");
		return MM_USER_QUIT;
	}

	badge_list_size = get_nearby_badge_list(NEARBY_BADGE_LIST_LEN, list);

	if (badge_list_size == 0) {
		status = mbp_ui_toggle_popup("ALONE!", 0, "Play", "Quit", "No other badges are nearby. Play against the badge?");
		if (status == 0) {
			return MM_AUTO_OPPONENT;
		} else {
			return MM_USER_QUIT;
		}
	}

	items[menu.count++] = (menu_item_t) { "Play the badge", NULL, NULL, __set_auto_opponent, NULL};
	for (uint8_t i = 0; i < badge_list_size; i++) {
		items[menu.count++] = (menu_item_t ) { list[i].text, NULL, NULL, __set_remote_opponent, &list[i].text };
	}

	status = mbp_submenu(&menu);

	free(list);

	if (status == MENU_QUIT) {
		return MM_USER_QUIT;
	} else if (m_opponent.remote) {
		return MM_REMOTE;
	} else {
		return MM_AUTO_OPPONENT;
	}
}


// Make a connection to the remote opponent.
static bool __mm_connect_remote_opponent(mm_opponent_t opponent) {
	util_gfx_cursor_area_reset();
	mbp_ui_cls();

	//!!! here we will try to make a Bluetooth connection
	//!!! when it's up, we'll start the codemaker server for incoming guesses

	// Fake dialog to keep the user entertained ...
	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_cursor(0, 5);
	util_gfx_set_color(COLOR_WHITE);
	util_gfx_print("+++");
	nrf_delay_ms(800);
	util_gfx_print("\nOK\nATZ\n");
	nrf_delay_ms(800);
	util_gfx_print("OK\nATDT8675309\n");
	nrf_delay_ms(1700);

	//!!! here we will wait for the connection if it's not already up

	util_gfx_print("CONNECT\n");

	//!!! here we will negotiate with the other badge to see if it will play.
	//!!! in the meantime, add a delay for comfort.
	nrf_delay_ms(2000);
	
	return true;
}


// "Connect" to the simulated opponent within this program.
static bool __mm_connect_auto_opponent(mm_opponent_t opponent) {
	// Roll up a random secret code.
	for (uint8_t i=0; i < MM_NUM_COLUMNS; i++) {
		m_their_code.peg[i] = util_math_rand8_max(MM_NUM_COLORS);
	}

	return true;
}


static bool __mm_connect_opponent(mm_opponent_t opponent) {
	if (opponent.remote) {
		return __mm_connect_remote_opponent(opponent);
	} else {
		return __mm_connect_auto_opponent(opponent);
	}
}


// Entry point from the Games menu.
// The badge that enters from the Games menu establishes the connection,
// if playing a remote opponent, but both users will play as codebreaker.
// The codemaker function is automatic, except for choosing the code.
void mastermind() {
	uint8_t player_type;
	char buf[40];

	// Try to collect the codemaker pegs from the user
	if (! __mm_make_code()) {
		return;
	}

	// Try to pick an opponent from the available players
	player_type = __mbp_mastermind_players();

	switch (player_type) {
		case MM_USER_QUIT:
			return;

		case MM_REMOTE:
			sprintf(buf, "Invite %s to play?", m_opponent.name);
			uint8_t invite = mbp_ui_toggle_popup("PLAYER", 0, "Invite", "Cancel", buf);

			if (invite != 0) {	// 0 means the left choice, i.e., "Invite"
				return;
			}
			// FALLTHROUGH

		case MM_AUTO_OPPONENT:

			if (__mm_connect_opponent(m_opponent)) {
				__mm_codebreaker();	// play the game locally.

				//!!! here we will join up with the codemaker server,
				// waiting for it to complete if necessary,
				// and then summarize the game-pair results.

			} else {
				mbp_ui_popup("FAILURE", "Unable to connect with opponent");
			}
	}
}


// Entry point from an incoming Bluetooth connection.
void mastermind_rsvp(void) {

	//!!! TODO
}
