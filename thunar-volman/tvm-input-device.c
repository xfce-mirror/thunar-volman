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

#include <thunar-volman/tvm-input-device.h>
#include <thunar-volman/tvm-context.h>
#include <thunar-volman/tvm-device.h>
#include <thunar-volman/tvm-notify.h>
#include <thunar-volman/tvm-run.h>



void
tvm_input_device_added (TvmContext *context)
{
  const gchar *devname;
  const gchar *icon;
  const gchar *summary;
  const gchar *message;
  const gchar *id_class;
  const gchar *id_model;
  const gchar *id_usb_driver;
  const gchar *driver;
  const gchar *enabled_property = NULL;
  const gchar *command_property = NULL;
  gboolean     is_keyboard;
  gboolean     is_mouse;
  gboolean     is_tablet;
  gboolean     enabled;
  gchar       *command;

  g_return_if_fail (context != NULL);

  /* collect device information */
  id_class = g_udev_device_get_property (context->device, "ID_CLASS");
  id_model = g_udev_device_get_property (context->device, "ID_MODEL");
  driver = g_udev_device_get_property (context->device, "DRIVER");
  id_usb_driver = g_udev_device_get_property (context->device, "ID_USB_DRIVER");
  is_keyboard = g_udev_device_get_property_as_boolean (context->device, "ID_INPUT_KEYBOARD");
  is_mouse = g_udev_device_get_property_as_boolean (context->device, "ID_INPUT_MOUSE");
  is_tablet = g_udev_device_get_property_as_boolean (context->device, "ID_INPUT_TABLET");
  devname = g_udev_device_get_property (context->device, "DEVNAME");

  if (is_keyboard || g_strcmp0 (id_class, "kbd") == 0)
    {
      /* we have a keyboard */
      enabled_property = "/autokeyboard/enabled";
      command_property = "/autokeyboard/command";

      icon = "input-keyboard";
      summary = _("Keyboard detected");
      message = _("A keyboard was detected");
    }
  else if (is_tablet
           || g_strcmp0 (driver, "wacom") == 0
           || g_strcmp0 (id_usb_driver, "wacom") == 0)
    {
      /* we have a wacom tablet */
      enabled_property = "/autotablet/enabled";
      command_property = "/autotablet/command";

      icon = "input-tablet";
      summary = _("Tablet detected");
      message = _("A graphics tablet was detected");
    }
  else if (is_mouse || g_strcmp0 (id_class, "mouse") == 0)
    {
      if (g_strstr_len (id_model, -1, "Tablet") != NULL 
          || g_strstr_len (id_model, -1, "TABLET") != NULL
          || g_strstr_len (id_model, -1, "tablet") != NULL)
        {
          /* we have a tablet that can be used as a mouse */
          enabled_property = "/autotablet/enabled";
          command_property = "/autotablet/command";
    
          icon = "input-tablet";
          summary = _("Tablet detected");
          message = _("A graphics tablet was detected");
        }
      else if (devname == NULL || !g_str_has_prefix (devname, "/dev/input/event"))
        {
          /* we have a normal mouse */
          enabled_property = "/automouse/enabled";
          command_property = "/automouse/command";
    
          icon = "input-mouse";
          summary = _("Mouse detected");
          message = _("A mouse was detected");
        }
    }

  /* check if we have a device that we support */
  if (enabled_property != NULL && command_property != NULL)
    {
      /* check whether handling the keyboard/tablet/mouse is enabled */
      enabled = xfconf_channel_get_bool (context->channel, enabled_property, FALSE);
      if (enabled)
        {
#ifdef HAVE_LIBNOTIFY
          /* display a detection notification */
          tvm_notify (icon, summary, message);
#endif

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
      /* return an error because we cannot handle the input device */
      g_set_error (context->error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("Unsupported input device type"));
    }

  /* finish processing the device */
  tvm_device_handler_finished (context);
}
