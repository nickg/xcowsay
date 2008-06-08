#include <string.h>
#include <stdio.h>

#include <panel-applet.h>
#include <gtk/gtklabel.h>

#include "display_cow.h"

static gboolean on_button_press(GtkWidget *applet,
                                GdkEventButton *event,
                                gpointer data)
{
   // Don't react to anything other than a left click
   if (event->button != 1)
      return FALSE;
   
   return TRUE;
}

static gboolean xcowsay_applet_fill(PanelApplet *applet,
                                    const gchar *iid,
                                    gpointer data)
{
   GtkWidget *image;

   printf("Here\n");
   
   if (strcmp(iid, "OAFIID:CowsayApplet") != 0)
      return FALSE;

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
