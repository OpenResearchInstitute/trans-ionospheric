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
 *  @sconklin
 *  @mustbeart
 *  @abraxas3d
 *****************************************************************************/

#include "system.h"

#define MM_NUM_COLUMNS	4
#define	MM_NUM_COLORS	6
#define MM_NO_COLOR		(MM_NUM_COLORS+1)
#define MM_ANSWER_MATCH	(MM_NUM_COLORS+2)
#define MM_ANSWER_WHITE	(MM_NUM_COLORS+3)	// right color, wrong column
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

#define MM_BACKGROUND_COLOR		0x2081	// very very yellow
#define MM_DIMMED_COLOR			0x4208	// very dark grey
#define MM_ENABLED_COLOR		COLOR_LIGHTGREY


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

// Identity of our worthy opponent.
//!!! right now this is text; soon it will need to contain either a
//!!! Bluetooth address or a flag that play is to proceed locally.
static ble_badge_list_menu_text_t m_opponent;

// These are the graphical colors used to display the various color indexes.
const uint16_t mm_colors[] = {
	COLOR_YELLOW, COLOR_RED, COLOR_ORANGE, COLOR_BLUE, COLOR_GREEN, COLOR_WHITE,
	MM_BACKGROUND_COLOR,		// MM_NO_COLOR
	COLOR_PINK, COLOR_WHITE		// Colors for answers
};

// The row height on the graphical display varies depending on how many
// rows need to be displayed.
static uint8_t m_row_height;

// Update a single row of the graphical display with color boxes
// for all of the guesses. This is used when redrawing the screen
// after a change in m_row_height, but not when actually entering guesses.
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
static void __draw_peg_outline(uint8_t y, uint8_t peg) {
	uint8_t x = MM_LEFT_MARGIN + peg * (MM_PEG_WIDTH + MM_PEG_KERNING);

	util_gfx_fill_rect(x, y, MM_PEG_WIDTH, m_row_height - MM_LEADING, COLOR_BLACK);
	util_gfx_draw_rect(x, y, MM_PEG_WIDTH, m_row_height - MM_LEADING, COLOR_DARKGREY);
}

// Current coordinates for the "Go" menu item. This item is updated on
// the fly (to show whether all guesses have been entered, making it valid
// to proceed) so it's useful to save these.
static uint8_t m_go_x, m_go_y;

// Set the current coordinates for the "Go" menu item.
static void __set_go_xy(uint8_t x, uint8_t y) {
	m_go_x = x;
	m_go_y = y;
}

// Display the "Go" menu item in the disabled state.
static void __disable_go(void) {
	util_gfx_set_cursor(m_go_x, m_go_y);
	util_gfx_set_color(MM_DIMMED_COLOR);
	util_gfx_print("Go ");
}

// Display the "Go" menu item in the enabled state.
static void __enable_go(void) {
	util_gfx_set_cursor(m_go_x, m_go_y);
	util_gfx_set_color(MM_ENABLED_COLOR);
	util_gfx_print("Go ");
}

// Collect guesses from the local user, in place on the graphic display
// Returns: true if the user guessed and hit Go
//			false if the user hit Quit
static bool __collect_guesses(uint8_t row) {
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

	//!!! Here's where we display the triangle cursor and handle buttons
	//!!! until the user selects Go or Quit

	nrf_delay_ms(5000); //!!! demo
	__enable_go();		//!!! this happens when all four guesses are filled in
	nrf_delay_ms(5000); //!!!

	// Erase other menu choices
	util_gfx_fill_rect(MM_GO_X_POSITION,
						y-1, 							// allow for ascender height
						GFX_WIDTH - MM_GO_X_POSITION,	// to right screen edge
						m_row_height+1,					// to the very bottom
						MM_BACKGROUND_COLOR);

	return true;	//!!! actually need to return correct status here.
}


// Send the guess to the code maker (remote or automatic) and display answers
// Returns: true if we successfully retrieved answers
//			false if not
static bool __collect_answers(uint8_t row) {

	//!!! TODO

	return true;
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

	// Initial screen draw.
	util_gfx_fill_screen(MM_BACKGROUND_COLOR);

	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_cursor(0, 5);
	util_gfx_set_color(COLOR_GREEN);
	util_gfx_print("   GUESS    ANSWER");

	util_gfx_draw_line(0, SUBMENU_TITLE_SIZE, GFX_WIDTH, SUBMENU_TITLE_SIZE, SUBMENU_TITLE_FG);

	while (1) {
		uint8_t row;

		if (turn == 6) {	// time to squeeze the display vertically
			m_row_height = 9;
			util_gfx_fill_rect(0,						// left screen edge
							   SUBMENU_TITLE_SIZE+1,	// just below the line
							   GFX_WIDTH, GFX_HEIGHT,	// to bottom right corner
							   MM_BACKGROUND_COLOR);
			for (row = 0; row < 6; row++) {	// redraw the old rows at new height
				__draw_guesses(row);
				__draw_answers(row);
			}
		}

		if ( ! __collect_guesses(turn)) {
			status = MM_USER_QUIT;
			break;
		}

		if ( ! __collect_answers(turn)) {
			status = MM_REMOTE_QUIT;
			break;
		}

		__draw_answers(row);

		if (__check_for_victory(row)) {
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
				sprintf(msg_buf, "You got it in %d turn%c!", turn-1, turn == 0 ? '!' : 's');
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


// Handle the action of selecting an opponent from the menu.
static void __set_opponent(void *p_data) {
	memcpy(m_opponent.text, ((ble_badge_list_menu_text_t *)p_data)->text, 20);
}


// Present a menu of available opponents to choose from.
//!!! This currently uses the text-only listing from JoCo's badge list.
//!!! It needs to be updated to use a complete list with Bluetooth addresses.
// Returns: true if the user picked an opponent.
//			false if the user canceled.
static bool __mbp_mastermind_players() {
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
		return false;
	}

	badge_list_size = get_nearby_badge_list(NEARBY_BADGE_LIST_LEN, list);

	if (badge_list_size == 0) {
		mbp_ui_popup("BUMMER!", "No other badges are nearby to play with you");
		return false;
	}

	for (uint8_t i = 0; i < badge_list_size; i++) {
		items[menu.count++] = (menu_item_t ) { list[i].text, NULL, NULL, __set_opponent, &list[i].text };
	}

	status = mbp_submenu(&menu);

	free(list);

	if (status == MENU_QUIT) {
		return false;
	} else {
		return true;
	}
}


// Make a connection to the remote opponent.
static bool __mm_connect_opponent(char *text) {
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
	return true;
}


// Entry point from the Games menu.
// The badge that enters from the Games menu establishes the connection,
// if playing a remote opponent, but both users will play as codebreaker.
// The codemaker function is automatic, except for choosing the code.
void mastermind() {

	//!!! here we need to collect the codemaker pegs from the user

	// Try to pick an opponent from the available players
	if (__mbp_mastermind_players()) {
		char buf[40];

		sprintf(buf, "Invite %s to play?", m_opponent.text);
		uint8_t invite = mbp_ui_toggle_popup("PLAYER", 0, "Invite", "Cancel", buf);

		if (invite != 0) {	// 0 means the left choice, i.e., "Invite"
			return;
		}

		if (__mm_connect_opponent(m_opponent.text)) {
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

	// TODO
}
