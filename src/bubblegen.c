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
#include <math.h>

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
#define BIG_KIRCLE_RADIUS (BIG_KIRCLE_DIAM / 2)

#define SMALL_KIRCLE_X      5
#define SMALL_KIRCLE_Y      40
#define SMALL_KIRCLE_DIAM   20
#define SMALL_KIRCLE_RADIUS (SMALL_KIRCLE_DIAM / 2)

// Min distance from top of the big kircle to the top of the bubble
#define KIRCLE_TOP_MIN  10

typedef struct {
   int width, height;
   cairo_surface_t *surface;
   cairo_t *cr;
} bubble_t;

typedef enum { NORMAL, THOUGHT } bubble_style_t;

static void bubble_corner_arcs(bubble_t *b, bubble_style_t style,
                               int corners[4][2])
{
   // Space between cow and bubble
   int middle = (style == NORMAL ? TIP_WIDTH : THINK_WIDTH);

   if (get_bool_option("left")) {
      corners[0][0] = BUBBLE_BORDER + CORNER_RADIUS;
      corners[0][1] = BUBBLE_BORDER + CORNER_RADIUS;

      corners[3][0] = BUBBLE_BORDER + CORNER_RADIUS;
      corners[3][1] = b->height - CORNER_DIAM + CORNER_RADIUS;

      corners[2][0] = b->width - CORNER_DIAM - BUBBLE_BORDER - middle + CORNER_RADIUS;
      corners[2][1] = b->height - CORNER_DIAM + CORNER_RADIUS;

      corners[1][0] = b->width - CORNER_DIAM - BUBBLE_BORDER - middle + CORNER_RADIUS;
      corners[1][1] = BUBBLE_BORDER + CORNER_RADIUS;
   }
   else {
      corners[0][0] = middle + BUBBLE_BORDER + CORNER_RADIUS;
      corners[0][1] = BUBBLE_BORDER + CORNER_RADIUS;

      corners[3][0] = middle + BUBBLE_BORDER + CORNER_RADIUS;
      corners[3][1] = b->height - CORNER_DIAM + CORNER_RADIUS;

      corners[2][0] = b->width - CORNER_DIAM - BUBBLE_BORDER + CORNER_RADIUS;
      corners[2][1] = b->height - CORNER_DIAM + CORNER_RADIUS;

      corners[1][0] = b->width - CORNER_DIAM - BUBBLE_BORDER + CORNER_RADIUS;
      corners[1][1] = BUBBLE_BORDER + CORNER_RADIUS;
   }
}

