/*  xcowsayd.c -- DBus xcowsay daemon.
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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "xcowsayd.h"

#ifdef WITH_DBUS

#include <dbus/dbus-glib-bindings.h>

#include "display_cow.h"

typedef struct {
   GObject parent;
   DBusGConnection *connection;
} Cowsay;

typedef struct {
   GObjectClass parent_class;
} CowsayClass;


static void cowsayd_init(Cowsay *server);
static void cowsayd_class_init(CowsayClass *class);

static gboolean cowsay_show_cow(Cowsay *obj, const gchar *mess, GError **error);
static gboolean cowsay_think(Cowsay *obj, const gchar *mess, GError **error);
static gboolean cowsay_dream(Cowsay *obj, const gchar *file, GError **error);

G_DEFINE_TYPE(Cowsay, cowsayd, G_TYPE_OBJECT);

#include "Cowsay_glue.h"

typedef struct _cowsay_queue_t {
   struct _cowsay_queue_t *next;
   char *message;
   cowmode_t mode;
} cowsay_queue_t;

static cowsay_queue_t *requests = NULL;
static GMutex *queue_lock = NULL;
static GCond *request_ready = NULL;
static GMutex *display_lock = NULL;
static GCond *display_complete = NULL;

#define QUEUE_MUTEX_LOCK g_mutex_lock(queue_lock)
#define QUEUE_MUTEX_UNLOCK g_mutex_unlock(queue_lock)
#define REQUEST_READY_WAIT g_cond_wait(request_ready, queue_lock)
#define REQUEST_READY_SIGNAL g_cond_signal(request_ready)

static void enqueue_request(const char *mess, cowmode_t mode)
{
   cowsay_queue_t *req = (cowsay_queue_t*)malloc(sizeof(cowsay_queue_t));
   req->next = NULL;
   req->message = malloc(strlen(mess)+1);
   req->mode = mode;
   g_assert(req->message);
   strcpy(req->message, mess);

   // Append the request to the end of the queue
   QUEUE_MUTEX_LOCK;
   {
      if (NULL == requests) {
         requests = req;
      }
      else {
         cowsay_queue_t *it;
         for (it = requests; it->next; it = it->next);
         it->next = req;
      }
      REQUEST_READY_SIGNAL;
   }
   QUEUE_MUTEX_UNLOCK;
}

static void wait_for_request(const char** mess, cowmode_t *mode)
{
   QUEUE_MUTEX_LOCK;
   {
      while (NULL == requests) {
         REQUEST_READY_WAIT;
      }
      *mess = requests->message;
      *mode = requests->mode;
   }
   QUEUE_MUTEX_UNLOCK;
}

static void request_complete()
{
   QUEUE_MUTEX_LOCK;
   {
      cowsay_queue_t *req = requests;
      g_assert(req);
      requests = req->next;
      free(req->message);
      free(req);
   }
   QUEUE_MUTEX_UNLOCK;
}

static gpointer cow_display_thread(gpointer data)
{
   bool debug = *(bool*)data;

   debug_msg("In the cow display thread\n");

   g_mutex_lock(display_lock);

   for (;;) {
      const char *mess;
      cowmode_t mode;
      wait_for_request(&mess, &mode);
      debug_msg("Processing request: %s\n", mess);

      // We need to wrap the GTK+ stuff in gdk_threads_X since
      // GTK assumes it is being called from the main thread
      // (and it isn't here)
      gdk_threads_enter();
      display_cow(debug, mess, false, mode);
      gdk_threads_leave();

      g_cond_wait(display_complete, display_lock);

      request_complete();
   }

   g_mutex_unlock(display_lock);

   return NULL;
}

static void cowsayd_class_init(CowsayClass *class)
{
   // Nothing to do here
}

static void cowsayd_init(Cowsay *server)
{
   GError *error = NULL;
   DBusGProxy *driver_proxy;
   guint request_ret;

   // Initialise the DBus connection
   server->connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
   if (NULL == server->connection) {
      g_warning("Unable to connect to DBus: %s", error->message);
      g_error_free(error);
      exit(EXIT_FAILURE);
   }

   dbus_g_object_type_install_info(cowsayd_get_type(),
                                   &dbus_glib_cowsay_object_info);

   // Register DBus path
   dbus_g_connection_register_g_object(server->connection,
                                       "/uk/me/doof/Cowsay",
                                       G_OBJECT(server));

   // Register the service name
   driver_proxy = dbus_g_proxy_new_for_name(server->connection,
                                            DBUS_SERVICE_DBUS,
                                            DBUS_PATH_DBUS,
                                            DBUS_INTERFACE_DBUS);
   if (!org_freedesktop_DBus_request_name(driver_proxy,
                                          "uk.me.doof.Cowsay",
                                          0, &request_ret, &error)) {
      g_warning("Unable to register service: %s", error->message);
      g_error_free(error);
      exit(EXIT_FAILURE);
   }

   g_object_unref(driver_proxy);

}

static gboolean cowsay_show_cow(Cowsay *obj, const gchar *mess, GError **error)
{
   printf("cowsay_show_cow mess=%s\n", mess);
   enqueue_request(mess, COWMODE_NORMAL);
   return true;
}

static gboolean cowsay_think(Cowsay *obj, const gchar *mess, GError **error)
{
   printf("cowsay_think mess=%s\n", mess);
   enqueue_request(mess, COWMODE_THINK);
   return true;
}

static gboolean cowsay_dream(Cowsay *obj, const gchar *file, GError **error)
{
   printf("cowsay_dream file=%s\n", file);
   enqueue_request(file, COWMODE_DREAM);
   return true;
}

void run_cowsay_daemon(bool debug, int argc, char **argv)
{
   if (!debug) {
      // Fork away from the terminal
      int pid = fork();
      if (pid < 0) {
         perror("fork");
         exit(EXIT_FAILURE);
      }
      else if (pid > 0) {
         exit(EXIT_SUCCESS);
      }

      // Detach from the terminal
      if (setsid() == -1) {
         perror("setsid");
         exit(EXIT_FAILURE);
      }

      // Close existing handles
      int fd;
      for (fd = 0; fd < getdtablesize(); fd++)
         close(fd);

      // Redirect everything to /dev/null
      fd = open("/dev/null", O_RDWR);  // fd = stdin
      if (-1 == fd)
         abort();

      dup(fd);   // stdout
      dup(fd);   // stderr
   }

   // g_thread_init must come before all other GLib calls
   g_thread_init(NULL);
   gdk_threads_init();

   queue_lock = g_mutex_new();
   request_ready = g_cond_new();
   display_complete = g_cond_new();
   display_lock = g_mutex_new();

   cowsay_init(&argc, &argv);

   g_type_init();
   Cowsay *server = g_object_new(cowsayd_get_type(), NULL);

   GThread *displ = g_thread_create(cow_display_thread, (gpointer)&debug, FALSE, NULL);
   g_assert(displ);

   debug_msg("Cowsay daemon starting...\n");
   for (;;) {
      gtk_main();
      g_cond_signal(display_complete);
   }

   g_object_unref(server);

   exit(EXIT_SUCCESS);
}

#else /* #ifdef WITH_DBUS */

void run_cowsay_daemon(bool debug, int argc, char **argv)
{
   fprintf(stderr, "Error: Daemon mode unavailable as xcowsay was compiled "
           "without DBus support.\n");
   exit(EXIT_FAILURE);
}

#endif /* #ifdef WITH_DBUS */
