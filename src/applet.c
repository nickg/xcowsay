#include "display_cow.h"

#include <string.h>

#include <panel-applet.h>
#include <gtk/gtklabel.h>

static gboolean myexample_applet_fill(PanelApplet *applet,
                                      const gchar *iid,
                                      gpointer data)
{
   GtkWidget *label;
   
   if (strcmp (iid, "OAFIID:ExampleApplet") != 0)
      return FALSE;
   
   label = gtk_label_new ("Hello World");
   gtk_container_add (GTK_CONTAINER (applet), label);
   
   gtk_widget_show_all (GTK_WIDGET (applet));
   
   return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:ExampleApplet_Factory",
                             PANEL_TYPE_APPLET,
                             "The Hello World Applet",
                             "0",
                             myexample_applet_fill,
                             NULL);
