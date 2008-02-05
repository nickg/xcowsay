#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>

#define SPEED  30   // Horizontal speed in pixels per 100ms

typedef struct {
   GtkWidget *window;
   GtkWidget *cow_image;
   GdkPixbuf *cow_pixbuf;
   int x, y, width, height;
   int screen_width;
} xcowsay_t;

static xcowsay_t xcowsay;

static void get_alpha_mask(GdkDrawable *bitmap)
{
   GdkColormap *colormap;
   GdkColor black;
   GdkColor white;
   GdkGC *gc;
   int rowstride, nchannels, x, y;
   guchar *pixels, *p;

   colormap = gdk_colormap_get_system();
   gdk_color_black(colormap, &black);
   gdk_color_white(colormap, &white);

   gc = gdk_gc_new(bitmap);
   
   gdk_gc_set_foreground(gc, &black);
   gdk_gc_set_background(gc, &white);
   gdk_draw_rectangle (bitmap, gc, TRUE, 0, 0, xcowsay.width, xcowsay.height);

   nchannels = gdk_pixbuf_get_n_channels(xcowsay.cow_pixbuf);
   g_assert(gdk_pixbuf_get_colorspace(xcowsay.cow_pixbuf) == GDK_COLORSPACE_RGB);
   g_assert(gdk_pixbuf_get_bits_per_sample(xcowsay.cow_pixbuf) == 8);
   g_assert(gdk_pixbuf_get_has_alpha(xcowsay.cow_pixbuf));
   g_assert(nchannels == 4);
   
   rowstride = gdk_pixbuf_get_rowstride(xcowsay.cow_pixbuf);
   pixels = gdk_pixbuf_get_pixels(xcowsay.cow_pixbuf);

   gdk_gc_set_foreground(gc, &white);
   gdk_gc_set_background(gc, &black);
   
   for (y = 0; y < xcowsay.height; y++) {
      for (x = 0; x < xcowsay.width; x++) {
         p = pixels + y*rowstride + x*nchannels;
         if (255 == p[3])   // p[3] is alpha channel
            gdk_draw_point(bitmap, gc, x, y);
      }
   }
}

static void quit_callback(GtkWidget *widget, gpointer data)
{
   gtk_main_quit();
}

gboolean tick(gpointer data)
{
   if (xcowsay.x + xcowsay.width < xcowsay.screen_width / 2) {
      xcowsay.x += SPEED;
      gtk_window_move(GTK_WINDOW(xcowsay.window), xcowsay.x, xcowsay.y);
   }
   return true;
}

int main(int argc, char **argv)
{
   GdkDrawable *window_shape_bitmap;
   GdkScreen *screen;
   
   gtk_init(&argc, &argv);
   
   xcowsay.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_decorated(GTK_WINDOW(xcowsay.window), FALSE);
   gtk_window_set_resizable(GTK_WINDOW(xcowsay.window), FALSE);
   gtk_window_set_title(GTK_WINDOW(xcowsay.window), "xcowsay");
   gtk_window_set_skip_taskbar_hint(GTK_WINDOW(xcowsay.window), TRUE);
   
   xcowsay.cow_pixbuf = gdk_pixbuf_new_from_file("cow_med.png", NULL);
   if (NULL == xcowsay.cow_pixbuf) {
      fprintf(stderr, "Failed to load cow image!\n");
      return EXIT_FAILURE;
   }
   xcowsay.width = gdk_pixbuf_get_width(xcowsay.cow_pixbuf);
   xcowsay.height = gdk_pixbuf_get_height(xcowsay.cow_pixbuf);

   window_shape_bitmap =
      (GdkDrawable*)gdk_pixmap_new(NULL, xcowsay.width, xcowsay.height, 1);
   get_alpha_mask(window_shape_bitmap);
   gtk_widget_shape_combine_mask(xcowsay.window, window_shape_bitmap, 0, 0);
   gdk_pixmap_unref(window_shape_bitmap);
   
   xcowsay.cow_image = gtk_image_new_from_pixbuf(xcowsay.cow_pixbuf);
   gtk_container_add(GTK_CONTAINER(xcowsay.window), xcowsay.cow_image);

   screen = gtk_widget_get_screen(xcowsay.window);
   xcowsay.screen_width = gdk_screen_get_width(screen);

   // Ok... this is a terrible hack *sniff*
   // In order to stop the cow flashing up before it's moved offscreen
   // we iconify it, show it, deiconify it, then move it (hopefully
   // before it gets painted again!)
   gtk_window_iconify(GTK_WINDOW(xcowsay.window));
   gtk_widget_show_all(xcowsay.window);

   xcowsay.x = -xcowsay.width;
   xcowsay.y = 100;
   gtk_window_deiconify(GTK_WINDOW(xcowsay.window));
   gtk_window_move(GTK_WINDOW(xcowsay.window), xcowsay.x, xcowsay.y);   
   
   g_signal_connect(G_OBJECT(xcowsay.window), "destroy",
                    G_CALLBACK(quit_callback), NULL);

   g_timeout_add(100, tick, NULL);
   
   gtk_main();
   
   return EXIT_SUCCESS;
}
