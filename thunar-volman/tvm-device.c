/* $Id$ */
/*-
 * Copyright (c) 2007 Benedikt Meurer <benny@xfce.org>.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <dbus/dbus-glib-lowlevel.h>

#include <thunar-volman/tvm-block-device.h>
#include <thunar-volman/tvm-camera-device.h>
#include <thunar-volman/tvm-device.h>
#include <thunar-volman/tvm-input-device.h>
#include <thunar-volman/tvm-pda-device.h>
#include <thunar-volman/tvm-printer-device.h>



typedef struct _TvmDeviceHandler TvmDeviceHandler;
struct _TvmDeviceHandler
{
  const gchar      *capability;
  TvmDeviceCallback callback;
};



static const TvmDeviceHandler handlers[] =
{
  { "block",          tvm_block_device_added,   },
  { "camera",         tvm_camera_device_added,  },
  { "input.keyboard", tvm_input_device_added,   },
  { "input.mouse",    tvm_input_device_added,   },
  { "input.tablet",   tvm_input_device_added,   },
  { "pda",            tvm_pda_device_added,     },
  { "printer",        tvm_printer_device_added, },
};



static gint
strptrcmp (gconstpointer strptr1,
           gconstpointer strptr2)
{
	return strcmp (*((const gchar **) strptr1),
                 *((const gchar **) strptr2));
}



/**
 * tvm_device_added:
 * @preferences : a #TvmPreferences.
 * @udi         : the HAL device UDI of the newly added device.
 * @error       : return location for errors or %NULL.
 *
 * Invoked whenever a new device is added, where @udi is the
 * HAL device UDI of the device in question. Returns %FALSE if
 * an unrecoverable error occurred, %TRUE otherwise.
 *
 * Return value: %FALSE in case of an unrecoverable error,
 *               %TRUE otherwise.
 **/
gboolean
tvm_device_added (TvmPreferences *preferences,
                  const gchar    *udi,
                  GError        **error)
{
  DBusConnection *connection;
  LibHalContext  *context;
  DBusError       derror;
  GError         *err = NULL;
  gchar         **capabilities;
  gint            n_capabilities;
  gint            i, j, n;

  g_return_val_if_fail (exo_hal_udi_validate (udi, -1, NULL), FALSE);
  g_return_val_if_fail (TVM_IS_PREFERENCES (preferences), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* try to allocate a new HAL context */
  context = libhal_ctx_new ();
  if (G_UNLIKELY (context == NULL))
    {
      /* out of memory */
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_NOMEM, g_strerror (ENOMEM));
      return FALSE;
    }

  /* initialize D-Bus error */
  dbus_error_init (&derror);

  /* try to connect to the system bus */
  connection = dbus_bus_get (DBUS_BUS_SYSTEM, &derror);
  if (G_UNLIKELY (connection == NULL))
    {
err0: /* release the HAL context */
      libhal_ctx_free (context);

      /* propagate the error */
      dbus_set_g_error (error, &derror);
      return FALSE;
    }

  /* setup the D-Bus connection for the GLib main loop */
  dbus_connection_setup_with_g_main (connection, NULL);

  /* setup the D-Bus connection for the HAL context */
  libhal_ctx_set_dbus_connection (context, connection);

  /* the HAL context now owns the connection */
  dbus_connection_unref (connection);

  /* try to initialize the HAL context */
  if (!libhal_ctx_init (context, &derror))
    goto err0;

  /* query the capabilities of the device */
  capabilities = libhal_device_get_property_strlist (context, udi, "info.capabilities", &derror);
  if (G_UNLIKELY (capabilities == NULL))
    {
      /* shutdown the HAL context */
      libhal_ctx_shutdown (context, NULL);
      goto err0;
    }

  /* determine the number of capabilities */
  n_capabilities = g_strv_length (capabilities);

  /* sort the capabilities */
  qsort (capabilities, n_capabilities, sizeof (*capabilities), strptrcmp);

  /* try various handlers until one of them succeeds */
  for (i = 0, j = 0; err == NULL && i < G_N_ELEMENTS (handlers) && j < n_capabilities; ++i)
    {
      /* search for a handler with the capability */
      for (n = -1; j < n_capabilities; )
        {
          /* check if we have a match here */
          n = strcmp (capabilities[j], handlers[i].capability);
          if (G_LIKELY (n >= 0))
            break;
          ++j;
        }

      /* check if we have a potential match */
      if (n == 0)
        {
          /* try to handle the device */
          if ((*handlers[i].callback) (preferences, context, udi, capabilities[j], &err))
            break;
          ++j;
        }
    }

  /* cleanup the capabilities */
  libhal_free_string_array (capabilities);

  /* shutdown the HAL context */
  libhal_ctx_shutdown (context, NULL);
  libhal_ctx_free (context);

  /* check if we failed */
  if (G_UNLIKELY (err != NULL))
    {
      /* propagate the error */
      g_propagate_error (error, err);
      return FALSE;
    }

  return TRUE;
}


