/*  applet.c -- Gnome panel applet.
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

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <X11/Xlib.h>
#include <panel-applet.h>
#include <gtk/gtklabel.h>
#include <gdk/gdkx.h>

#include "display_cow.h"
#include "settings.h"
#include "xcowsayd.h"
#include "config_file.h"
#include "i18n.h"

// Default settings
#define DEF_LEAD_IN_TIME  250
#define DEF_DISPLAY_TIME  CALCULATE_DISPLAY_TIME
#define DEF_LEAD_OUT_TIME LEAD_IN_TIME
#define DEF_MIN_TIME      2000
#define DEF_MAX_TIME      30000
#define DEF_FONT          "Bitstream Vera Sans 14"
#define DEF_READING_SPEED 250   // Human average is apparently 200-250 WPM (=5 WPS)
#define DEF_COW_SIZE      "med"
#define DEF_IMAGE_BASE    "cow"

#define MAX_STDIN 4096   // Maximum chars to read from stdin


static pid_t daemon_pid = 0;

static void restart_daemon(void)
{
   daemon_pid = fork();
   if (daemon_pid == 0) {
      close(ConnectionNumber(gdk_display));
      
      execl("/usr/bin/env", "/usr/bin/env", "xcowsay",
            "--daemon", "--debug", NULL);
      perror("execl");
      abort();
   }
   else if (daemon_pid == -1) {
      perror("fork");
      abort();
   }
}

static gboolean on_button_press(GtkWidget *applet,
                                GdkEventButton *event,
                                gpointer data)
{
   int status;
   char *buf;
   FILE *pout;
   
   // Don't react to anything other than a left click
   if (event->button != 1)
      return FALSE;

   // Check daemon is still alive
   pid_t rc = waitpid(daemon_pid, &status, WNOHANG);
   if (rc == 0) {
      // Daemon is still alive
      printf("Daemon still alive\n");
   }
   else if (rc == daemon_pid) {
      // Daemon has terminated
      printf("Daemon terminated - restarting\n");
      restart_daemon();
   }
   else if (rc == -1) {
      perror("waitpid");
      abort();
   }

   buf = calloc(MAX_STDIN, 1);
   assert(buf);
   pout = popen("fortune", "r");
   if (NULL == pout)
      strcpy(buf, "Error: Failed to run fortune program!");
   else {
      fread(buf, 1, MAX_STDIN, pout);
      fclose(pout);
   }

   display_cow_or_invoke_daemon(false, buf);

   free(buf);
   
   return TRUE;
}

static void load_defaults(void)
{
   add_int_option("lead_in_time", DEF_LEAD_IN_TIME);
   add_int_option("display_time", DEF_DISPLAY_TIME);
   add_int_option("lead_out_time", get_int_option("lead_in_time"));
   add_int_option("min_display_time", DEF_MIN_TIME);
   add_int_option("max_display_time", DEF_MAX_TIME);
   add_int_option("reading_speed", DEF_READING_SPEED);
   add_string_option("font", DEF_FONT);
   add_string_option("cow_size", DEF_COW_SIZE);
   add_string_option("image_base", DEF_IMAGE_BASE);
   
   parse_config_file();
}

static gboolean xcowsay_applet_fill(PanelApplet *applet,
                                    const gchar *iid,
                                    gpointer data)
{
   GtkWidget *image;
   GdkWindow *win;
   
   if (strcmp(iid, "OAFIID:CowsayApplet") != 0)
      return FALSE;

   load_defaults();

   restart_daemon();

   image = gtk_image_new_from_file(DATADIR "/cow_panel_icon.png");
   g_assert(image);
   
   gtk_container_add(GTK_CONTAINER(applet), image);

   g_signal_connect(G_OBJECT(applet),
                    "button_press_event",
                    G_CALLBACK(on_button_press),
                    image);

   gtk_widget_show_all(GTK_WIDGET(applet));

   gdk_window_set_back_pixmap(GTK_WIDGET(applet)->window, NULL, TRUE);
   
   return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY("OAFIID:CowsayApplet_Factory",
                            PANEL_TYPE_APPLET,
                            "The Hello World Applet",
                            "0",
                            xcowsay_applet_fill,
                            NULL);
