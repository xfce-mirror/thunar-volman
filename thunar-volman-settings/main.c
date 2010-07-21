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

#include <libxfce4util/libxfce4util.h>

#include <xfconf/xfconf.h>

#include <thunar-volman-settings/tvm-preferences-dialog.h>


int
main (int    argc,
      char **argv)
{
  GtkWidget *dialog;
  GError    *error = NULL;

  /* setup translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* setup application name */
  g_set_application_name (_("Thunar Volume Manager Settings"));

#ifdef G_ENABLE_DEBUG
  /* Do NOT remove this line for now. If something doesn't work, fix your code instead */
  g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);
#endif

  /* initialize the threading system */
  if (!g_thread_supported ())
    g_thread_init (NULL);

  /* initialize GTK+ */
  if (!gtk_init_with_args (&argc, &argv, NULL, NULL, GETTEXT_PACKAGE, &error))
    {
      g_fprintf (stderr, "%s: %s.\n", g_get_prgname (), error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  /* initialize xfconf */
  if (!xfconf_init (&error))
    {
      g_fprintf (stderr, "%s: %s.\n", g_get_prgname (), error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  /* display the dialog */
  dialog = tvm_preferences_dialog_new ();
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  /* free xfconf resources */
  xfconf_shutdown ();

  return EXIT_SUCCESS;
}
