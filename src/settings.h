/*  settings.h -- Manage user options.
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

#ifndef INC_SETTINGS_H
#define INC_SETTINGS_H

#include <stdlib.h>
#include <stdbool.h>

void add_int_option(const char *name, int ival);
void add_bool_option(const char *name, bool bval);
void add_string_option(const char *name, const char *sval);

int get_int_option(const char *name);
bool get_bool_option(const char *name);
const char *get_string_option(const char *name);

void set_int_option(const char *name, int ival);
void set_bool_option(const char *name, bool bval);
void set_string_option(const char *name, const char *sval);

#endif
