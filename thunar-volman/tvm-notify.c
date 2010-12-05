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

#include <glib.h>

#include <libnotify/notify.h>

#include <libxfce4util/libxfce4util.h>

#include <thunar-volman/tvm-notify.h>



static gboolean tvm_notify_initted = FALSE;



void
tvm_notify (const gchar *icon,
            const gchar *summary,
            const gchar *message)
{
  NotifyNotification *notification;
  gchar              *spec_version = NULL;

  if (G_UNLIKELY (!tvm_notify_initted))
    {
      if (notify_init (PACKAGE_NAME))
        {
          /* we do this to work around bugs in libnotify < 0.6.0. Older
           * versions crash in notify_uninit() when no notifications are
           * displayed before. These versions also segfault when the
           * ret_spec_version parameter of notify_get_server_info is
           * NULL... */
          notify_get_server_info (NULL, NULL, NULL, &spec_version);
          g_free (spec_version);

          tvm_notify_initted = TRUE;
        }
      else
        {
          /* initialization failed; don't bother about the notification */
          return;
        }
    }

#ifdef NOTIFY_CHECK_VERSION
#if NOTIFY_CHECK_VERSION (0, 7, 0)
  notification = notify_notification_new (summary, message, icon);
#else
  notification = notify_notification_new (summary, message, icon, NULL);
#endif
#else
  notification = notify_notification_new (summary, message, icon, NULL);
#endif
  notify_notification_set_urgency (notification, NOTIFY_URGENCY_NORMAL);
  notify_notification_set_timeout (notification, NOTIFY_EXPIRES_DEFAULT);
  notify_notification_show (notification, NULL);
  g_object_unref (notification);
}



gchar *
tvm_notify_decode (const gchar *str)
{
  GString     *string;
  const gchar *p;
  gchar       *result;
  gchar        decoded_c;

  if (str == NULL)
    return NULL;

  if (!g_utf8_validate (str, -1, NULL))
    return NULL;

  string = g_string_new (NULL);

  for (p = str; p != NULL && *p != '\0'; ++p)
    {
      if (*p == '\\' && p[1] == 'x' && g_ascii_isalnum (p[2]) && g_ascii_isalnum (p[3]))
        {
          decoded_c = (g_ascii_xdigit_value (p[2]) << 4) | g_ascii_xdigit_value (p[3]);
          g_string_append_c (string, decoded_c);
          p = p + 3;
        }
      else
        g_string_append_c (string, *p);
    }

  result = string->str;
  g_string_free (string, FALSE);

  return result;
}



void
tvm_notify_uninit (void)
{
  if (tvm_notify_initted
      && notify_is_initted ())
    notify_uninit ();
}
