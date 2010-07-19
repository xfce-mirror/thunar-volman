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

#include <thunar-volman/tvm-context.h>
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
  g_return_val_if_fail (G_IS_MOUNT (mount), FALSE);
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
      g_debug ("%s", command_line->str);

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
