#ifndef INC_SETTINGS_H
#define INC_SETTINGS_H

#include <stdlib.h>
#include <stdbool.h>

void settings_init();

void add_int_option(const char *name, int ival);
void add_bool_option(const char *name, bool bval);
void add_string_option(const char *name, const char *sval);

int get_int_option(const char *name);
bool get_bool_option(const char *name);
const char *get_string_option(const char *name);


#endif
