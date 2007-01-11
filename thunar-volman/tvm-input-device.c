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

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <thunar-volman/tvm-input-device.h>
#include <thunar-volman/tvm-prompt.h>
#include <thunar-volman/tvm-run.h>



static const struct
{
  const gchar *capability;
  const gchar *auto_option_name;
  const gchar *auto_command_name;
} commands[] = {
  { "input.keyboard", "autokeyboard", "autokeyboard-command", },
  { "input.mouse",    "automouse",    "automouse-command",    },
  { "input.tablet",   "autotablet",   "autotablet-command",   },
};



/**
 * tvm_block_input_added:
 * @preferences : a #TvmPreferences.
 * @context     : a #LibHalContext.
 * @udi         : the HAL device UDI of the newly added input device.
 * @capability  : the capability, which caused this handler to be run.
 * @error       : return location for errors or %NULL.
 *
 * See #TvmDeviceCallback for further information.
 *
 * Return value: %TRUE if handled, %FALSE if not handled or an
 *               unrecoverable error occurred.
 **/
gboolean
tvm_input_device_added (TvmPreferences *preferences,
                        LibHalContext  *context,
                        const gchar    *udi,
                        const gchar    *capability,
                        GError        **error)
{
  gboolean result = FALSE;
  gboolean auto_enabled;
  gchar   *auto_command;
  guint    n;

  g_return_val_if_fail (exo_hal_udi_validate (udi, -1, NULL), FALSE);
  g_return_val_if_fail (TVM_IS_PREFERENCES (preferences), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (capability != NULL, FALSE);
  g_return_val_if_fail (context != NULL, FALSE);

  /* check which type of input device we have */
  for (n = 0; n < G_N_ELEMENTS (commands); ++n)
    if (strcmp (commands[n].capability, capability) == 0)
      break;
  if (G_LIKELY (n < G_N_ELEMENTS (commands)))
    {
      /* check if this handling is enabled and we have a command */
      g_object_get (G_OBJECT (preferences), commands[n].auto_option_name, &auto_enabled, commands[n].auto_command_name, &auto_command, NULL);
      if (G_LIKELY (auto_enabled && auto_command != NULL && *auto_command != '\0'))
        {
          /* try to run the command */
          result = tvm_run_command (context, udi, auto_command, NULL, NULL, error);
        }
      g_free (auto_command);
    }

  return result;
}




