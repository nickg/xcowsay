/*  display_cow.c -- Display a cow in a popup window.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>

#ifdef WITH_DBUS
#include <dbus/dbus-glib-bindings.h>
#define XCOWSAY_PATH "/uk/me/doof/Cowsay"
#define XCOWSAY_NAMESPACE "uk.me.doof.Cowsay"
#endif

#include "floating_shape.h"
#include "display_cow.h"
#include "settings.h"
#include "i18n.h"

GdkPixbuf *make_text_bubble(char *text, int *p_width, int *p_height, cowmode_t mode);
GdkPixbuf *make_dream_bubble(const char *file, int *p_width, int *p_height);

#define TICK_TIMEOUT   100
#define BUBBLE_XOFFSET 5  // Distance from cow to bubble

#define max(a, b) ((a) > (b) ? (a) : (b))

typedef enum {
   csLeadIn, csDisplay, csLeadOut, csCleanup
} cowstate_t;

typedef struct {
   float_shape_t *cow, *bubble;
   int bubble_width, bubble_height;
   GdkPixbuf *cow_pixbuf, *bubble_pixbuf;
   cowstate_t state;
   int transition_timeout;
   int display_time;
} xcowsay_t;

static xcowsay_t xcowsay;

static cowstate_t next_state(cowstate_t state)
{
   switch (state) {
   case csLeadIn:
      return csDisplay;
   case csDisplay:
      return csLeadOut;
   case csLeadOut:
      return csCleanup;
   case csCleanup:
   default:
      return csCleanup;
   }
}

// TODO: Change this to use the actual max path length...
#define MAX_COW_PATH 256
static GdkPixbuf *load_cow()
{
   char cow_path[MAX_COW_PATH];
   snprintf(cow_path, MAX_COW_PATH, "%s/%s_%s.png", DATADIR,
            get_string_option("image_base"), get_string_option("cow_size"));
   
   GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(cow_path, NULL);
   if (NULL == pixbuf) {
      fprintf(stderr, i18n("Failed to load cow image: %s\n"), cow_path);
      exit(EXIT_FAILURE);
   }
   return pixbuf;
}

static gboolean tick(gpointer data)
{
   xcowsay.transition_timeout -= TICK_TIMEOUT;
   if (xcowsay.transition_timeout <= 0) {
      xcowsay.state = next_state(xcowsay.state);
      switch (xcowsay.state) {
      case csLeadIn:
         fprintf(stderr, "Internal Error: Invalid state csLeadIn\n");
         exit(EXIT_FAILURE);
      case csDisplay:
         show_shape(xcowsay.bubble);
         xcowsay.transition_timeout = xcowsay.display_time;
         break;
      case csLeadOut:
         hide_shape(xcowsay.bubble);
         xcowsay.transition_timeout = get_int_option("lead_out_time");
         break;
      case csCleanup:
         destroy_shape(xcowsay.cow);
         xcowsay.cow = NULL;
         destroy_shape(xcowsay.bubble);
         xcowsay.bubble = NULL;
         break;
      }
   }
   
   return (xcowsay.state != csCleanup);
}

static gboolean cow_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
   if (csDisplay == xcowsay.state) {
      xcowsay.transition_timeout = 0;
      tick(NULL);
   }
   return true;
}

void cowsay_init(int *argc, char ***argv)
{
   gtk_init(argc, argv);
   
   xcowsay.cow = NULL;
   xcowsay.bubble = NULL;
   xcowsay.bubble_pixbuf = NULL;
   xcowsay.cow_pixbuf = load_cow();
}

static int count_words(const char *s)
{
   bool last_was_space = false;
   int words;
   for (words = 1; *s; s++) {
      if (isspace(*s) && !last_was_space) {
         words++;
         last_was_space = true;
      }
      else
         last_was_space = false;
   }
   return words;
}

static void normal_setup(const char *text, bool debug, cowmode_t mode)
{
   char *text_copy = strdup(text);
   
   // Trim any trailing newline
   size_t len = strlen(text_copy);
   if ('\n' == text_copy[len-1])
      text_copy[len-1] = '\0';
   
   // Count the words and work out the display time, if neccessary
   xcowsay.display_time = get_int_option("display_time");
   if (xcowsay.display_time < 0) {
      int words = count_words(text_copy);
      xcowsay.display_time = words * get_int_option("reading_speed");
      debug_msg("Calculated display time as %dms from %d words\n",
                xcowsay.display_time, words);
   }
   else {
      debug_msg("Using default display time %dms\n", xcowsay.display_time);
   }

   int min_display = get_int_option("min_display_time");
   int max_display = get_int_option("max_display_time");
   if (xcowsay.display_time < min_display) {
      xcowsay.display_time = min_display;
      debug_msg("Display time too short: clamped to %d\n", min_display);
   }
   else if (xcowsay.display_time > max_display) {
      xcowsay.display_time = max_display;
      debug_msg("Display time too long: clamped to %d\n", max_display);
   }
   
   xcowsay.bubble_pixbuf = make_text_bubble(text_copy, &xcowsay.bubble_width,
                                            &xcowsay.bubble_height, mode);
   free(text_copy);
}

static void dream_setup(const char *file, bool debug)
{
   debug_msg("Dreaming file: %s\n", file);

   // TODO: calculate display time
   xcowsay.display_time = 10000;

   xcowsay.bubble_pixbuf = make_dream_bubble(file, &xcowsay.bubble_width,
                                             &xcowsay.bubble_height);   
}

void display_cow(bool debug, const char *text, bool run_main, cowmode_t mode)
{
   switch (mode) {
   case COWMODE_NORMAL:
   case COWMODE_THINK:
      normal_setup(text, debug, mode);
      break;
   case COWMODE_DREAM:
      dream_setup(text, debug);
      break;
   default:
      fprintf(stderr, "Error: Unsupported cow mode %d\n", mode);
      exit(1);
   }
   
   g_assert(xcowsay.cow_pixbuf);
   xcowsay.cow = make_shape_from_pixbuf(xcowsay.cow_pixbuf);
   
   int total_width = shape_width(xcowsay.cow) + BUBBLE_XOFFSET
      + xcowsay.bubble_width;
   int total_height = max(shape_height(xcowsay.cow), xcowsay.bubble_height);

   int bubble_off = max((xcowsay.bubble_height - shape_height(xcowsay.cow))/2, 0);

   GdkScreen *screen = gdk_screen_get_default();
   int area_w = gdk_screen_get_width(screen) - total_width;
   int area_h = gdk_screen_get_height(screen) - total_height;

   // Fit the cow on the screen as best as we can
   // The area can't be be zero or we'd get an FPE
   if (area_w < 1)
      area_w = 1;
   if (area_h < 1)
      area_h = 1;

   move_shape(xcowsay.cow, rand()%area_w, bubble_off + rand()%area_h);
   show_shape(xcowsay.cow);

   xcowsay.bubble = make_shape_from_pixbuf(xcowsay.bubble_pixbuf);   
   int bx = shape_x(xcowsay.cow) + shape_width(xcowsay.cow) + BUBBLE_XOFFSET;
   int by = shape_y(xcowsay.cow)
      + (shape_height(xcowsay.cow) - shape_height(xcowsay.bubble))/2;
   move_shape(xcowsay.bubble, bx, by);
   
   xcowsay.state = csLeadIn;
   xcowsay.transition_timeout = get_int_option("lead_in_time");
   g_timeout_add(TICK_TIMEOUT, tick, NULL);

   GdkEventMask events = gdk_window_get_events(shape_window(xcowsay.cow)->window);
   events |= GDK_BUTTON_PRESS_MASK;
   gdk_window_set_events(shape_window(xcowsay.cow)->window, events);
   g_signal_connect(G_OBJECT(shape_window(xcowsay.cow)), "button-press-event",
                    G_CALLBACK(cow_clicked), NULL);

   if (run_main)
      gtk_main();

   g_object_unref(xcowsay.bubble_pixbuf);
   xcowsay.bubble_pixbuf = NULL;
}

#ifndef WITH_DBUS

bool try_dbus(bool debug, const char *text, cowmode_t mode)
{
   debug_msg("Skipping DBus (disabled by configure)\n");
   return false;
}

#else 

bool try_dbus(bool debug, const char *text, cowmode_t mode)
{
   DBusGConnection *connection;
   GError *error;
   DBusGProxy *proxy;

   g_type_init();
   error = NULL;
   connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
   if (NULL == connection) {
      debug_err("Failed to open connection to bus: %s\n", error->message);
      g_error_free(error);
      return false;
   }

   proxy = dbus_g_proxy_new_for_name(connection, XCOWSAY_NAMESPACE,
                                     XCOWSAY_PATH, XCOWSAY_NAMESPACE);
   g_assert(proxy);

   error = NULL;
   if (!dbus_g_proxy_call(proxy, "ShowCow", &error, G_TYPE_STRING, text,
                          G_TYPE_INVALID, G_TYPE_INVALID)) {
      debug_err("ShowCow failed: %s\n", error->message);
      g_error_free(error);
      return false;
   }

   return true;
}

#endif /* #ifndef WITH_DBUS */       

void display_cow_or_invoke_daemon(bool debug, const char *text, cowmode_t mode)
{
   if (!try_dbus(debug, text, mode))
      display_cow(debug, text, true, mode);
}
