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

#include <gio/gio.h>

#include <thunar-volman/tvm-gio-extensions.h>



GVolume *
tvm_g_volume_monitor_get_volume_for_kind (GVolumeMonitor *monitor,
                                          const gchar    *kind,
                                          const gchar    *identifier)
{
  GVolume *volume = NULL;
  GList   *volumes;
  GList   *lp;
  gchar   *value;

  g_return_val_if_fail (G_IS_VOLUME_MONITOR (monitor), NULL);
  g_return_val_if_fail (kind != NULL && *kind != '\0', NULL);
  g_return_val_if_fail (identifier != NULL && *identifier != '\0', NULL);

  volumes = g_volume_monitor_get_volumes (monitor);

  for (lp = volumes; volume == NULL && lp != NULL; lp = lp->next)
    {
      value = g_volume_get_identifier (lp->data, kind);
      if (value != NULL)
        {
          if (g_strcmp0 (value, identifier) == 0)
            volume = g_object_ref (lp->data);
         
          g_free (value);
        }

      g_object_unref (lp->data);
    }

  g_list_free (volumes);

  return volume;
}
