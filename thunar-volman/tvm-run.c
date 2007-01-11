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

#include <thunar-volman/tvm-prompt.h>
#include <thunar-volman/tvm-run.h>



static void tvm_run_resolve (LibHalContext *context,
                             const gchar   *udi,
                             gchar        **device_file_return,
                             gchar        **mount_point_return);



static void
tvm_run_resolve (LibHalContext *context,
                 const gchar   *udi,
                 gchar        **device_file_return,
                 gchar        **mount_point_return)
{
  GSList *mount_points;
  gchar  *device_file;

  /* initialize return values */
  if (device_file_return != NULL)
    *device_file_return = NULL;
  if (mount_point_return != NULL)
    *mount_point_return = NULL;

  /* determine the device file path of the device */
  device_file = libhal_device_get_property_string (context, udi, "block.device", NULL);
  if (G_LIKELY (device_file != NULL))
    {
      /* determine the active mount point(s) for the device from the kernel */
      mount_points = exo_mount_point_list_matched (EXO_MOUNT_POINT_MATCH_ACTIVE | EXO_MOUNT_POINT_MATCH_DEVICE, device_file, NULL, NULL, NULL);
      if (G_LIKELY (mount_points != NULL))
        {
          /* return the mount point folder path */
          if (G_LIKELY (mount_point_return != NULL))
            *mount_point_return = g_strdup (((ExoMountPoint *) mount_points->data)->folder);

          /* release the mount points */
          g_slist_foreach (mount_points, (GFunc) exo_mount_point_free, NULL);
          g_slist_free (mount_points);
        }

      /* return the device file path */
      if (G_LIKELY (device_file_return != NULL))
        *device_file_return = g_strdup (device_file);

      /* cleanup */
      libhal_free_string (device_file);
    }
}



/**
 * tvm_run_cdburner:
 * @preferences : a #TvmPreferences.
 * @context     : a #LibHalContext.
 * @udi         : the UDI of the blank CD/DVD.
 * @error       : return location for errors or %NULL.
 *
 * Tries to spawn the preferred CD-Burner application for the
 * given @udi. Returns %TRUE if the device was handled, %FALSE
 * if not handled or an unrecoverable error occurred.
 *
 * Return value: %TRUE if handled, %FALSE otherwise.
 **/
