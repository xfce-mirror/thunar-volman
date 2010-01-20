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

#include <gudev/gudev.h>

#include <xfconf/xfconf.h>

#include <thunar-volman/tvm-input-device.h>



static gboolean
tvm_block_device_mount (GUdevClient   *client,
                        GUdevDevice   *device,
                        XfconfChannel *channel,
                        GError       **error)
{
  g_return_val_if_fail (G_UDEV_IS_CLIENT (client), FALSE);
  g_return_val_if_fail (G_UDEV_IS_DEVICE (device), FALSE);
  g_return_val_if_fail (XFCONF_IS_CHANNEL (channel), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  return TRUE;
}



gboolean
tvm_block_device_added (GUdevClient   *client,
                        GUdevDevice   *device,
                        XfconfChannel *channel,
                        GError       **error)
{ 
  const gchar *devtype;
  const gchar *id_type;
  const gchar *media_state;
  const gchar *fs_usage;
  gboolean     is_cdrom;
  gboolean     is_partition;
  guint64      audio_tracks;

  g_return_val_if_fail (G_UDEV_IS_CLIENT (client), FALSE);
  g_return_val_if_fail (G_UDEV_IS_DEVICE (device), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* collect general device information */
  devtype = g_udev_device_get_devtype (device);
  id_type = g_udev_device_get_property (device, "ID_TYPE");

  /* distinguish device types */
  is_cdrom = (g_strcmp0 (id_type, "cd") == 0);
  is_partition = (g_strcmp0 (devtype, "partition") == 0);

  if (is_cdrom)
    {
      /* abort silently if CD media is available */
      if (!g_udev_device_get_property_as_boolean (device, "ID_CDROM_MEDIA"))
        return TRUE;

      /* collect CD information */
      media_state = g_udev_device_get_property (device, "ID_CDROM_MEDIA_STATE");
      audio_tracks = g_udev_device_get_property_as_uint64 (device, "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO");

      if (g_strcmp0 (media_state, "blank") == 0)
        {
          return tvm_run_burn_software (client, device, channel, error);
        }
      else if (audio_tracks > 0)
        {
          /* TODO detect mixed CDs with audio AND data tracks */
          return tvm_run_cd_player (client, device, channel, error);
        }
      else
        {
          /* mount the CD/DVD and continue with inspecting its contents */
          return tvm_block_device_mount (client, device, channel, error);
        }
    }
  else if (is_partition)
    {
      /* collect more information about the partition */
      fs_usage = g_udev_device_get_property (device, "ID_FS_USAGE");

      if (g_strcmp0 (fs_usage, "crypto") == 0)
        {
          /* TODO return tvm_crypto_device_added (device, channel, error); */
        }
      else
        {
          /* mount the partition and continue with inspecting its contents */
          return tvm_block_device_mount (client, device, channel, error);
        }
    }
  else
    {
      /* TODO detect encrypted volumes, implement this before 
       * the regular partition check */
    }

  return TRUE;
}
