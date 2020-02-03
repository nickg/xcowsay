/*  display_cow.c -- Display a cow in a popup window.
 *  Copyright (C) 2008-2020  Nick Gasson
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <gtk/gtk.h>

#ifdef WITH_DBUS
#include <dbus/dbus-glib-bindings.h>
#define XCOWSAY_PATH "/uk/me/doof/Cowsay"
#define XCOWSAY_NAMESPACE "uk.me.doof.Cowsay"
#endif

#include "floating_shape.h"
#include "display_cow.h"
#include "settings.h"
#include "i18n.h"

GdkPixbuf *make_text_bubble(char *text, int *p_width, int *p_height,
                            int max_width, cowmode_t mode);
GdkPixbuf *make_dream_bubble(const char *file, int *p_width, int *p_height);

#define TICK_TIMEOUT   100

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
   int screen_width, screen_height;
} xcowsay_t;

static xcowsay_t xcowsay;

static gboolean tick(gpointer data);

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

static GdkPixbuf *load_cow()
{
   char *cow_path;
   const char *alt_image = get_string_option("alt_image");

   if (*alt_image)
      cow_path = strdup(alt_image);
   else
      asprintf(&cow_path, "%s/%s_%s.png", DATADIR,
         get_string_option("image_base"),
         get_string_option("cow_size"));

   GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(cow_path, NULL);
   if (NULL == pixbuf) {
      fprintf(stderr, i18n("Failed to load cow image: %s\n"), cow_path);
      exit(EXIT_FAILURE);
   }

   free(cow_path);
   return pixbuf;
}

static gboolean cow_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
   if (csDisplay == xcowsay.state) {
      xcowsay.transition_timeout = 0;
      tick(NULL);
   }
   return true;
}

/*
 * Set up a shape to call cow_clicked when it's clicked.
 */
static void close_when_clicked(float_shape_t *shape)
{
   GdkWindow *w = gtk_widget_get_window(shape_window(shape));
   GdkEventMask events = gdk_window_get_events(w);
   events |= GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK;
   gdk_window_set_events(w, events);
   g_signal_connect(G_OBJECT(shape_window(shape)),
                    get_string_option("close_event"),
                    G_CALLBACK(cow_clicked), NULL);
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
         close_when_clicked(xcowsay.bubble);
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

void cowsay_init(int *argc, char ***argv)
{
   // Window positioning doesn't work on Wayland
   setenv("GDK_BACKEND", "x11", 1);

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
   if (xcowsay.display_time == 0) {
      xcowsay.display_time = INT_MAX;
      debug_msg("Set display time to permanent\n");
   }
   else if (xcowsay.display_time < min_display) {
      xcowsay.display_time = min_display;
      debug_msg("Display time too short: clamped to %d\n", min_display);
   }
   else if (xcowsay.display_time > max_display) {
      xcowsay.display_time = max_display;
      debug_msg("Display time too long: clamped to %d\n", max_display);
   }

   const int cow_width = shape_width(xcowsay.cow);
   const int max_width = xcowsay.screen_width - cow_width;

   if (xcowsay.bubble_pixbuf != NULL)
      g_object_unref(xcowsay.bubble_pixbuf);

   xcowsay.bubble_pixbuf = make_text_bubble(
      text_copy, &xcowsay.bubble_width, &xcowsay.bubble_height,
      max_width, mode);
   free(text_copy);
}

static void dream_setup(const char *file, bool debug)
{
   debug_msg("Dreaming file: %s\n", file);

   xcowsay.display_time = get_int_option("display_time");
   if (xcowsay.display_time < 0)
      xcowsay.display_time = get_int_option("dream_time");

   if (xcowsay.bubble_pixbuf != NULL)
      g_object_unref(xcowsay.bubble_pixbuf);

   xcowsay.bubble_pixbuf = make_dream_bubble(file, &xcowsay.bubble_width,
                                             &xcowsay.bubble_height);
}

void display_cow(bool debug, const char *text, cowmode_t mode)
{
   GdkScreen *screen = gdk_screen_get_default();

   gint n_monitors = gdk_screen_get_n_monitors(screen);

   gint pick = get_int_option("monitor");
   if (pick < 0 || pick >= n_monitors)
      pick = random() % n_monitors;

   GdkRectangle geom;
   gdk_screen_get_monitor_geometry(screen, pick, &geom);

   xcowsay.screen_width = geom.width;
   xcowsay.screen_height = geom.height;

   g_assert(xcowsay.cow_pixbuf);
   xcowsay.cow = make_shape_from_pixbuf(xcowsay.cow_pixbuf);

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

   xcowsay.bubble = make_shape_from_pixbuf(xcowsay.bubble_pixbuf);

   int total_width = shape_width(xcowsay.cow)
      + get_int_option("bubble_x")
      + xcowsay.bubble_width;
   int total_height = max(shape_height(xcowsay.cow), xcowsay.bubble_height);

   int bubble_off = max((xcowsay.bubble_height - shape_height(xcowsay.cow))/2, 0);

   int area_w = xcowsay.screen_width - total_width;
   int area_h = xcowsay.screen_height - total_height;

   // Fit the cow on the screen as best as we can
   // The area can't be be zero or we'd get an FPE
   if (area_w < 1)
      area_w = 1;
   if (area_h < 1)
      area_h = 1;

   int cow_x = get_int_option("at_x");
   if (cow_x < 0)
      cow_x = random() % area_w;
   else if (cow_x >= area_w)
      cow_x = area_w - 1;

   int cow_y = get_int_option("at_y");
   if (cow_y < 0)
      cow_y = random() % area_h;
   else if (cow_y >= area_h)
      cow_y = area_h - 1;

   if (get_bool_option("left")) {
      move_shape(xcowsay.cow,
                 geom.x + cow_x + xcowsay.bubble_width,
                 geom.y + bubble_off + cow_y);
      show_shape(xcowsay.cow);

      int bx = shape_x(xcowsay.cow) - xcowsay.bubble_width
         + get_int_option("bubble_x");
      int by = shape_y(xcowsay.cow)
         + (shape_height(xcowsay.cow) - shape_height(xcowsay.bubble))/2
         + get_int_option("bubble_y");
      move_shape(xcowsay.bubble, bx, by);
   }
   else {
      move_shape(xcowsay.cow,
                 geom.x + cow_x,
                 geom.y + bubble_off + cow_y);
      show_shape(xcowsay.cow);

      int bx = shape_x(xcowsay.cow) + shape_width(xcowsay.cow)
         + get_int_option("bubble_x");
      int by = shape_y(xcowsay.cow)
         + (shape_height(xcowsay.cow) - shape_height(xcowsay.bubble))/2
         + get_int_option("bubble_y");
      move_shape(xcowsay.bubble, bx, by);
   }

   xcowsay.state = csLeadIn;
   xcowsay.transition_timeout = get_int_option("lead_in_time");
   g_timeout_add(TICK_TIMEOUT, tick, NULL);

   close_when_clicked(xcowsay.cow);
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

   const char *method = NULL;
   switch (mode) {
   case COWMODE_NORMAL:
      method = "ShowCow";
      break;
   case COWMODE_THINK:
      method = "Think";
      break;
   case COWMODE_DREAM:
      method = "Dream";
      break;
   default:
      g_assert(false);
   }

   error = NULL;
   if (!dbus_g_proxy_call(proxy, method, &error, G_TYPE_STRING, text,
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
   if (!try_dbus(debug, text, mode)) {
      display_cow(debug, text, mode);
      gtk_main();
   }
}
