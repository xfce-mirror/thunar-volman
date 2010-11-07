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

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>

#include <thunar-volman/tvm-context.h>
#include <thunar-volman/tvm-prompt.h>
#include <thunar-volman/tvm-run.h>



gboolean
tvm_run_command (TvmContext  *context,
                 GMount      *mount,
                 const gchar *command,
                 GError     **error)
{
  const gchar *device_file;
  const gchar *p;
  gboolean     result;
  GString     *command_line;
  GError      *err = NULL;
  GFile       *mount_point;
  gchar      **argv;
  gchar       *mount_path;
  gchar       *quoted;

  g_return_val_if_fail (context != NULL, FALSE);
  g_return_val_if_fail (mount == NULL || G_IS_MOUNT (mount), FALSE);
  g_return_val_if_fail (command != NULL && *command != '\0', FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* perform the required substitutions */
  command_line = g_string_new (NULL);
  for (p = command; *p != '\0'; ++p)
    {
      /* check if we should substitute */
      if (G_UNLIKELY (p[0] == '%' && p[1] != '\0'))
        {
          /* check which substitution to perform */
          switch (*++p)
            {
            case 'd': /* device file */
              device_file = g_udev_device_get_device_file (context->device);
              if (G_LIKELY (device_file != NULL))
                g_string_append (command_line, device_file);
              break;

            case 'm': /* mount point */
              if (mount != NULL)
                {
                  mount_point = g_mount_get_root (mount);
                  mount_path = g_file_get_path (mount_point);
                  if (G_LIKELY (mount_path != NULL))
                    {
                      /* substitute mount point quoted */
                      quoted = g_shell_quote (mount_path);
                      g_string_append (command_line, quoted);
                      g_free (quoted);
                    }
                  else
                    {
                      /* %m must always be substituted */
                      g_string_append (command_line, "\"\"");
                    }
                  g_free (mount_path);
                  g_object_unref (mount_point);
                }
              else
                {
                  /* %m must always be substituted */
                  g_string_append (command_line, "\"\"");
                }
              break;
              
            case '%':
              g_string_append_c (command_line, '%');
              break;

            default:
              g_string_append_c (command_line, '%');
              g_string_append_c (command_line, *p);
              break;
            }
        }
      else
        {
          /* just append the character */
          g_string_append_c (command_line, *p);
        }
    }

  /* try to parse the command line */
  result = g_shell_parse_argv (command_line->str, NULL, &argv, &err);
  if (G_LIKELY (result))
    {
      /* try to spawn the command asynchronously in the users home directory */
      result = g_spawn_async (g_get_home_dir (), argv, NULL, G_SPAWN_SEARCH_PATH, 
                              NULL, NULL, NULL, &err);

      /* cleanup */
      g_strfreev (argv);
    }

  /* cleanup */
  g_string_free (command_line, TRUE);

  if (err != NULL)
    g_propagate_error (error, err);

  return result;
}



gboolean 
tvm_run_burn_software (TvmContext *context,
                       GError    **error)
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
  gboolean            is_cd = FALSE;
  gboolean            is_dvd = FALSE;
  gboolean            result = FALSE;
  const gchar        *command_property;
  gchar              *command;
  guint               n;
  gint                response;

  g_return_val_if_fail (context != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* abort without error if autoburning is turned off */
  if (!xfconf_channel_get_bool (context->channel, "/autoburn/enabled", FALSE))
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, 
                   _("Autoburning of blank CDs and DVDs is disabled"));
      return FALSE;
    }

  /* check if the disc is a CD */
  for (n = 0; !is_cd && n < G_N_ELEMENTS (cd_criteria); ++n)
    if (g_udev_device_get_property_as_boolean (context->device, cd_criteria[n]))
      is_cd = TRUE;

  /* check if the disc is a DVD */
  for (n = 0; !is_cd && !is_dvd && n < G_N_ELEMENTS (dvd_criteria); ++n)
    if (g_udev_device_get_property_as_boolean (context->device, dvd_criteria[n]))
      is_dvd = TRUE;

  if (is_dvd)
    {
      /* ask what to do with the empty DVD */
      response = tvm_prompt (context, "media-optical",
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
      response = tvm_prompt (context, "media-optical",
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
  command = xfconf_channel_get_string (context->channel, command_property, NULL);

  /* only try to launch the command if it is set and non-empty */
  if (command != NULL && *command != '\0')
    {
      /* try to execute the preferred burn software */
      result = tvm_run_command (context, NULL, command, error);
    }
  else
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("The burn command may not be empty"));
      result = FALSE;
    }

  /* free the burn command */
  g_free (command);

  return result;
}



gboolean 
tvm_run_cd_player (TvmContext *context,
                   GError    **error)
{
  gboolean result = FALSE;
  gchar   *command;

  g_return_val_if_fail (context != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* check whether autoplaying audio CDs is enabled */
  if (xfconf_channel_get_bool (context->channel, "/autoplay-audio-cds/enabled", FALSE))
    {
      /* determine the audio CD player command */
      command = xfconf_channel_get_string (context->channel, 
                                           "/autoplay-audio-cds/command", NULL);

      /* check whether the command is set and non-empty */
      if (command != NULL && *command != '\0')
        {
          /* try to lanuch the audio CD player */
          result = tvm_run_command (context, NULL, command, error);
        }
      else
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("The CD player command is undefined"));
        }
      
      /* free the command string */
      g_free (command);
    }

  return result;
}
