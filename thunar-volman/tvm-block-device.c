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

#include <gudev/gudev.h>

#include <libxfce4util/libxfce4util.h>

#include <thunar-volman/tvm-block-device.h>
#include <thunar-volman/tvm-context.h>
#include <thunar-volman/tvm-device.h>
#include <thunar-volman/tvm-gio-extensions.h>
#include <thunar-volman/tvm-run.h>



typedef gboolean (*TvmBlockDeviceHandler) (TvmContext *context,
                                           GMount     *mount,
                                           GError    **error);



static gboolean tvm_file_test               (GMount      *mount,
                                             const gchar *filename,
                                             GFileTest    test);
static gboolean tvm_block_device_autorun    (TvmContext  *context,
                                             GMount      *mount,
                                             GError     **error);
static gboolean tvm_block_device_autobrowse (TvmContext  *context,
                                             GMount      *mount,
                                             GError     **error);



static TvmBlockDeviceHandler block_device_handlers[] =
{
#if 0
  tvm_block_device_autoipod,
  tvm_block_device_autophoto,
#endif
  tvm_block_device_autorun,
  tvm_block_device_autobrowse,
};



static gboolean
tvm_file_test (GMount      *mount,
               const gchar *filename,
               GFileTest    test)
{
  const gchar *name;
  gboolean     result = FALSE;
  GFile       *mount_point;
  gchar       *directory;
  gchar       *path;
  GDir        *dp;

  g_return_val_if_fail (G_IS_MOUNT (mount), FALSE);
  g_return_val_if_fail (filename != NULL && *filename != '\0', FALSE);

  mount_point = g_mount_get_root (mount);
  directory = g_file_get_path (mount_point);
  g_object_unref (mount_point);

  /* try to open the specified directory */
  dp = g_dir_open (directory, 0, NULL);
  if (G_LIKELY (dp != NULL))
    {
      while (!result)
        {
          /* read the next entry */
          name = g_dir_read_name (dp);
          if (G_UNLIKELY (name == NULL))
            break;

          /* check if we have a potential match */
          if (g_ascii_strcasecmp (name, filename) == 0)
            {
              /* check if test condition met */
              path = g_build_filename (directory, name, NULL);
              result = g_file_test (path, test);
              g_free (path);
            }
        }

      /* cleanup */
      g_dir_close (dp);
    }

  g_free (directory);

  return result;
}



