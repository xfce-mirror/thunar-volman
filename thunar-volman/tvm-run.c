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

#include <glib/gi18n.h>

#include <gudev/gudev.h>

#include <gtk/gtk.h>

#include <xfconf/xfconf.h>

#include <thunar-volman/tvm-prompt.h>



gboolean 
tvm_run_burn_software (GUdevClient   *client,
                       GUdevDevice   *device,
                       XfconfChannel *channel,
                       GError       **error)
{
  static const gchar *cd_criteria[] = {
    "ID_CDROM_MEDIA_CD_R",
    "ID_CDROM_MEDIA_CD_RW",
  };
  
  static const gchar *dvd_criteria[] = {
    "ID_CDROM_MEDIA_DVD_R",
    "ID_CDROM_MEDIA_DVD_RW",
    "ID_CDROM_MEDIA_DVD_PLUS_R",
    "ID_CDROM_MEDIA_DVD_PLUS_RW",
  };
  gboolean            autoburn;
  gboolean            is_cd = FALSE;
  gboolean            is_dvd = FALSE;
  gboolean            result = FALSE;
  const gchar        *command_property;
  gchar              *command;
  guint               n;
  gint                response;

  g_return_val_if_fail (G_UDEV_IS_DEVICE (device), FALSE);
  g_return_val_if_fail (XFCONF_IS_CHANNEL (channel), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* abort without error if autoburning is turned off */
  if (!xfconf_channel_get_bool (channel, "/autoburn/enabled", FALSE))
    return FALSE;

  /* check if the disc is a CD */
  for (n = 0; !is_cd && n < G_N_ELEMENTS (cd_criteria); ++n)
    if (g_udev_device_get_property_as_boolean (device, cd_criteria[n]))
      is_cd = TRUE;

  /* check if the disc is a DVD */
  for (n = 0; !is_cd && !is_dvd && n < G_N_ELEMENTS (dvd_criteria); ++n)
    if (g_udev_device_get_property_as_boolean (device, dvd_criteria[n]))
      is_dvd = TRUE;

  g_debug ("is_cd = %i, is_dvd = i", is_cd, is_dvd);

  if (is_dvd)
    {
      /* ask what to do with the empty DVD */
      response = tvm_prompt (client, device, "gnome-dev-disc-dvdr",
                             _("Blank DVD inserted"),
                             _("You have inserted a blank DVD."),
                             _("What would you like to do?"),
                             _("Ig_nore"), GTK_RESPONSE_CANCEL,
                             _("Burn _DVD"), TVM_RESPONSE_BURN_DATA_CD, 
                             NULL);
    }
  else
    {
      /* ask whether to burn data or audio CD */
      response = tvm_prompt (client, device, "gnome-dev-disc-cdr",
                             _("Blank CD inserted"),
                             _("You have inserted a blank CD."),
                             _("What would you like to do?"),
                             _("Ig_nore"), GTK_RESPONSE_CANCEL,
                             _("Burn _Data CD"), TVM_RESPONSE_BURN_DATA_CD,
                             _("Burn _Audio CD"), TVM_RESPONSE_BURN_AUDIO_CD,
                             NULL);
    }

  /* determine the autoburn command property */
  if (response == TVM_RESPONSE_BURN_DATA_CD)
    command_property = "/autoburn/data-cd-command";
  else if (response == TVM_RESPONSE_BURN_AUDIO_CD)
    command_property = "/autoburn/audio-cd-command";
  else 
    return TRUE;

  /* determine the command to launch */
  command = xfconf_channel_get_string (channel, command_property, NULL);

  /* only try to launch the command if it is set and non-empty */
  if (command != NULL && *command != '\0')
    {
      /* try to execute the preferred burn software */
      /* result = tvm_run_command (device, command, NULL, NULL, error); */
    }
  else
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("The burn command may not be empty"));
    }

  /* free the burn command */
  g_free (command);

  return result;
}
