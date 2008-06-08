#include <string.h>
#include <stdio.h>

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


static gboolean on_button_press(GtkWidget *applet,
                                GdkEventButton *event,
                                gpointer data)
{
   // Don't react to anything other than a left click
   if (event->button != 1)
      return FALSE;

   printf("Moo!\n");
   system("/usr/bin/env xcowfortune &");
   
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
   
   if (strcmp(iid, "OAFIID:CowsayApplet") != 0)
      return FALSE;

   load_defaults();

   if (fork() == 0) {
      close(ConnectionNumber(gdk_display));
      
      execl("/usr/bin/env", "/usr/bin/env", "xcowsay",
            "--daemon", "--debug", NULL);
      perror("execl");
      abort();
   }

   image = gtk_image_new_from_file(DATADIR "/cow_panel_icon.png");
   g_assert(image);
   
   gtk_container_add(GTK_CONTAINER(applet), image);

   g_signal_connect(G_OBJECT(applet),
                    "button_press_event",
                    G_CALLBACK(on_button_press),
                    image);
   
   gtk_widget_show_all(GTK_WIDGET(applet));

   
   
   return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY("OAFIID:CowsayApplet_Factory",
                            PANEL_TYPE_APPLET,
                            "The Hello World Applet",
                            "0",
                            xcowsay_applet_fill,
                            NULL);
