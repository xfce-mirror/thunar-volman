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

#include <libxfce4util/libxfce4util.h>

#include <thunar-volman/tvm-context.h>
#include <thunar-volman/tvm-device.h>
#include <thunar-volman/tvm-run.h>
#include <thunar-volman/tvm-usb-device.h>



void
tvm_usb_device_added (TvmContext *context)
{
  const gchar *driver;
  const gchar *enabled_property = NULL;
  const gchar *command_property = NULL;
  gboolean     enabled;
  gboolean     is_camera;
  gchar       *command;

  g_return_if_fail (context != NULL);

  g_debug ("usb device added");

  /* collect device information */
  driver = g_udev_device_get_property (context->device, "DRIVER");
  is_camera = g_udev_device_get_property_as_boolean (context->device, "ID_GPHOTO2");

  if (is_camera)
    {
      g_debug ("is camera");

      enabled_property = "/autophoto/enabled";
      command_property = "/autophoto/command";
    }
  else if (g_strcmp0 (driver, "usblp") == 0)
    {
      g_debug ("is printer");

      enabled_property = "/autoprinter/enabled";
      command_property = "/autoprinter/command";
    }

  /* check if we have a device that we support */
  if (enabled_property != NULL && command_property != NULL)
    {
      /* check whether handling the printer or whatever is enabled */
      enabled = xfconf_channel_get_bool (context->channel, enabled_property, FALSE);
      if (enabled)
        {
          /* fetch the command for the input device type and try to run it */
          command = xfconf_channel_get_string (context->channel, command_property, NULL);
          if (command != NULL && *command != '\0')
            {
              tvm_run_command (context, NULL, command, context->error);
            }
          g_free (command);
        }
    }
  else
    {
      /* return an error because we cannot handle the usb device */
      g_set_error (context->error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("Unsupported USB device type"));
    }

  /* finish processing the device */
  tvm_device_handler_finished (context);
}
