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



void
tvm_notify (const gchar *icon,
            const gchar *summary,
            const gchar *message)
{
  NotifyNotification *notification;

  notification = notify_notification_new (summary, message, icon, NULL);
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