#ifndef INC_DISPLAY_COW_H
#define INC_DISPLAY_COW_H

#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>

// Show a cow with the given string and clean up afterwards
void display_cow(const char *text);
void cowsay_init(int *argc, char ***argv);

#endif
