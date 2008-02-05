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
void free_shape(float_shape_t *shape);

#define shape_window(s) (s->window)
#define shape_x(s) (s->x)
#define shape_y(s) (s->y)
#define shape_width(s) (s->width)
#define shape_height(s) (s->height)

#endif
