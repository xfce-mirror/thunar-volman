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

#include <gudev/gudev.h>

#include <libxfce4util/libxfce4util.h>

#include <xfconf/xfconf.h>

#include <thunar-volman/tvm-block-device.h>
#include <thunar-volman/tvm-context.h>
#include <thunar-volman/tvm-device.h>
#include <thunar-volman/tvm-input-device.h>
#include <thunar-volman/tvm-usb-device.h>



typedef struct _TvmDeviceHandler TvmDeviceHandler;



static void tvm_device_try_next_handler (TvmContext *context);



struct _TvmDeviceHandler
{
  const gchar         *subsystem;
  TvmDeviceHandlerFunc func;
};



static TvmDeviceHandler subsystem_handlers[] = 
{
  { "block",       tvm_block_device_added },
  { "input",       tvm_input_device_added },
  { "usb",         tvm_usb_device_added   },
#if 0
  { "sound",       tvm_sound_device_added },
  { "video4linux", tvm_video_device_added },
#endif
};



void
tvm_device_handler_finished (TvmContext *context)
{
  g_return_if_fail (context != NULL);
  
  if (context->error != NULL && *context->error != NULL)
    {
      g_list_free (context->handlers);
      g_main_loop_quit (context->loop);
    }
  else
    {
      if (context->handlers != NULL)
        tvm_device_try_next_handler (context);
      else
        g_main_loop_quit (context->loop);
    }
}



static void
tvm_device_try_next_handler (TvmContext *context)
{
  TvmDeviceHandler *handler;

  g_return_if_fail (context != NULL);

  handler = context->handlers->data;
  context->handlers = g_list_delete_link (context->handlers, context->handlers);

  handler->func (context);
}



void
tvm_device_added (TvmContext *context)
{
  const gchar *const *keys = NULL;
  const gchar        *subsystem;
  gint                n;

  g_return_if_fail (context != NULL);

#ifdef DEBUG
  g_debug ("tvm_device_added:");
  keys = g_udev_device_get_property_keys (context->device);
  for (n = 0; keys != NULL && keys[n] != NULL; ++n)
    g_debug ("    %s = %s", keys[n], g_udev_device_get_property (context->device, keys[n]));
#endif

  /* determine the subsystem to which the device belongs */
  subsystem = g_udev_device_get_subsystem (context->device);

  /* find all subsystem handlers for this subsystem */
  for (n = G_N_ELEMENTS (subsystem_handlers)-1; n >= 0; --n)
    if (g_strcmp0 (subsystem, subsystem_handlers[n].subsystem) == 0)
      context->handlers = g_list_prepend (context->handlers, &subsystem_handlers[n]);

  /* check if we have at least one handler */
  if (context->handlers != NULL)
    {
      /* try the next handler in the list */
      tvm_device_try_next_handler (context);
    }
  else
    {
      g_set_error (context->error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("Device type not supported"));
      g_main_loop_quit (context->loop);
    }
}
