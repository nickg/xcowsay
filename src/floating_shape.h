/*  floating_shape.h -- Low-ish level window creation and management.
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

#ifndef INC_FLOATING_SHAPE_H
#define INC_FLOATING_SHAPE_H

#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>

/*
 * A widget type thing which used the XShape extension.
 */
typedef struct {
   GtkWidget *window;
   GtkWidget *image;
   GdkPixbuf *pixbuf;
   GdkDrawable *mask_bitmap;
   int x, y, width, height;
} float_shape_t;

float_shape_t *make_shape_from_pixbuf(GdkPixbuf *pixbuf);
void move_shape(float_shape_t *shape, int x, int y);
void show_shape(float_shape_t *shape);
void hide_shape(float_shape_t *shape);
void destroy_shape(float_shape_t *shape);

#define shape_window(s) (s->window)
#define shape_x(s) (s->x)
#define shape_y(s) (s->y)
#define shape_width(s) (s->width)
#define shape_height(s) (s->height)

#endif