static void bubble_init_cairo(bubble_t *b, cairo_t *cr, bubble_style_t style)
{
   GdkPoint tip_points[5];
   bool right = !get_bool_option("left");

   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
   cairo_rectangle(cr, 0, 0, b->width, b->height);
   cairo_fill(cr);

   b->width -= BUBBLE_BORDER;
   b->height -= BUBBLE_BORDER;

   // Space between cow and bubble
   int middle = style == NORMAL ? TIP_WIDTH : THINK_WIDTH;

   // Draw the white corners
   int corners[4][2];
   bubble_corner_arcs(b, style, corners);

   cairo_set_line_width(cr, 4.0);
   cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

   for (int i = 0; i < 4; i++) {
      cairo_move_to(cr, corners[i][0], corners[i][1]);
      cairo_arc(cr, corners[i][0], corners[i][1],
                CORNER_RADIUS,
                M_PI + i * (M_PI / 2.0),
                M_PI + (i+1) * (M_PI / 2.0));
      cairo_close_path(cr);
      cairo_fill(cr);
   }

   // Fill in the middle of the bubble
   cairo_rectangle(cr,
                   CORNER_RADIUS + (right ? middle : 0) + BUBBLE_BORDER,
                   BUBBLE_BORDER,
                   b->width - middle - BUBBLE_BORDER - CORNER_DIAM,
                   b->height - BUBBLE_BORDER);
   cairo_rectangle(cr,
                   (right ? middle : 0) + BUBBLE_BORDER,
                   BUBBLE_BORDER + CORNER_RADIUS,
                   b->width - middle - BUBBLE_BORDER*2,
                   b->height - BUBBLE_BORDER - CORNER_DIAM);
   cairo_fill(cr);

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

      cairo_move_to(cr, tip_points[0].x, tip_points[0].y);
      for (int i = 1; i < 5; i++) {
         cairo_line_to(cr, tip_points[i].x, tip_points[i].y);
      }

      cairo_close_path(cr);
      cairo_fill(cr);
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
      cairo_arc(cr,
                (right ? BIG_KIRCLE_X + BIG_KIRCLE_RADIUS
                 : b->width - BIG_KIRCLE_X - BIG_KIRCLE_RADIUS),
                b->height/2 - big_y + BIG_KIRCLE_RADIUS, BIG_KIRCLE_RADIUS,
                0, 2.0 * M_PI);

      cairo_arc(cr,
                (right ? SMALL_KIRCLE_X + SMALL_KIRCLE_RADIUS
                 : b->width - SMALL_KIRCLE_X - SMALL_KIRCLE_RADIUS),
                b->height/2 - small_y + SMALL_KIRCLE_RADIUS, SMALL_KIRCLE_RADIUS,
                0, 2.0 * M_PI);

      cairo_fill(cr);

      cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);

      cairo_arc(cr,
                (right ? BIG_KIRCLE_X + BIG_KIRCLE_RADIUS
                 : b->width - BIG_KIRCLE_X - BIG_KIRCLE_RADIUS),
                b->height/2 - big_y + BIG_KIRCLE_RADIUS, BIG_KIRCLE_RADIUS,
                0, 2.0 * M_PI);

      cairo_stroke(cr);

      cairo_arc(cr,
                (right ? SMALL_KIRCLE_X + SMALL_KIRCLE_RADIUS
                 : b->width - SMALL_KIRCLE_X - SMALL_KIRCLE_RADIUS),
                b->height/2 - small_y + SMALL_KIRCLE_RADIUS, SMALL_KIRCLE_RADIUS,
                0, 2.0 * M_PI);

      cairo_stroke(cr);
   }

   // Draw the black rounded corners
   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);

   // Top left
   cairo_arc(cr,
             (right ? middle : 0) + BUBBLE_BORDER + CORNER_RADIUS,
             BUBBLE_BORDER + CORNER_RADIUS,
             CORNER_RADIUS,
             M_PI, M_PI + (M_PI / 2.0));

   // Top right
   cairo_arc(cr,
             b->width - (!right ? middle : 0) - CORNER_RADIUS - BUBBLE_BORDER,
             BUBBLE_BORDER + CORNER_RADIUS,
             CORNER_RADIUS,
             M_PI + (M_PI / 2.0), 2 * M_PI);

   if (style == NORMAL && !right) {
      cairo_move_to(cr, tip_points[0].x, tip_points[0].y);
      for (int i = 1; i < 5; i++) {
         cairo_line_to(cr, tip_points[i].x, tip_points[i].y);
      }
   }

   // Bottom left
   cairo_arc(cr,
             b->width - (!right ? middle : 0) - CORNER_RADIUS - BUBBLE_BORDER,
             b->height - CORNER_RADIUS,
             CORNER_RADIUS,
             0.0, (M_PI / 2.0));

   cairo_arc(cr,
             (right ? middle : 0) + BUBBLE_BORDER + CORNER_RADIUS,
             b->height - CORNER_RADIUS,
             CORNER_RADIUS,
             M_PI / 2.0, M_PI);

   if (style == NORMAL && right) {
      cairo_move_to(cr, tip_points[0].x, tip_points[0].y);
      for (int i = 1; i < 5; i++) {
         cairo_line_to(cr, tip_points[i].x, tip_points[i].y);
      }
   }
   else {
      cairo_line_to(cr, (right ? middle : 0) + BUBBLE_BORDER,
                    BUBBLE_BORDER + CORNER_RADIUS);
   }

   cairo_stroke(cr);
}

static void bubble_init(bubble_t *b, bubble_style_t style)
{
   b->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                           b->width, b->height);
   g_assert(b->surface);

   b->cr = cairo_create(b->surface);

   bubble_init_cairo(b, b->cr, style);
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
      gdk_pixbuf_get_from_surface(b->surface, 0, 0, b->width + BUBBLE_BORDER, b->height + BUBBLE_BORDER);

   cairo_surface_destroy(b->surface);
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

   gdk_cairo_set_source_pixbuf(bubble.cr, image,
                               bubble_content_left(THOUGHT),
                               bubble_content_top());
   cairo_paint(bubble.cr);

   cairo_destroy(bubble.cr);
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
   cairo_move_to(bubble.cr, bubble_content_left(style), bubble_content_top());
   pango_cairo_show_layout(bubble.cr, layout);

   cairo_destroy(bubble.cr);

   // Make sure to free the Pango objects
   g_object_unref(pango_context);
   g_object_unref(layout);
   pango_font_description_free(font);
   if (NULL != pango_attrs)
      pango_attr_list_unref(pango_attrs);

   return bubble_tidy(&bubble);
}
