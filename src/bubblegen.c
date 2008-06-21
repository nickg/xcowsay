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

#include "floating_shape.h"
#include "settings.h"
#include "i18n.h"

#define LEFT_BUF       5   // Amount of pixels to leave after cow's tail
#define TIP_WIDTH      20  // Length of the triangle bit on the speech bubble
#define CORNER_RADIUS  30  // Radius of corners on the speech bubble
#define CORNER_DIAM    CORNER_RADIUS*2
#define BUBBLE_BORDER  5   // Pixels to leave free around edge of bubble
#define MIN_TIP_HEIGHT 15


GdkPixbuf *make_text_bubble(char *text, int *p_width, int *p_height)
{
   GdkGC *bubble_gc;
   GdkColor black, white, bright_green;
   GdkColormap *colormap;
   GdkPixmap *bubble_pixmap;
   int bubble_width, bubble_height;
   int text_width, text_height;
   GdkPoint tip_points[5];

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
   
   bubble_width = 2*BUBBLE_BORDER + CORNER_DIAM + TIP_WIDTH + text_width;
   bubble_height = BUBBLE_BORDER + CORNER_DIAM + text_height;

   bubble_pixmap = gdk_pixmap_new(NULL, bubble_width, bubble_height, 24);
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
   
   bubble_width -= BUBBLE_BORDER;
   bubble_height -= BUBBLE_BORDER;

   // Draw the white corners
   gdk_gc_set_foreground(bubble_gc, &white);
   gdk_draw_arc(bubble_pixmap, bubble_gc, TRUE, TIP_WIDTH + BUBBLE_BORDER,
                BUBBLE_BORDER, CORNER_DIAM, CORNER_DIAM, 90*64, 90*64);
   gdk_draw_arc(bubble_pixmap, bubble_gc, TRUE, TIP_WIDTH + BUBBLE_BORDER,
                bubble_height - CORNER_DIAM, CORNER_DIAM,
                CORNER_DIAM, 180*64, 90*64);
   gdk_draw_arc(bubble_pixmap, bubble_gc, TRUE,
                bubble_width - CORNER_DIAM - BUBBLE_BORDER,
                bubble_height - CORNER_DIAM, CORNER_DIAM,
                CORNER_DIAM, 270*64, 90*64);
   gdk_draw_arc(bubble_pixmap, bubble_gc, TRUE,
                bubble_width - CORNER_DIAM - BUBBLE_BORDER,
                BUBBLE_BORDER, CORNER_DIAM, CORNER_DIAM, 0*64, 90*64);

   // Fill in the middle of the bubble
   gdk_draw_rectangle(bubble_pixmap, bubble_gc, TRUE,
                      CORNER_RADIUS + TIP_WIDTH + BUBBLE_BORDER,
                      BUBBLE_BORDER,
                      bubble_width - TIP_WIDTH - BUBBLE_BORDER - CORNER_DIAM,
                      bubble_height - BUBBLE_BORDER);
   gdk_draw_rectangle(bubble_pixmap, bubble_gc, TRUE,
                      TIP_WIDTH + BUBBLE_BORDER, BUBBLE_BORDER + CORNER_RADIUS,
                      bubble_width - TIP_WIDTH - BUBBLE_BORDER*2,
                      bubble_height - BUBBLE_BORDER - CORNER_DIAM);

   // The points on the tip part
   int tip_compute_offset = (bubble_height - BUBBLE_BORDER - CORNER_DIAM)/3;
   int tip_offset[3] = { tip_compute_offset, tip_compute_offset, tip_compute_offset };
   if (tip_compute_offset < MIN_TIP_HEIGHT) {
      int new_offset = (bubble_height - BUBBLE_BORDER - CORNER_DIAM - MIN_TIP_HEIGHT)/2;
      tip_offset[0] = new_offset;
      tip_offset[1] = MIN_TIP_HEIGHT;
      tip_offset[2] = new_offset;
   }
   
   tip_points[0].x = TIP_WIDTH + BUBBLE_BORDER;
   tip_points[0].y = BUBBLE_BORDER + CORNER_RADIUS;
   tip_points[1].x = TIP_WIDTH + BUBBLE_BORDER;
   tip_points[1].y = BUBBLE_BORDER + CORNER_RADIUS + tip_offset[0];
   tip_points[2].x = BUBBLE_BORDER;
   tip_points[2].y = BUBBLE_BORDER + CORNER_RADIUS + tip_offset[0] + tip_offset[1]/2;
   tip_points[3].x = TIP_WIDTH + BUBBLE_BORDER;
   tip_points[3].y = BUBBLE_BORDER + CORNER_RADIUS + tip_offset[0] + tip_offset[1];
   tip_points[4].x = TIP_WIDTH + BUBBLE_BORDER;
   tip_points[4].y = bubble_height - CORNER_RADIUS;

   gdk_draw_polygon(bubble_pixmap, bubble_gc, TRUE, tip_points, 5);

   // Draw the black rounded corners
   gdk_gc_set_line_attributes(bubble_gc, 4, GDK_LINE_SOLID,
                              GDK_CAP_ROUND, GDK_JOIN_ROUND);
   gdk_gc_set_foreground(bubble_gc, &black);
   gdk_draw_arc(bubble_pixmap, bubble_gc, FALSE, TIP_WIDTH + BUBBLE_BORDER,
                BUBBLE_BORDER, CORNER_DIAM, CORNER_DIAM, 90*64, 90*64);
   gdk_draw_arc(bubble_pixmap, bubble_gc, FALSE, TIP_WIDTH + BUBBLE_BORDER,
                bubble_height - CORNER_DIAM, CORNER_DIAM,
                CORNER_DIAM, 180*64, 90*64);
   gdk_draw_arc(bubble_pixmap, bubble_gc, FALSE,
                bubble_width - CORNER_DIAM - BUBBLE_BORDER,
                bubble_height - CORNER_DIAM, CORNER_DIAM,
                CORNER_DIAM, 270*64, 90*64);
   gdk_draw_arc(bubble_pixmap, bubble_gc, FALSE,
                bubble_width - CORNER_DIAM - BUBBLE_BORDER,
                BUBBLE_BORDER, CORNER_DIAM, CORNER_DIAM, 0*64, 90*64);
   
   gdk_draw_lines(bubble_pixmap, bubble_gc, tip_points, 5);

   // Draw the top, bottom, and right sides (easy as they're straight!)
   gdk_draw_line(bubble_pixmap, bubble_gc,
                 bubble_width - BUBBLE_BORDER,
                 CORNER_RADIUS + BUBBLE_BORDER,
                 bubble_width - BUBBLE_BORDER, bubble_height - CORNER_RADIUS);
   gdk_draw_line(bubble_pixmap, bubble_gc,
                 BUBBLE_BORDER + TIP_WIDTH + CORNER_RADIUS, BUBBLE_BORDER,
                 bubble_width - CORNER_RADIUS, BUBBLE_BORDER);
   gdk_draw_line(bubble_pixmap, bubble_gc,
                 BUBBLE_BORDER + TIP_WIDTH + CORNER_RADIUS, bubble_height,
                 bubble_width - CORNER_RADIUS, bubble_height);
   
   *p_width = bubble_width + BUBBLE_BORDER;
   *p_height = bubble_height + BUBBLE_BORDER;

   // Render the text
   gdk_draw_layout(bubble_pixmap, bubble_gc,
                   BUBBLE_BORDER + TIP_WIDTH + CORNER_RADIUS,
                   CORNER_RADIUS, layout);

   // Make sure to free the Pango objects
   g_object_unref(pango_context);
   g_object_unref(layout);
   pango_font_description_free(font);
   if (NULL != pango_attrs)
      pango_attr_list_unref(pango_attrs);

   GdkPixbuf *pixbuf =
      gdk_pixbuf_get_from_drawable(NULL, bubble_pixmap, NULL,
                                   0, 0, 0, 0,
                                   bubble_width + BUBBLE_BORDER,
                                   bubble_height + BUBBLE_BORDER);
   g_object_unref(bubble_pixmap);
   return pixbuf;
}
