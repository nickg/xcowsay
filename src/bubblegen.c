/*  bubblegen.c -- Generate various sorts of bubbles.
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

static void get_colour(guint16 r, guint16 g, guint16 b, GdkColor *c)
{
   GdkColormap *colormap;
   gboolean ok;

   colormap = gdk_colormap_get_system();

   c->red = r;
   c->green = g;
   c->blue = b;

   ok = gdk_colormap_alloc_color(colormap, c, FALSE, TRUE);
   g_assert(ok);
}

static void bubble_corner_arcs(bubble_t *b, bubble_style_t style,
                               int corners[4][2])
{
   // Space between cow and bubble
   int middle = (style == NORMAL ? TIP_WIDTH : THINK_WIDTH);

   if (get_bool_option("left")) {
      corners[0][0] = BUBBLE_BORDER;
      corners[0][1] = BUBBLE_BORDER;

      corners[1][0] = BUBBLE_BORDER;
      corners[1][1] = b->height - CORNER_DIAM;

      corners[2][0] = b->width - CORNER_DIAM - BUBBLE_BORDER - middle;
      corners[2][1] = b->height - CORNER_DIAM;

      corners[3][0] = b->width - CORNER_DIAM - BUBBLE_BORDER - middle;
      corners[3][1] = BUBBLE_BORDER;
   }
   else {
      corners[0][0] = middle + BUBBLE_BORDER;
      corners[0][1] = BUBBLE_BORDER;

      corners[1][0] = middle + BUBBLE_BORDER;
      corners[1][1] = b->height - CORNER_DIAM;

      corners[2][0] = b->width - CORNER_DIAM - BUBBLE_BORDER;
      corners[2][1] = b->height - CORNER_DIAM;

      corners[3][0] = b->width - CORNER_DIAM - BUBBLE_BORDER;
      corners[3][1] = BUBBLE_BORDER;
   }
}

static void bubble_init_shared(bubble_t *b, bubble_style_t style, bool right)
{
   GdkColor black, white, bright_green;
   GdkPoint tip_points[5];

   get_colour(0, 0, 0, &black);
   get_colour(0xffff, 0xffff, 0xffff, &white);
   get_colour(0, 0xffff, 0, &bright_green);

   gdk_gc_set_background(b->gc, &black);
   gdk_gc_set_rgb_fg_color(b->gc, &bright_green);

   gdk_draw_rectangle(b->pixmap, b->gc, TRUE, 0, 0, b->width, b->height);

   b->width -= BUBBLE_BORDER;
   b->height -= BUBBLE_BORDER;

   // Space between cow and bubble
   int middle = style == NORMAL ? TIP_WIDTH : THINK_WIDTH;

   // Draw the white corners
   int corners[4][2];
   bubble_corner_arcs(b, style, corners);

   gdk_gc_set_foreground(b->gc, &white);
   gdk_draw_arc(b->pixmap, b->gc, TRUE, corners[0][0], corners[0][1],
                CORNER_DIAM, CORNER_DIAM, 90*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, TRUE, corners[1][0], corners[1][1],
                CORNER_DIAM, CORNER_DIAM, 180*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, TRUE, corners[2][0], corners[2][1],
                CORNER_DIAM, CORNER_DIAM, 270*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, TRUE, corners[3][0], corners[3][1],
                CORNER_DIAM, CORNER_DIAM, 0*64, 90*64);

   // Fill in the middle of the bubble
   gdk_draw_rectangle(b->pixmap, b->gc, TRUE,
                      CORNER_RADIUS + (right ? middle : 0) + BUBBLE_BORDER,
                      BUBBLE_BORDER,
                      b->width - middle - BUBBLE_BORDER - CORNER_DIAM,
                      b->height - BUBBLE_BORDER);
   gdk_draw_rectangle(b->pixmap, b->gc, TRUE,
                      (right ? middle : 0) + BUBBLE_BORDER,
                      BUBBLE_BORDER + CORNER_RADIUS,
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

      if (right) {
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
      }
      else {
         tip_points[0].x = b->width - middle - BUBBLE_BORDER;
         tip_points[0].y = BUBBLE_BORDER + CORNER_RADIUS;
         tip_points[1].x = b->width - middle - BUBBLE_BORDER;
         tip_points[1].y = BUBBLE_BORDER + CORNER_RADIUS + tip_offset[0];
         tip_points[2].x = b->width - BUBBLE_BORDER;
         tip_points[2].y = BUBBLE_BORDER + CORNER_RADIUS + tip_offset[0] + tip_offset[1]/2;
         tip_points[3].x = b->width - middle - BUBBLE_BORDER;
         tip_points[3].y = BUBBLE_BORDER + CORNER_RADIUS + tip_offset[0] + tip_offset[1];
         tip_points[4].x = b->width - middle - BUBBLE_BORDER;
         tip_points[4].y = b->height - CORNER_RADIUS;
      }

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
                   right ? BIG_KIRCLE_X : b->width - BIG_KIRCLE_X - BIG_KIRCLE_DIAM,
                   b->height/2 - big_y, BIG_KIRCLE_DIAM,
                   BIG_KIRCLE_DIAM, 0, 360*64);

      gdk_draw_arc(b->pixmap, b->gc, TRUE,
                   right ? SMALL_KIRCLE_X : b->width - SMALL_KIRCLE_X - SMALL_KIRCLE_DIAM,
                   b->height/2 - small_y, SMALL_KIRCLE_DIAM,
                   SMALL_KIRCLE_DIAM, 0, 360*64);

      gdk_gc_set_line_attributes(b->gc, 4, GDK_LINE_SOLID,
                                 GDK_CAP_ROUND, GDK_JOIN_ROUND);
      gdk_gc_set_foreground(b->gc, &black);
      gdk_draw_arc(b->pixmap, b->gc, FALSE,
                   right ? BIG_KIRCLE_X : b->width - BIG_KIRCLE_X - BIG_KIRCLE_DIAM,
                   b->height/2 - big_y, BIG_KIRCLE_DIAM,
                   BIG_KIRCLE_DIAM, 0, 360*64);

      gdk_draw_arc(b->pixmap, b->gc, FALSE,
                   right ? SMALL_KIRCLE_X : b->width - SMALL_KIRCLE_X - SMALL_KIRCLE_DIAM,
                   b->height/2 - small_y, SMALL_KIRCLE_DIAM,
                   SMALL_KIRCLE_DIAM, 0, 360*64);
   }

   // Draw the black rounded corners
   gdk_gc_set_line_attributes(b->gc, 4, GDK_LINE_SOLID,
                              GDK_CAP_ROUND, GDK_JOIN_ROUND);
   gdk_gc_set_foreground(b->gc, &black);
   gdk_draw_arc(b->pixmap, b->gc, FALSE, (right ? middle : 0) + BUBBLE_BORDER,
                BUBBLE_BORDER, CORNER_DIAM, CORNER_DIAM, 90*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, FALSE, (right ? middle : 0) + BUBBLE_BORDER,
                b->height - CORNER_DIAM, CORNER_DIAM,
                CORNER_DIAM, 180*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, FALSE,
                b->width - (!right ? middle : 0) - CORNER_DIAM - BUBBLE_BORDER,
                b->height - CORNER_DIAM, CORNER_DIAM,
                CORNER_DIAM, 270*64, 90*64);
   gdk_draw_arc(b->pixmap, b->gc, FALSE,
                b->width - (!right ? middle : 0) - CORNER_DIAM - BUBBLE_BORDER,
                BUBBLE_BORDER, CORNER_DIAM, CORNER_DIAM, 0*64, 90*64);

   // Draw the top, bottom, and right sides (easy as they're straight!)
   gdk_draw_line(b->pixmap, b->gc,
                 right ? b->width - BUBBLE_BORDER : BUBBLE_BORDER,
                 CORNER_RADIUS + BUBBLE_BORDER,
                 right ? b->width - BUBBLE_BORDER : BUBBLE_BORDER,
                 b->height - CORNER_RADIUS);
   gdk_draw_line(b->pixmap, b->gc,
                 BUBBLE_BORDER + (right ? middle : 0) + CORNER_RADIUS ,
                 BUBBLE_BORDER,
                 b->width - CORNER_RADIUS - (!right ? middle : 0),
                 BUBBLE_BORDER);
   gdk_draw_line(b->pixmap, b->gc,
                 BUBBLE_BORDER + (right ? middle : 0) + CORNER_RADIUS,
                 b->height,
                 b->width - CORNER_RADIUS - (!right ? middle : 0),
                 b->height);

   if (style == NORMAL)
      gdk_draw_lines(b->pixmap, b->gc, tip_points, 5);
   else
      gdk_draw_line(b->pixmap, b->gc,
                    BUBBLE_BORDER + middle,
                    CORNER_RADIUS + BUBBLE_BORDER,
                    BUBBLE_BORDER + middle,
                    b->height - CORNER_RADIUS);
}

static void bubble_init(bubble_t *b, bubble_style_t style)
{
   GdkVisual *root_visual;

   root_visual = gdk_visual_get_system();
   b->pixmap = gdk_pixmap_new(NULL, b->width, b->height,
                              gdk_visual_get_depth(root_visual));
   g_assert(b->pixmap);
   b->gc = gdk_gc_new(b->pixmap);

   bubble_init_shared(b, style, !get_bool_option("left"));
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
   if (get_bool_option("left")) {
      return BUBBLE_BORDER + CORNER_RADIUS;
   }
   else {
      const int middle = style == NORMAL ? TIP_WIDTH : THINK_WIDTH;
      return BUBBLE_BORDER + middle + CORNER_RADIUS;
   }
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

   g_object_unref(image);

   return bubble_tidy(&bubble);
}

GdkPixbuf *make_text_bubble(char *text, int *p_width, int *p_height,
                            int max_width, cowmode_t mode)
{
   bubble_t bubble;
   int text_width, text_height;

   // Work out the size of the bubble from the text
   PangoContext *pango_context = gdk_pango_context_get();
   PangoLayout *layout = pango_layout_new(pango_context);
   PangoFontDescription *font =
      pango_font_description_from_string(get_string_option("font"));
   PangoAttrList *pango_attrs = NULL;

   // Adjust max width to account for bubble edges
   max_width -= LEFT_BUF;
   max_width -= TIP_WIDTH;
   max_width -= 2 * BUBBLE_BORDER;
   max_width -= CORNER_DIAM;

   if (get_bool_option("wrap")) {
      pango_layout_set_width(layout, max_width * PANGO_SCALE);
      pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
   }

   char *stripped;
   if (!pango_parse_markup(text, -1, 0, &pango_attrs,
         &stripped, NULL, NULL)) {

      // This isn't fatal as the the text may contain angled brackets, etc.
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
