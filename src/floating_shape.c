/*  floating_shape.c -- Low-ish level window creation and management.
 *  Copyright (C) 2008, 2011  Nick Gasson
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
   float_shape_t *s = (float_shape_t*)malloc(sizeof(float_shape_t));
   g_assert(s);
   return s;
}

static void quit_callback(GtkWidget *widget, gpointer data)
{
   gtk_main_quit();
}

static void get_alpha_mask(float_shape_t *shape)
{
   GdkColormap *colormap;
   GdkColor black;
   GdkColor white;
   GdkGC *gc;
   int rowstride, nchannels, x, y;
   guchar *pixels, *p;
   bool bright_green, has_alpha;
   gboolean ok;

   colormap = gdk_colormap_get_system();

   black.red = 0;
   black.green = 0;
   black.blue = 0;
   ok = gdk_colormap_alloc_color(colormap, &black, FALSE, TRUE);
   g_assert(ok);

   white.red = 0xffff;
   white.green = 0xffff;
   white.blue = 0xffff;
   ok = gdk_colormap_alloc_color(colormap, &white, FALSE, TRUE);
   g_assert(ok);
   
   shape->mask_bitmap =
      (GdkDrawable*)gdk_pixmap_new(NULL, shape->width, shape->height, 1);
   gc = gdk_gc_new(shape->mask_bitmap);
   
   gdk_gc_set_foreground(gc, &black);
   gdk_gc_set_background(gc, &white);
   gdk_draw_rectangle(shape->mask_bitmap, gc, TRUE, 0, 0,
                      shape->width, shape->height);

   nchannels = gdk_pixbuf_get_n_channels(shape->pixbuf);
   g_assert(gdk_pixbuf_get_colorspace(shape->pixbuf) == GDK_COLORSPACE_RGB);
   g_assert(gdk_pixbuf_get_bits_per_sample(shape->pixbuf) == 8);

   has_alpha = gdk_pixbuf_get_has_alpha(shape->pixbuf);
   
   rowstride = gdk_pixbuf_get_rowstride(shape->pixbuf);
   pixels = gdk_pixbuf_get_pixels(shape->pixbuf);

   gdk_gc_set_foreground(gc, &white);
   gdk_gc_set_background(gc, &black);
   
   for (y = 0; y < shape->height; y++) {
      for (x = 0; x < shape->width; x++) {
         p = pixels + y*rowstride + x*nchannels;
         bright_green = 0 == p[0] && 255 == p[1] && 0 == p[2];
         if (has_alpha) {
            if (255 == p[3])  // p[3] is alpha channel
               gdk_draw_point(shape->mask_bitmap, gc, x, y);
         } 
         else if (!bright_green) {   // Bright green is alpha for RGB images
            gdk_draw_point(shape->mask_bitmap, gc, x, y);
         }
      }
   }
}

float_shape_t *make_shape_from_pixbuf(GdkPixbuf *pixbuf)
{
   float_shape_t *s = alloc_shape();
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
   
   s->image = gtk_image_new_from_pixbuf(pixbuf);
   gtk_container_add(GTK_CONTAINER(s->window), s->image);

   get_alpha_mask(s);
   gtk_widget_shape_combine_mask(s->window, s->mask_bitmap, 0, 0);
   
   g_signal_connect(G_OBJECT(s->window), "destroy",
                    G_CALLBACK(quit_callback), NULL);

   return s;
}

void show_shape(float_shape_t *shape)
{
   gtk_window_move(GTK_WINDOW(shape->window), shape->x, shape->y);
   gtk_window_resize(GTK_WINDOW(shape->window), shape->width, shape->height);
   gtk_widget_show_all(shape->window);
 
   gdk_window_set_back_pixmap(gtk_widget_get_window(shape->window), NULL, TRUE);
}

void hide_shape(float_shape_t *shape)
{
   gtk_widget_hide_all(shape->window);
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
   g_object_unref(shape->mask_bitmap);
   
   free(shape);
}