static gboolean
tvm_block_device_autorun (TvmContext *context,
                          GMount     *mount,
                          GError    **error)
{
  gboolean autoplay;
  gboolean result = FALSE;
  GError  *err = NULL;
  gchar   *autoplay_command;

  g_return_val_if_fail (context != NULL, FALSE);
  g_return_val_if_fail (G_IS_MOUNT (mount), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* check if autoplaying video CDs and DVDs is enabled */
  autoplay = xfconf_channel_get_bool (context->channel, 
                                      "/autoplay-video-cds/enabled", FALSE);
  if (autoplay)
    {
      /* check if we have a video CD or video DVD here */
      if (tvm_file_test (mount, "vcd", G_FILE_TEST_IS_DIR) 
          || tvm_file_test (mount, "video_ts", G_FILE_TEST_IS_DIR))
        {
          /* determine the autoplay command for video CDs/DVDs */
          autoplay_command = xfconf_channel_get_string (context->channel,
                                                        "/autoplay-video-cds/command", 
                                                        "parole");

          /* try to spawn the preferred video CD/DVD player */
          result = tvm_run_command (context, mount, autoplay_command, &err);

          /* free the command string */
          g_free (autoplay_command);

          /* forward errors to the caller */
          if (err != NULL)
            g_propagate_error (error, err);

          /* return success/failure to the caller */
          return result;
        }
    }

  return TRUE;
}



static gboolean
tvm_block_device_autobrowse (TvmContext *context,
                             GMount     *mount,
                             GError    **error)
{
  gboolean autobrowse;
  gboolean result = FALSE;
  GError  *err = NULL;

  g_return_val_if_fail (context != NULL, FALSE);
  g_return_val_if_fail (G_IS_MOUNT (mount), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* check whether auto-browsing of volumes is enabled */
  autobrowse = xfconf_channel_get_bool (context->channel, "/autobrowse/enabled", FALSE);
  if (autobrowse) 
    {
      /* try to open the mount point in thunar */
      result = tvm_run_command (context, mount, "Thunar %m", &err);
    }

  /* forward errors to the caller */
  if (err != NULL)
    g_propagate_error (error, err);

  return result;
}



static void
tvm_block_device_mounted (TvmContext *context,
                          GMount     *mount,
                          GError    **error)
{
  gboolean success = FALSE;
  GError  *err = NULL;
  guint    n;

  g_return_if_fail (context != NULL);
  g_return_if_fail (G_IS_MOUNT (mount));
  g_return_if_fail (error == NULL || *error == NULL);

  /* try block device handlers (iPod, cameras etc.) until one succeeds */
  for (n = 0; !success && n < G_N_ELEMENTS (block_device_handlers); ++n)
    success = (block_device_handlers[n]) (context, mount, &err);

  /* forward errors to the caller */
  if (err != NULL)
    g_propagate_error (error, err);
}



static void
tvm_block_device_mount_finish (GVolume      *volume,
                               GAsyncResult *result,
                               TvmContext   *context)
{
  GMount *mount;
  GError *error = NULL;
  
  g_return_if_fail (G_IS_VOLUME (volume));
  g_return_if_fail (G_IS_ASYNC_RESULT (result));
  g_return_if_fail (context != NULL);

  /* finish mounting the volume */
  if (g_volume_mount_finish (volume, result, &error))
    {
      /* get the moint point of the volume */
      mount = g_volume_get_mount (volume);

      if (mount != NULL)
        {
          /* inspect volume contents and perform actions based on them */
          tvm_block_device_mounted (context, mount, &error);

          /* release the mount point */
          g_object_unref (mount);
        }
      else
        {
          /* could not locate the mount point */
          g_set_error (&error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("Unable to locate mount point"));
        }
    }

  /* release the volume */
  g_object_unref (volume);

  /* move error information into the context */
  if (error != NULL)
    g_propagate_error (context->error, error);

  /* finish processing the device */
  tvm_device_handler_finished (context);
}



static void
tvm_block_device_mount (TvmContext *context)
{
  GMountOperation *mount_operation;
  GVolume         *volume;

  g_return_if_fail (context != NULL);

  /* determine the GVolume corresponding to the udev device */
  volume = 
    tvm_g_volume_monitor_get_volume_for_kind (context->monitor,
                                              G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE,
                                              g_udev_device_get_device_file (context->device));

  /* check if we have a volume */
  if (volume != NULL)
    {
      /* check if we can mount the volume */
      if (g_volume_can_mount (volume))
        {
          /* try to mount the volume asynchronously */
          mount_operation = gtk_mount_operation_new (NULL);
          g_volume_mount (volume, G_MOUNT_MOUNT_NONE, mount_operation,
                          NULL, (GAsyncReadyCallback) tvm_block_device_mount_finish, context);
          g_object_unref (mount_operation);
        }
      else
        {
          g_set_error (context->error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("Unable to mount the device"));
        
          /* finish processing the device */
          tvm_device_handler_finished (context);
        }
    }
  else
    {
      g_set_error (context->error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("Could not detect the volume corresponding to the device"));
    
      /* finish processing the device */
      tvm_device_handler_finished (context);
    }
}



void
tvm_block_device_added (TvmContext *context)
{
  const gchar *devtype;
  const gchar *id_type;
  const gchar *media_state;
  const gchar *id_fs_usage;
  gboolean     is_cdrom;
  gboolean     is_partition;
  gboolean     is_volume;
  guint64      audio_tracks;
  GError      *error = NULL;

  g_return_if_fail (context != NULL);

  /* collect general device information */
  devtype = g_udev_device_get_devtype (context->device);
  id_type = g_udev_device_get_property (context->device, "ID_TYPE");
  id_fs_usage = g_udev_device_get_property (context->device, "ID_FS_USAGE");

  /* distinguish device types */
  is_cdrom = (g_strcmp0 (id_type, "cd") == 0);
  is_partition = (g_strcmp0 (devtype, "partition") == 0);
  is_volume = (g_strcmp0 (devtype, "disk") == 0) 
    && (g_strcmp0 (id_fs_usage, "filesystem") == 0);

  if (is_cdrom)
    {
      /* silently ignore CD drives without media */
      if (g_udev_device_get_property_as_boolean (context->device, "ID_CDROM_MEDIA"))
        {
          /* collect CD information */
          media_state = g_udev_device_get_property (context->device, 
                                                    "ID_CDROM_MEDIA_STATE");
          audio_tracks = 
            g_udev_device_get_property_as_uint64 (context->device, 
                                                  "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO");

          /* check if we have a blank CD/DVD here */
          if (g_strcmp0 (media_state, "blank") == 0)
            {
              /* try to run the burn program */
              if (!tvm_run_burn_software (context, &error))
                g_propagate_error (context->error, error);

              /* finish processing the device */
              tvm_device_handler_finished (context);
            }
          else if (audio_tracks > 0)
            {
#if 0
              /* TODO detect mixed CDs with audio AND data tracks */
              tvm_run_cd_player (client, device, channel, &error);
#endif
            }
          else
            {
              /* mount the CD/DVD and continue with inspecting its contents */
              tvm_block_device_mount (context);
            }
        }
      else
        {
          /* finish processing the device */
          tvm_device_handler_finished (context);
        }
    }
  else if (is_partition || is_volume)
    {
      /* mount the partition and continue with inspecting its contents */
      tvm_block_device_mount (context);
    }
  else
    {
      /* generate an error for logging */
      g_set_error (context->error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("Unknown block device type"));

      /* finish processing the device */
      tvm_device_handler_finished (context);
    }
}
