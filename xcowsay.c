#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>

#include "floating_shape.h"

#define SPEED    10  // Horizontal speed in pixels per 100ms
#define LEFT_BUF 5   // Amount of pixels to leave after cow's tail

typedef struct {
   float_shape_t *cow, *bubble;
} xcowsay_t;

static xcowsay_t xcowsay;

static GdkPixbuf *load_cow()
{
   GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("cow_large.png", NULL);
   if (NULL == pixbuf) {
      fprintf(stderr, "Failed to load cow image!\n");
      exit(EXIT_FAILURE);
   }
   return pixbuf;
}

static GdkPixbuf *create_bubble()
{
   GdkGC *bubble_gc;
   GdkColor black, white, bright_green;
   GdkColormap *colormap;
   GdkPixmap *bubble_pixmap;
   int bubble_width, bubble_height;

   bubble_width = 200;
   bubble_height = 100;

   bubble_pixmap = gdk_pixmap_new(shape_window(xcowsay.cow)->window,
                                  bubble_width, bubble_height, -1);
   bubble_gc = gdk_gc_new(bubble_pixmap);
   
   colormap = gdk_colormap_get_system();
   gdk_color_black(colormap, &black);
   gdk_color_white(colormap, &white);

   bright_green.red = 0;
   bright_green.green = 65535;   // Bright green is alpha
   bright_green.blue = 0;
   gdk_gc_set_background(bubble_gc, &black);
   gdk_gc_set_rgb_fg_color(bubble_gc, &bright_green);

   gdk_draw_rectangle(bubble_pixmap, bubble_gc, TRUE, 0, 0,
                      bubble_width, bubble_height);

   gdk_gc_set_foreground(bubble_gc, &white);
   gdk_draw_arc(bubble_pixmap, bubble_gc, TRUE, 0, 0,
                bubble_width, bubble_height, 0, 360*64);

   return gdk_pixbuf_get_from_drawable(NULL, bubble_pixmap, NULL,
                                       0, 0, 0, 0, bubble_width,
                                       bubble_height);
}

int main(int argc, char **argv)
{   
   gtk_init(&argc, &argv);

   xcowsay.cow = make_shape_from_pixbuf(load_cow());
   show_shape(xcowsay.cow);

   xcowsay.bubble = make_shape_from_pixbuf(create_bubble());
   show_shape(xcowsay.bubble);   
   
   gtk_main();
   
   return EXIT_SUCCESS;
}
