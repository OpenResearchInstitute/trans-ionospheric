 /*****************************************************************************
 * (C) Copyright 2017 AND!XOR LLC (http://andnxor.com/).
 * (C) Copyright 2018 Open Research Institute (https://openresearch.institute)
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
 * 	@andnxor
 * 	@zappbrandnxor
 * 	@hyr0n1
 * 	@andrewnriley
 * 	@lacosteaef
 * 	@bitstr3m
 *
 * Further modifications made by
 *      @sconklin
 *      @mustbeart
 *
 *****************************************************************************/
#ifndef MBP_MENU_H_
#define MBP_MENU_H_

#include "system.h"

#define MENU_OK		0
#define MENU_QUIT	1

#define MENU_DRAW_EVERYTHING	0
#define MENU_DRAW_UPDATES		1

typedef void (menu_callback_t)(void *data);
typedef void (menu_global_callback_t)(uint8_t itemno);
typedef void (menu_drawback_t)(uint8_t itemno, uint16_t x, uint16_t y, uint8_t menu_draw_method);
typedef int (menu_resort_t)(uint8_t filter);

typedef struct {
	char *text;
	char *icon;
	char *preview;
	menu_callback_t *callback;
	void *data;
} menu_item_t;

typedef struct {
	char *title;
	uint8_t count;
	uint8_t top;
	uint8_t selected;
	menu_item_t *items;			// set to NULL to use custom draw function
	menu_global_callback_t *callback;	// action callback for all items, only if items is NULL
	menu_drawback_t *draw_item;	// custom draw function, only if items is NULL
	menu_resort_t *resorter;	// called on scroll past top, only if items is NULL
	uint8_t resort_filter;		// argument to resorter
} menu_t;

extern uint8_t mbp_menu(menu_t *p_menu);
extern void mbp_menu_main();
extern void mbp_sort_menu(menu_t *p_menu);

/**
 * Displays sub menu and allows the user to select an item. If the callback is not null
 * the function pointer is called with data. if the callback is null, this returns.
 * This function will change the menu_t struct as the user interacts.
 */
extern uint8_t mbp_submenu(menu_t *p_menu);

#endif /* MBP_MENU_H_ */