gboolean
tvm_run_cdburner (TvmPreferences *preferences,
                  LibHalContext  *context,
                  const gchar    *udi,
                  GError        **error)
{
  const gchar *autoburn_command_name;
  gboolean     autoburn;
  gboolean     result = FALSE;
  gboolean     is_dvd;
  gchar       *autoburn_command;
  gchar       *disc_type;
  gint         response;

  g_return_val_if_fail (exo_hal_udi_validate (udi, -1, NULL), FALSE);
  g_return_val_if_fail (TVM_IS_PREFERENCES (preferences), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (context != NULL, FALSE);

  /* check if autoburn is enabled */
  g_object_get (G_OBJECT (preferences), "autoburn", &autoburn, NULL);
  if (G_UNLIKELY (!autoburn))
    return TRUE;

  /* check if we have a DVD here */
  disc_type = libhal_device_get_property_string (context, udi, "volume.disc.type", NULL);
  is_dvd = (disc_type != NULL && strncmp (disc_type, "dvd_", 4) == 0);
  libhal_free_string (disc_type);

  /* ask the user what to do now */
  if (G_LIKELY (is_dvd))
    {
      /* ask what to do with the empty DVD */
      response = tvm_prompt (context, udi, "gnome-dev-disc-dvdr",
                             _("Choose Disc Type"),
                             _("You have inserted a blank disc."),
                             _("What would you like to do?"),
                             _("Ig_nore"), GTK_RESPONSE_CANCEL,
                             _("Make _DVD"), TVM_RESPONSE_BURN_DATA_CD,
                             NULL);
    }
  else
    {
      /* ask whether to burn data or audio CD */
      response = tvm_prompt (context, udi, "gnome-dev-disc-cdr",
                             _("Choose Disc Type"),
                             _("You have inserted a blank disc."),
                             _("What would you like to do?"),
                             _("Ig_nore"), GTK_RESPONSE_CANCEL,
                             _("Make _Data CD"), TVM_RESPONSE_BURN_DATA_CD,
                             _("Make _Audio CD"), TVM_RESPONSE_BURN_AUDIO_CD,
                             NULL);
    }

  /* determine the autoburn command name */
  if (response == TVM_RESPONSE_BURN_DATA_CD)
    autoburn_command_name = "autoburn-data-command";
  else if (response == TVM_RESPONSE_BURN_AUDIO_CD)
    autoburn_command_name = "autoburn-audio-command";
  else
    return TRUE;

  /* determine the command */
  g_object_get (G_OBJECT (preferences), autoburn_command_name, &autoburn_command, NULL);
  if (G_LIKELY (autoburn_command != NULL && *autoburn_command != '\0'))
    {
      /* try to execute the preferred CD-Burner application */
      result = tvm_run_command (context, udi, autoburn_command, NULL, NULL, error);
    }
  g_free (autoburn_command);

  return result;
}



/**
 * tvm_run_cdplayer:
 * @preferences : a #TvmPreferences.
 * @context     : a #LibHalContext.
 * @udi         : the UDI of the Audio CD.
 * @error       : return location for errors or %NULL.
 *
 * Tries to spawn the preferred CD-Player application for the
 * given @udi. Returns %TRUE if the device was handled, %FALSE
 * if not handled or an unrecoverable error occurred.
 *
 * Return value: %TRUE if handled, %FALSE otherwise.
 **/
gboolean
tvm_run_cdplayer (TvmPreferences *preferences,
                  LibHalContext  *context,
                  const gchar    *udi,
                  GError        **error)
{
  gboolean autoplay;
  gboolean result = TRUE;
  gchar   *autoplay_command;

  g_return_val_if_fail (exo_hal_udi_validate (udi, -1, NULL), FALSE);
  g_return_val_if_fail (TVM_IS_PREFERENCES (preferences), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (context != NULL, FALSE);

  /* check if autoplay is enabled and we have a command */
  g_object_get (G_OBJECT (preferences), "autoplay-audio-cd", &autoplay, "autoplay-audio-cd-command", &autoplay_command, NULL);
  if (G_LIKELY (autoplay && autoplay_command != NULL && *autoplay_command != '\0'))
    {
      /* try to run the preferred CD-Player application */
      result = tvm_run_command (context, udi, autoplay_command, NULL, NULL, error);
    }
  g_free (autoplay_command);

  return result;
}



/**
 * tvm_run_command:
 * @context     : a #LibHalContext.
 * @udi         : the UDI of the device for which to run the @command.
 * @command     : the command line of the application to spawn.
 * @device_file : the value to substitute for <literal>%d</literal> or %NULL.
 * @mount_point : the value to substitute for <literal>%m</literal> or %NULL.
 * @error       : return location for errors or %NULL.
 *
 * Substitutes special values in @command and tries to spawn the application
 * identified by the @command.
 *
 * The HAL UDI will be substituted for <literal>%h</literal>, the device file path
 * for <literal>%d</literal> and the mount point for <literal>%m</literal>.
 *
 * Return value: %TRUE if the command was spawn successfully, %FALSE otherwise.
 **/
gboolean
tvm_run_command (LibHalContext *context,
                 const gchar   *udi,
                 const gchar   *command,
                 const gchar   *device_file,
                 const gchar   *mount_point,
                 GError       **error)
{
  const gchar *p;
  gboolean     result;
  GString     *command_line;
  gchar       *quoted;
  gchar       *device;
  gchar       *folder;
  gchar      **argv;

  g_return_val_if_fail (exo_hal_udi_validate (udi, -1, NULL), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (context != NULL, FALSE);

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
              /* check if know the device file path */
              if (G_LIKELY (device_file != NULL))
                device = g_strdup (device_file);
              else
                tvm_run_resolve (context, udi, &device, NULL);
              if (G_LIKELY (device != NULL))
                g_string_append (command_line, device);
              g_free (device);
              break;

            case 'h': /* HAL UDI */
              g_string_append (command_line, udi);
              break;

            case 'm': /* mount point */
              /* check if know the mount point */
              if (G_LIKELY (mount_point != NULL))
                folder = g_strdup (mount_point);
              else
                tvm_run_resolve (context, udi, NULL, &folder);
              if (G_LIKELY (folder != NULL))
                {
                  /* substitute mount point quoted */
                  quoted = g_shell_quote (folder);
                  g_string_append (command_line, quoted);
                  g_free (quoted);
                }
              else
                {
                  /* %m must always be substituted */
                  g_string_append (command_line, "\"\"");
                }
              g_free (folder);
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
  result = g_shell_parse_argv (command_line->str, NULL, &argv, error);
  if (G_LIKELY (result))
    {
      /* try to spawn the command asynchronously in the users home directory */
      result = g_spawn_async (g_get_home_dir (), argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, error);

      /* cleanup */
      g_strfreev (argv);
    }

  /* cleanup */
  g_string_free (command_line, TRUE);

  return result;
}



