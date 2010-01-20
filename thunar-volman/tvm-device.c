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

#include <thunar-volman/tvm-block-device.h>
#include <thunar-volman/tvm-device.h>
#include <thunar-volman/tvm-input-device.h>



typedef struct _TvmDeviceHandler TvmDeviceHandler;
struct _TvmDeviceHandler
{
  const gchar       *subsystem;
  TvmDeviceCallback  callback;
};



static TvmDeviceHandler subsystem_handlers[] =
{
  { "block",       tvm_block_device_added },
  { "input",       tvm_input_device_added },
#if 0 /* TODO */
  { "sound",       tvm_sound_device_added },
  { "video4linux", tvm_video_device_added },
#endif
};



void
tvm_device_added (GUdevClient   *client,
                  GUdevDevice   *device,
                  XfconfChannel *channel,
                  GError       **error)
{
  const gchar *const *keys;
  const gchar        *subsystem;
  GError             *err = NULL;
  guint               n;

  g_return_if_fail (G_UDEV_IS_CLIENT (client));
  g_return_if_fail (G_UDEV_IS_DEVICE (device));
  g_return_if_fail (XFCONF_IS_CHANNEL (channel));
  g_return_if_fail (error == NULL || *error == NULL);

  g_debug ("device added:");
  keys = g_udev_device_get_property_keys (device);
  for (n = 0; keys != NULL && keys[n] != NULL; ++n)
    g_debug ("    %s=%s", keys[n], g_udev_device_get_property (device, keys[n]));

  /* determine the subsystem to which the device belongs */
  subsystem = g_udev_device_get_subsystem (device);

  /* try all subsystem handlers until one of them succeeds */
  for (n = 0; err == NULL && n < G_N_ELEMENTS (subsystem_handlers); ++n)
    {
      if (g_strcmp0 (subsystem, subsystem_handlers[n].subsystem) == 0)
        {
          if (subsystem_handlers[n].callback (client, device, channel, &err))
            break;
        }
    }

  /* propagate possible errors */
  if (err != NULL)
    g_propagate_error (error, err);
}
