/*  floating_shape.c -- Low-ish level window creation and management.
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

#include <stdlib.h>
#include <stdbool.h>
#include "floating_shape.h"

static float_shape_t *alloc_shape()
{
   float_shape_t *s = (float_shape_t*)calloc(1, sizeof(float_shape_t));
   g_assert(s);
   return s;
}

static void quit_callback(GtkWidget *widget, gpointer data)
{
   gtk_main_quit();
}

static gboolean draw_shape(GtkWidget *widget, GdkEventExpose *event,
                           gpointer userdata)
{
   float_shape_t *s = (float_shape_t *)userdata;
   cairo_t *cr;
   int width, height;

   cr = gdk_cairo_create(gtk_widget_get_window(widget));

   cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);

   gtk_window_get_size(GTK_WINDOW(widget), &width, &height);

   gdk_cairo_set_source_pixbuf(cr, s->pixbuf, 0, 0);
   cairo_paint(cr);

   cairo_destroy(cr);
   return FALSE;
}

static void screen_changed(GtkWidget *widget, GdkScreen *screen,
                           gpointer user_data)
{
   GdkVisual *visual;

   visual = gdk_screen_get_rgba_visual(screen);
   g_assert(visual);

   gtk_widget_set_visual(widget, visual);
}

float_shape_t *make_shape_from_pixbuf(GdkPixbuf *pixbuf)
{
   float_shape_t *s;
   GdkScreen *screen;
   GdkVisual *visual;

   s = alloc_shape();
   s->x = 0;
   s->y = 0;
   s->pixbuf = pixbuf;
   s->width = gdk_pixbuf_get_width(pixbuf);
   s->height = gdk_pixbuf_get_height(pixbuf);

   s->window = gtk_window_new(GTK_WINDOW_POPUP);
   gtk_window_set_decorated(GTK_WINDOW(s->window), FALSE);
   gtk_window_set_title(GTK_WINDOW(s->window), "shape");
   gtk_window_set_skip_taskbar_hint(GTK_WINDOW(s->window), TRUE);
   gtk_window_set_keep_above(GTK_WINDOW(s->window), TRUE);
   gtk_window_set_resizable(GTK_WINDOW(s->window), FALSE);
   gtk_window_set_default_size(GTK_WINDOW(s->window), s->width, s->height);

   gtk_widget_set_app_paintable(GTK_WIDGET(s->window), TRUE);

   screen = gtk_widget_get_screen(s->window);
   visual = gdk_screen_get_rgba_visual(screen);
   g_assert(visual);
   gtk_widget_set_visual(s->window, visual);

   g_signal_connect(G_OBJECT(s->window), "draw",
                    G_CALLBACK(draw_shape), s);
   g_signal_connect(G_OBJECT(s->window), "screen-changed",
                    G_CALLBACK(screen_changed), s);
   g_signal_connect(G_OBJECT(s->window), "destroy",
                    G_CALLBACK(quit_callback), NULL);

   return s;
}

void show_shape(float_shape_t *shape)
{
   gtk_window_resize(GTK_WINDOW(shape->window), shape->width, shape->height);
   gtk_window_move(GTK_WINDOW(shape->window), shape->x, shape->y);
   gtk_widget_show_all(shape->window);
}

void hide_shape(float_shape_t *shape)
{
   gtk_widget_hide(shape->window);
}

void move_shape(float_shape_t *shape, int x, int y)
{
   shape->x = x;
   shape->y = y;
   gtk_window_move(GTK_WINDOW(shape->window), shape->x, shape->y);
}

void destroy_shape(float_shape_t *shape)
{
   g_assert(shape);

   gtk_widget_destroy(shape->window);

   free(shape);
}
