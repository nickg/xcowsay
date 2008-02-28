#ifndef INC_DISPLAY_COW_H
#define INC_DISPLAY_COW_H

#include <stdbool.h>

#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>

#define CALCULATE_DISPLAY_TIME -1   // Work out display time from word count

#define debug_msg(...) if (debug) printf(__VA_ARGS__);
#define debug_err(...) if (debug) g_printerr(__VA_ARGS__);

// Show a cow with the given string and clean up afterwards
void display_cow(bool debug, const char *text);
void display_cow_or_invoke_daemon(bool debug, const char *text);
void cowsay_init(int *argc, char ***argv);

#endif
