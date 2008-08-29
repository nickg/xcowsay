/*  bubblegen.c -- Generate various sorts of bubbles.
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
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "floating_shape.h"
#include "display_cow.h"
#include "settings.h"
#include "i18n.h"

#define LEFT_BUF       5   // Amount of pixels to leave after cow's tail
#define TIP_WIDTH      20  // Length of the triangle bit on the speech bubble
#define THINK_WIDTH    80 // Spaces for thinking circles
#define CORNER_RADIUS  30  // Radius of corners on the speech bubble
#define CORNER_DIAM    CORNER_RADIUS*2
#define BUBBLE_BORDER  5   // Pixels to leave free around edge of bubble
#define MIN_TIP_HEIGHT 15

// These next ones control the size and position of the "thinking circles"
// (or whatever you call them)
#define BIG_KIRCLE_X      38
#define BIG_KIRCLE_Y      70
#define BIG_KIRCLE_DIAM   35

#define SMALL_KIRCLE_X    5
#define SMALL_KIRCLE_Y    40
#define SMALL_KIRCLE_DIAM 20

// Min distance from top of the big kircle to the top of the bubble
#define KIRCLE_TOP_MIN  10

typedef struct {
   int width, height;
   GdkPixmap *pixmap;
   GdkGC *gc;
} bubble_t;

typedef enum { NORMAL, THOUGHT } bubble_style_t;

static void bubble_init(bubble_t *b, bubble_style_t style)
{
   GdkColor black, white, bright_green;
   GdkColormap *colormap;
   GdkPoint tip_points[5];
   
   b->pixmap = gdk_pixmap_new(NULL, b->width, b->height, 24);
   b->gc = gdk_gc_new(b->pixmap);
   
   colormap = gdk_colormap_get_system();
   gdk_color_black(colormap, &black);
   gdk_color_white(colormap, &white);

   bright_green.red = 0;
   bright_green.green = 65535;   // Bright green is alpha
   bright_green.blue = 0;
   gdk_gc_set_background(b->gc, &black);
   gdk_gc_set_rgb_fg_color(b->gc, &bright_green);

   gdk_draw_rectangle(b->pixmap, b->gc, TRUE, 0, 0, b->width, b->height);
   
   b->width -= BUBBLE_BORDER;
   b->height -= BUBBLE_BORDER;

   // Space between cow and bubble
   int middle = style == NORMAL ? TIP_WIDTH : THINK_WIDTH;

   // Draw the white corners
   gdk_gc_set_foreground(b->gc, &white);
   gdk_draw_arc(b->pixmap, b->gc, TRUE, middle + BUBBLE_BORDER,
                BUBBLE_BORDER, CORNER_DIAM, CORNER_DIAM, 90*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, TRUE, middle + BUBBLE_BORDER,
                b->height - CORNER_DIAM, CORNER_DIAM,
                CORNER_DIAM, 180*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, TRUE,
                b->width - CORNER_DIAM - BUBBLE_BORDER,
                b->height - CORNER_DIAM, CORNER_DIAM,
                CORNER_DIAM, 270*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, TRUE,
                b->width - CORNER_DIAM - BUBBLE_BORDER,
                BUBBLE_BORDER, CORNER_DIAM, CORNER_DIAM, 0*64, 90*64);

   // Fill in the middle of the bubble
   gdk_draw_rectangle(b->pixmap, b->gc, TRUE,
                      CORNER_RADIUS + middle + BUBBLE_BORDER,
                      BUBBLE_BORDER,
                      b->width - middle - BUBBLE_BORDER - CORNER_DIAM,
                      b->height - BUBBLE_BORDER);
   gdk_draw_rectangle(b->pixmap, b->gc, TRUE,
                      middle + BUBBLE_BORDER, BUBBLE_BORDER + CORNER_RADIUS,
                      b->width - middle - BUBBLE_BORDER*2,
                      b->height - BUBBLE_BORDER - CORNER_DIAM);

   if (style == NORMAL) {
      // The points on the tip part
      int tip_compute_offset = (b->height - BUBBLE_BORDER - CORNER_DIAM)/3;
      int tip_offset[3] = { tip_compute_offset, tip_compute_offset, tip_compute_offset };
      if (tip_compute_offset < MIN_TIP_HEIGHT) {
         int new_offset = (b->height - BUBBLE_BORDER - CORNER_DIAM - MIN_TIP_HEIGHT)/2;
         tip_offset[0] = new_offset;
         tip_offset[1] = MIN_TIP_HEIGHT;
         tip_offset[2] = new_offset;
      }
      
      tip_points[0].x = middle + BUBBLE_BORDER;
      tip_points[0].y = BUBBLE_BORDER + CORNER_RADIUS;
      tip_points[1].x = middle + BUBBLE_BORDER;
      tip_points[1].y = BUBBLE_BORDER + CORNER_RADIUS + tip_offset[0];
      tip_points[2].x = BUBBLE_BORDER;
      tip_points[2].y = BUBBLE_BORDER + CORNER_RADIUS + tip_offset[0] + tip_offset[1]/2;
      tip_points[3].x = middle + BUBBLE_BORDER;
      tip_points[3].y = BUBBLE_BORDER + CORNER_RADIUS + tip_offset[0] + tip_offset[1];
      tip_points[4].x = middle + BUBBLE_BORDER;
      tip_points[4].y = b->height - CORNER_RADIUS;
      
      gdk_draw_polygon(b->pixmap, b->gc, TRUE, tip_points, 5);
   }
   else {
      // Incrementally move the top kircle down so it's within the
      // bubble's border
      int big_y = BIG_KIRCLE_Y;
      int small_y = SMALL_KIRCLE_Y;
      
      while (big_y + KIRCLE_TOP_MIN > b->height/2) {
         big_y /= 2;
         small_y /= 2;
      }
      
      // Draw two think kircles
      gdk_draw_arc(b->pixmap, b->gc, TRUE,
                   BIG_KIRCLE_X,
                   b->height/2 - big_y, BIG_KIRCLE_DIAM,
                   BIG_KIRCLE_DIAM, 0, 360*64);

      gdk_draw_arc(b->pixmap, b->gc, TRUE,
                   SMALL_KIRCLE_X,
                   b->height/2 - small_y, SMALL_KIRCLE_DIAM,
                   SMALL_KIRCLE_DIAM, 0, 360*64);
      
      gdk_gc_set_line_attributes(b->gc, 4, GDK_LINE_SOLID,
                                 GDK_CAP_ROUND, GDK_JOIN_ROUND);
      gdk_gc_set_foreground(b->gc, &black);
      gdk_draw_arc(b->pixmap, b->gc, FALSE,
                   BIG_KIRCLE_X,
                   b->height/2 - big_y, BIG_KIRCLE_DIAM,
                   BIG_KIRCLE_DIAM, 0, 360*64);

      gdk_draw_arc(b->pixmap, b->gc, FALSE,
                   SMALL_KIRCLE_X,
                   b->height/2 - small_y, SMALL_KIRCLE_DIAM,
                   SMALL_KIRCLE_DIAM, 0, 360*64);
   }

   // Draw the black rounded corners
   gdk_gc_set_line_attributes(b->gc, 4, GDK_LINE_SOLID,
                              GDK_CAP_ROUND, GDK_JOIN_ROUND);
   gdk_gc_set_foreground(b->gc, &black);
   gdk_draw_arc(b->pixmap, b->gc, FALSE, middle + BUBBLE_BORDER,
                BUBBLE_BORDER, CORNER_DIAM, CORNER_DIAM, 90*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, FALSE, middle + BUBBLE_BORDER,
                b->height - CORNER_DIAM, CORNER_DIAM,
                CORNER_DIAM, 180*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, FALSE,
                b->width - CORNER_DIAM - BUBBLE_BORDER,
                b->height - CORNER_DIAM, CORNER_DIAM,
                CORNER_DIAM, 270*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, FALSE,
                b->width - CORNER_DIAM - BUBBLE_BORDER,
                BUBBLE_BORDER, CORNER_DIAM, CORNER_DIAM, 0*64, 90*64);

   // Draw the top, bottom, and right sides (easy as they're straight!)
   gdk_draw_line(b->pixmap, b->gc,
                 b->width - BUBBLE_BORDER,
                 CORNER_RADIUS + BUBBLE_BORDER,
                 b->width - BUBBLE_BORDER, b->height - CORNER_RADIUS);
   gdk_draw_line(b->pixmap, b->gc,
                 BUBBLE_BORDER + middle + CORNER_RADIUS, BUBBLE_BORDER,
                 b->width - CORNER_RADIUS, BUBBLE_BORDER);
   gdk_draw_line(b->pixmap, b->gc,
                 BUBBLE_BORDER + middle + CORNER_RADIUS, b->height,
                 b->width - CORNER_RADIUS, b->height);
   
   if (style == NORMAL)
      gdk_draw_lines(b->pixmap, b->gc, tip_points, 5);
   else
      gdk_draw_line(b->pixmap, b->gc,
                    BUBBLE_BORDER + middle,
                    CORNER_RADIUS + BUBBLE_BORDER,
                    BUBBLE_BORDER + middle,
                    b->height - CORNER_RADIUS);
}

static void bubble_size_from_content(bubble_t *b, bubble_style_t style,
                                     int c_width, int c_height)
{
   int middle = style == NORMAL ? TIP_WIDTH : THINK_WIDTH;
   b->width = 2*BUBBLE_BORDER + CORNER_DIAM + middle + c_width;
   b->height = BUBBLE_BORDER + CORNER_DIAM + c_height;
}

static GdkPixbuf *bubble_tidy(bubble_t *b)
{
   GdkPixbuf *pixbuf =
      gdk_pixbuf_get_from_drawable(NULL, b->pixmap, NULL,
                                   0, 0, 0, 0,
                                   b->width + BUBBLE_BORDER,
                                   b->height + BUBBLE_BORDER);
   g_object_unref(b->pixmap);
   return pixbuf;
}

static int bubble_content_left(bubble_style_t style)
{
   int middle = style == NORMAL ? TIP_WIDTH : THINK_WIDTH;
   return BUBBLE_BORDER + middle + CORNER_RADIUS;
}

static int bubble_content_top()
{
   return CORNER_RADIUS;
}

GdkPixbuf *make_dream_bubble(const char *file, int *p_width, int *p_height)
{
   bubble_t bubble;
   GError *error = NULL;
   GdkPixbuf *image = gdk_pixbuf_new_from_file(file, &error);
   
   if (NULL == image) {
      fprintf(stderr, "Error: failed to load %s\n", file);
      exit(1);
   }
   
   bubble_size_from_content(&bubble, THOUGHT, gdk_pixbuf_get_width(image),
                            gdk_pixbuf_get_height(image));
   *p_width = bubble.width;
   *p_height = bubble.height;

   bubble_init(&bubble, THOUGHT);

   gdk_draw_pixbuf(bubble.pixmap, bubble.gc, image, 0, 0,
                   bubble_content_left(THOUGHT), bubble_content_top(),
                   -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
   
   gdk_pixbuf_unref(image);
   
   return bubble_tidy(&bubble);
}

GdkPixbuf *make_text_bubble(char *text, int *p_width, int *p_height, cowmode_t mode)
{
   bubble_t bubble;
   int text_width, text_height;

   // Work out the size of the bubble from the text
   PangoContext *pango_context = gdk_pango_context_get();
   PangoLayout *layout = pango_layout_new(pango_context);
   PangoFontDescription *font =
      pango_font_description_from_string(get_string_option("font"));
   PangoAttrList *pango_attrs = NULL;

   char *stripped;
   if (!pango_parse_markup(text, -1, 0, &pango_attrs, &stripped, NULL, NULL)) {
      fprintf(stderr, i18n("Warning: Failed to parse Pango attributes\n"));
      stripped = text;
   }
   else {
      pango_layout_set_attributes(layout, pango_attrs);
   }
   
   pango_layout_set_font_description(layout, font);
   pango_layout_set_text(layout, stripped, -1);
   pango_layout_get_pixel_size(layout, &text_width, &text_height);

   bubble_style_t style = mode == COWMODE_NORMAL ? NORMAL : THOUGHT;   
   
   bubble_size_from_content(&bubble, style, text_width, text_height);
   *p_width = bubble.width;
   *p_height = bubble.height;
   
   bubble_init(&bubble, style);
   
   // Render the text
   gdk_draw_layout(bubble.pixmap, bubble.gc,
                   bubble_content_left(mode), bubble_content_top(), layout);

   // Make sure to free the Pango objects
   g_object_unref(pango_context);
   g_object_unref(layout);
   pango_font_description_free(font);
   if (NULL != pango_attrs)
      pango_attr_list_unref(pango_attrs);

   return bubble_tidy(&bubble);
}
