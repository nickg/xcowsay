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

   colormap = gdk_colormap_get_system();
   gdk_color_black(colormap, &black);
   gdk_color_white(colormap, &white);

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
         bright_green = 0 != p[0] && 255 != p[1] && 0 != p[2];
         if ((has_alpha && 255 == p[3])  // p[3] is alpha channel
             || bright_green)            // Green is also alpha
            gdk_draw_point(shape->mask_bitmap, gc, x, y);
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
   
   s->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_decorated(GTK_WINDOW(s->window), FALSE);
   gtk_window_set_title(GTK_WINDOW(s->window), "shape");
   gtk_window_set_skip_taskbar_hint(GTK_WINDOW(s->window), TRUE);
   
   s->image = gtk_image_new_from_pixbuf(pixbuf);
   gtk_container_add(GTK_CONTAINER(s->window), s->image);

   get_alpha_mask(s);
   //gtk_widget_shape_combine_mask(s->window, s->mask_bitmap, 0, 0);
   
   g_signal_connect(G_OBJECT(s->window), "destroy",
                    G_CALLBACK(quit_callback), NULL);

   return s;
}

void show_shape(float_shape_t *shape)
{
   gtk_window_move(GTK_WINDOW(shape->window), shape->x, shape->y);
   gtk_window_resize(GTK_WINDOW(shape->window), shape->width, shape->height);
   gtk_widget_show_all(shape->window);
}

void move_shape(float_shape_t *shape, int x, int y)
{
   shape->x = x;
   shape->y = y;
   gtk_window_move(GTK_WINDOW(shape->window), shape->x, shape->y);
}
