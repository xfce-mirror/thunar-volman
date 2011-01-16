/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2010 Jannis Pohlmann <jannis@xfce.org>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free 
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>

#include <gtk/gtk.h>

#include <gudev/gudev.h>

#ifdef HAVE_LIBNOTIFY
#include <thunar-volman/tvm-notify.h>
#endif

#include <libxfce4util/libxfce4util.h>

#include <xfconf/xfconf.h>

#include <thunar-volman/tvm-context.h>
#include <thunar-volman/tvm-device.h>



/* variables for command line options */
static gchar   *opt_sysfs_path = NULL;
static gboolean opt_configure = FALSE;
static gboolean opt_version = FALSE;



/* command line options */
static GOptionEntry option_entries[] =
{
  { "device-added", 'a', 0, G_OPTION_ARG_STRING, &opt_sysfs_path, N_ ("The syfs path of the newly added device"), NULL, },
  { "configure", 'c', 0, G_OPTION_ARG_NONE, &opt_configure, N_ ("Configure management of removable drives and media"), NULL, },
  { "version", 'V', 0, G_OPTION_ARG_NONE, &opt_version, N_ ("Print version information and exit"), NULL, },
  { NULL, },
};



/* udev subsystems supported by thunar-volman */
static const gchar *supported_udev_subsystems[] =
{
  "block",
  "input",
  NULL,
};



int
main (int    argc,
      char **argv)
{
  XfconfChannel *channel;
  GUdevClient   *client;
  GUdevDevice   *device;
  TvmContext    *context = NULL;
  GMainLoop     *loop = NULL;
  GError        *error = NULL;
  gint           exit_code = EXIT_SUCCESS;

  /* setup translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* setup application name */
  g_set_application_name (_("Thunar Volume Manager"));

#ifdef G_ENABLE_DEBUG
  /* Do NOT remove this line for now. If something doesn't work, fix your code instead */
  g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);
#endif

  /* initialize the threading system */
  if (!g_thread_supported ())
    g_thread_init (NULL);

  /* initialize GTK+ */
  if (!gtk_init_with_args (&argc, &argv, NULL, option_entries, GETTEXT_PACKAGE, &error))
    {
      g_fprintf (stderr, "%s: %s.\n", g_get_prgname (), error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  /* check if we should print version information */
  if (opt_version)
    {
      g_print ("%s %s (Xfce %s)\n\n", PACKAGE_NAME, PACKAGE_VERSION, 
               xfce_version_string ());
      g_print ("%s\n", "Copyright (c) 2004-2007 Benedikt Meurer <benny@xfce.org>");
      g_print ("%s\n", "Copyright (c) 2010-2011 Jannis Pohlmann <jannis@xfce.org>");
      g_print ("\n%s\n\n", _("All rights reserved."));
      g_print (_("Please report bugs to <%s>."), PACKAGE_BUGREPORT);
      g_print ("\n");
      return EXIT_SUCCESS;
    }

  /* initialize xfconf */
  if (!xfconf_init (&error))
    {
      g_fprintf (stderr, "%s: %s.\n", g_get_prgname (), error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  /* check if we should print the version information */
  if (opt_configure)
    {
      /* the --configure/-c option of thunar-volman exists for backwards-compatibility
       * reasons only. what we really do here is spawning thunar-volman-settings */
      if (!g_spawn_command_line_sync ("thunar-volman-settings", NULL, NULL, &exit_code, 
                                      &error))
        {
          g_fprintf (stderr, "%s: %s.\n", g_get_prgname (), error->message);
          g_error_free (error);
          exit_code = WEXITSTATUS (exit_code);
        }
    }
  else if (opt_sysfs_path != NULL)
    {
      /* create an udev client */
      client = g_udev_client_new (supported_udev_subsystems);

      /* determine the device belonging to the sysfs path */
      device = g_udev_client_query_by_sysfs_path (client, opt_sysfs_path);

      if (device != NULL)
        {
          /* get a reference on the thunar-volman settings channel */
          channel = xfconf_channel_get ("thunar-volman");

          /* create a new main loop */
          loop = g_main_loop_new (NULL, FALSE);

          /* allocate a new TvmContext */
          context = tvm_context_new (client, device, channel, loop, &error);

          /* handle the new device in an idle handler */
          g_idle_add ((GSourceFunc) tvm_context_run, context);

          /* release channel and device */
          g_object_unref (device);
        }
      else
        {
          g_set_error (&error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("There is no device with the sysfs path \"%s\""), 
                       opt_sysfs_path);
        }

      /* release the udev client */
      g_object_unref (client);
    }
  else
    {
      /* TRANSLATORS: thunar-volman wasn't invoked with either --device-added or
       * --configure */
      g_set_error (&error, G_FILE_ERROR, G_FILE_ERROR_FAILED, 
                   _("Must specify the sysfs path of new devices with --device-added"));
    }

  /* run the main loop */
  if (loop != NULL)
    {
      g_main_loop_run (loop);
      g_main_loop_unref (loop);
    }

  if (error != NULL)
    {
      g_fprintf (stderr, "%s: %s.\n", g_get_prgname (), error->message);
      g_error_free (error);
      exit_code = EXIT_FAILURE;
    }

#ifdef HAVE_LIBNOTIFY
  tvm_notify_uninit ();
#endif

  /* release the device context */
  if (context != NULL)
    tvm_context_free (context);

  /* free xfconf resources */
  xfconf_shutdown ();

  return exit_code;
}
