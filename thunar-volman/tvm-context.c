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
#include <gio/gio.h>

#include <gudev/gudev.h>

#include <xfconf/xfconf.h>

#include <thunar-volman/tvm-context.h>
#include <thunar-volman/tvm-device.h>



TvmContext *
tvm_context_new (GUdevClient   *client,
                 GUdevDevice   *device,
                 XfconfChannel *channel,
                 GMainLoop     *loop,
                 GError       **error)
{
  TvmContext *context;

  g_return_val_if_fail (G_UDEV_IS_CLIENT (client), NULL);
  g_return_val_if_fail (G_UDEV_IS_DEVICE (device), NULL);
  g_return_val_if_fail (XFCONF_IS_CHANNEL (channel), NULL);
  g_return_val_if_fail (loop != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  context = g_slice_new0 (TvmContext);

  context->client = g_object_ref (client);
  context->device = g_object_ref (device);
  context->channel = g_object_ref (channel);
  context->error = error;
  context->loop = g_main_loop_ref (loop);
  context->handlers = NULL;
  context->monitor = g_volume_monitor_get ();

  return context;
}



void
tvm_context_free (TvmContext *context)
{
  if (context == NULL)
    return;

  g_main_loop_unref (context->loop);

  g_list_free (context->handlers);

  g_object_unref (context->monitor);
  g_object_unref (context->channel);
  g_object_unref (context->device);
  g_object_unref (context->client);

  g_slice_free (TvmContext, context);
}



gboolean
tvm_context_run (TvmContext *context)
{
  tvm_device_added (context);

  return FALSE;
}
