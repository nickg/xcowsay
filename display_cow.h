#ifndef INC_DISPLAY_COW_H
#define INC_DISPLAY_COW_H

#include <stdbool.h>

#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>

// Show a cow with the given string and clean up afterwards
void display_cow(const char *text);
void display_cow_or_invoke_daemon(bool debug, const char *text);
void cowsay_init(int *argc, char ***argv);

#endif
