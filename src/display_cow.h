/*  display_cow.h -- Display a cow in a popup window.
 *  Copyright (C) 2008  Nick Gasson
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INC_DISPLAY_COW_H
#define INC_DISPLAY_COW_H

#include <stdbool.h>

#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>

#define CALCULATE_DISPLAY_TIME -1   // Work out display time from word count

#define debug_msg(...) if (debug) printf(__VA_ARGS__);
#define debug_err(...) if (debug) g_printerr(__VA_ARGS__);

typedef enum {
   COWMODE_NORMAL,
   COWMODE_DREAM,
} cowmode_t;

// Show a cow with the given string and clean up afterwards
void display_cow(bool debug, const char *text, bool run_main, cowmode_t mode);
void display_cow_or_invoke_daemon(bool debug, const char *text, cowmode_t mode);
void cowsay_init(int *argc, char ***argv);

#endif
