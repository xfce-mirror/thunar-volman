/* $Id$ */
/*-
 * Copyright (c) 2007 Benedikt Meurer <benny@xfce.org>.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <glib/gstdio.h>

#include <exo-hal/exo-hal.h>

#include <thunar-vfs/thunar-vfs.h>

#include <thunar-volman/tvm-device.h>
#include <thunar-volman/tvm-preferences-dialog.h>



/* --- globals --- */
static gchar   *opt_hal_udi = NULL;
static gboolean opt_configure = FALSE;
static gboolean opt_version = FALSE;



/* --- command line options --- */
static GOptionEntry option_entries[] =
{
  { "device-added", 'a', 0, G_OPTION_ARG_STRING, &opt_hal_udi, N_ ("The HAL device UDI of the newly added device"), NULL, },
  { "configure", 'c', 0, G_OPTION_ARG_NONE, &opt_configure, N_ ("Configure management of removable drives and media"), NULL, },
  { "version", 'v', 0, G_OPTION_ARG_NONE, &opt_version, N_ ("Print version information and exit"), NULL, },
  { NULL, },
};



int
main (int argc, char **argv)
{
  TvmPreferences *preferences;
  GtkWidget      *dialog;
  GError         *err = NULL;

  /* setup translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* setup application name */
  g_set_application_name (_("Thunar Volume Manager"));

#ifdef G_ENABLE_DEBUG
  /* Do NOT remove this line for now, If something doesn't work,
   * fix your code instead!
   */
  g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);
#endif

  /* initialize the GThread system */
  if (!g_thread_supported ())
    g_thread_init (NULL);

  /* initialize Gtk+ */
  if (!gtk_init_with_args (&argc, &argv, NULL, option_entries, GETTEXT_PACKAGE, &err))
    {
      /* check if we have an error message */
      if (G_LIKELY (err == NULL))
        {
          /* no error message, the GUI initialization failed */
          const gchar *display_name = gdk_get_display_arg_name ();
          g_fprintf (stderr, "thunar-volman: %s: %s.\n", _("Failed to open display"), (display_name != NULL) ? display_name : " ");
        }
      else
        {
          /* yep, there's an error, so print it */
          g_fprintf (stderr, "%s: %s.\n", g_get_prgname (), err->message);
          g_error_free (err);
        }
      return EXIT_FAILURE;
    }

  /* check if we should print version information */
  if (G_UNLIKELY (opt_version))
    {
      g_print ("%s %s (Xfce %s)\n\n", PACKAGE_NAME, PACKAGE_VERSION, xfce_version_string ());
      g_print ("%s\n", "Copyright (c) 2004-2007");
      g_print ("\t%s\n\n", _("The Thunar development team. All rights reserved."));
      g_print ("%s\n\n", _("Written by Benedikt Meurer <benny@xfce.org>."));
      g_print (_("Please report bugs to <%s>."), PACKAGE_BUGREPORT);
      g_print ("\n");
      return EXIT_SUCCESS;
    }

  /* initialize the ThunarVFS library */
  thunar_vfs_init ();

  /* load the preferences for the volume manager */
  preferences = tvm_preferences_get ();

  /* check if we should configure the volume manager */
  if (G_UNLIKELY (opt_configure))
    {
      /* bring up the preferences dialog */
      dialog = tvm_preferences_dialog_new ();
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
    }
  else if (G_LIKELY (opt_hal_udi != NULL))
    {
      /* make sure the specified UDI is valid */
      if (!exo_hal_udi_validate (opt_hal_udi, -1, NULL))
        {
          /* TRANSLATORS: A HAL device UDI must match certain conditions to be valid (to be exact, it must be a valid D-Bus object path) */
          g_set_error (&err, G_FILE_ERROR, G_FILE_ERROR_FAILED, _("The specified UDI \"%s\" is not a valid HAL device UDI"), opt_hal_udi);
        }
      else
        {
          /* try to handle the newly added device */
          tvm_device_added (preferences, opt_hal_udi, &err);
        }
    }
  else
    {
      /* TRANSLATORS: thunar-volman wasn't invoked with either --device-added or --configure. */
      g_set_error (&err, G_FILE_ERROR, G_FILE_ERROR_FAILED, _("Must specify the new HAL device UDI with --device-added"));
    }

  /* flush the preferences */
  g_object_unref (G_OBJECT (preferences));

  /* shutdown thunar-vfs */
  thunar_vfs_shutdown ();

  /* check if an error occurred */
  if (G_UNLIKELY (err != NULL))
    {
      /* tell the user about the problem */
      g_fprintf (stderr, "%s: %s.\n", g_get_prgname (), err->message);
      g_error_free (err);
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

