/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2007-2008 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2010      Jannis Pohlmann <jannis@xfce.org>
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

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <gio/gio.h>

#include <gtk/gtk.h>

#include <gudev/gudev.h>

#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#include <thunar-volman/tvm-notify.h>
#endif

#include <libxfce4util/libxfce4util.h>

#include <thunar-volman/tvm-block-device.h>
#include <thunar-volman/tvm-context.h>
#include <thunar-volman/tvm-device.h>
#include <thunar-volman/tvm-gio-extensions.h>
#include <thunar-volman/tvm-prompt.h>
#include <thunar-volman/tvm-run.h>



typedef gboolean (*TvmBlockDeviceHandler) (TvmContext *context,
                                           GMount     *mount,
                                           GError    **error);



static gboolean tvm_file_test               (GMount      *mount,
                                             const gchar *filename,
                                             GFileTest    test);
static gboolean tvm_block_device_autoipod   (TvmContext  *context,
                                             GMount      *mount,
                                             GError     **error);
static gboolean tvm_block_device_autophoto  (TvmContext  *context,
                                             GMount      *mount,
                                             GError     **error);
static gboolean tvm_block_device_autorun    (TvmContext  *context,
                                             GMount      *mount,
                                             GError     **error);
static gboolean tvm_block_device_autobrowse (TvmContext  *context,
                                             GMount      *mount,
                                             GError     **error);



static TvmBlockDeviceHandler block_device_handlers[] =
{
  tvm_block_device_autoipod,
  tvm_block_device_autophoto,
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
tvm_block_device_autoipod (TvmContext *context,
                           GMount     *mount,
                           GError    **error)
{
  gboolean autoipod;
  gboolean is_audio_player = FALSE;
  gboolean is_ipod = FALSE;
  gboolean result = FALSE;
  GFile   *mount_point;
  gchar   *autoipod_command;
  gchar   *autophoto_command;
  gchar   *mount_path;
  gchar   *path_dcim = NULL;
  gint     response = TVM_RESPONSE_NONE;

  g_return_val_if_fail (context != NULL, FALSE);
  g_return_val_if_fail (G_IS_MOUNT (mount), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  autoipod = xfconf_channel_get_bool (context->channel, "/autoipod/enabled", FALSE);
  if (autoipod)
    {
      /* check if we have a portable audio player here */
      is_audio_player = g_udev_device_has_property (context->device, 
                                                    "ID_MEDIA_PLAYER");

      /* check if we have an iPod */
      is_ipod = g_str_has_prefix (g_udev_device_get_property (context->device, 
                                                              "ID_MODEL"), "iPod");
      if (is_ipod)
        {
          /* determine the mount point path */
          mount_point = g_mount_get_root (mount);
          mount_path = g_file_get_path (mount_point);
          g_object_unref (mount_point);

          /* build dcim path */
          path_dcim = g_build_filename (mount_path, "dcim", NULL);
          g_free (mount_path);

          /* check if the iPod has photos */
          if (!g_file_test (path_dcim, G_FILE_TEST_IS_DIR))
            {
              /* no photos */
              g_free (path_dcim);
              path_dcim = NULL;
            }
        }

      autoipod_command = xfconf_channel_get_string (context->channel,
                                                    "/autoipod/command", NULL);
      autophoto_command = xfconf_channel_get_string (context->channel,
                                                     "/autophoto/command", NULL);

      /* check if autophoto command is specified, otherwise we cannot handle the photos 
       * on the iPod anyway */
      if (autophoto_command == NULL || *autophoto_command == '\0')
        {
          g_free (path_dcim);
          path_dcim = NULL;
        }

      /* iPods can carry music and photos... */
      if (path_dcim != NULL)
        {
          /* ...so we need to prompt what to do */
          response = tvm_prompt (context, "multimedia-player", _("Photos and Music"),
                                 _("Photos were found on your portable music player"),
                                 _("Would you like to import the photos or manage the "
                                   "music?"),
                                 _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                 _("Import _Photos"), TVM_RESPONSE_PHOTOS,
                                 _("Manage _Music"), TVM_RESPONSE_MUSIC,
                                 NULL);
        }
      else if (is_audio_player || is_ipod)
        {
          response = TVM_RESPONSE_MUSIC;
        }

      /* check what to do */
      if (response == TVM_RESPONSE_MUSIC)
        {
          /* run the preferred application to manage music players */
          result = tvm_run_command (context, mount, autoipod_command, error);
        }
      else if (response == TVM_RESPONSE_PHOTOS)
        {
          /* run the preferred application to manage photos */
          result = tvm_run_command (context, mount, autophoto_command, error);
        }
      else if (path_dcim != NULL)
        {
          /* when the user has decided to ignore photos/music, we don't want
           * to ask him again in autophoto and we don't want to mount the 
           * device either... I guess? */
          result = TRUE;
        }

      g_free (autophoto_command);
      g_free (autoipod_command);
      g_free (path_dcim);
    }

  return result;
}



static gboolean
tvm_block_device_autophoto (TvmContext *context,
                            GMount     *mount,
                            GError    **error)
{
  gboolean autophoto;
  gboolean result = FALSE;
  gchar   *autophoto_command;
  gint     response;

  g_return_val_if_fail (context != NULL, FALSE);
  g_return_val_if_fail (G_IS_MOUNT (mount), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* check if autophoto support is enabled */
  autophoto = xfconf_channel_get_bool (context->channel, "/autophoto/enabled", FALSE);
  if (autophoto)
    {
      autophoto_command = xfconf_channel_get_string (context->channel, 
                                                     "/autophoto/command", NULL);
      if (autophoto_command != NULL && *autophoto_command != '\0')
        {
          /* check if we have any photos on the volume */
          if (tvm_file_test (mount, "dcim", G_FILE_TEST_IS_DIR))
            {
              /* ask the user to import photos */
              response = tvm_prompt (context, "camera-photo", _("Photo Import"),
                                     _("A photo card has been detected"),
                                     _("There are photos on the card. Would you like to "
                                       "add these photos to your album?"),
                                     _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                     _("Import _Photos"), TVM_RESPONSE_PHOTOS,
                                     NULL);

              if (response == TVM_RESPONSE_PHOTOS)
                {
                  /* run the preferred photo application */
                  result = tvm_run_command (context, mount, autophoto_command, error);
                }
              else
                {
                  /* pretend that we handled the device */
                  result = TRUE;
                }
            }
        }
          
      g_free (autophoto_command);
    }

  return result;
}



static gboolean
tvm_block_device_autorun (TvmContext *context,
                          GMount     *mount,
                          GError    **error)
{
  struct stat statb_mount_point;
  struct stat statb_autoopen;
  gboolean    autoopen;
  gboolean    autoplay;
  gboolean    autorun;
  gboolean    result = FALSE;
  GError     *err = NULL;
  GFile      *mount_point;
  gchar     **argv;
  gchar      *autoplay_command;
  gchar      *message;
  gchar      *mount_path;
  gchar      *path_autoopen;
  gchar      *wine;
  gchar       line[1024];
  guint       n;
  FILE       *fp;
  gint        response;

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

  /* check if autorun is enabled */
  autorun = xfconf_channel_get_bool (context->channel, "/autorun/enabled", FALSE);
  if (autorun)
    {
      /* Autostart files according to the Desktop Application Autostart
       * Specification */
      static const gchar *autorun_files[] = { ".autorun", "autorun", "autorun.sh" };
      for (n = 0; n < G_N_ELEMENTS (autorun_files); ++n) 
        {
          /* check if one of the autorun files is present and executable */
          if (tvm_file_test (mount, autorun_files[n], G_FILE_TEST_IS_EXECUTABLE)
              && tvm_file_test (mount, autorun_files[n], G_FILE_TEST_IS_REGULAR))
            {
              /* prompt the user to execute the file */
              message = g_strdup_printf (_("Would you like to allow \"%s\" to run?"),
                                         autorun_files[n]);
              response = tvm_prompt (context, "gnome-fs-executable", 
                                     _("Auto-Run Confirmation"),
                                     _("Auto-Run capability detected"), message,
                                     _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                     _("_Allow Auto-Run"), TVM_RESPONSE_AUTORUN,
                                     NULL);
              g_free (message);

              /* check if we should autorun */
              if (response == TVM_RESPONSE_AUTORUN)
                {
                  /* determine the mount point as a string */
                  mount_point = g_mount_get_root (mount);
                  mount_path = g_file_get_path (mount_point);
                  g_object_unref (mount_point);

                  /* prepare argv to launch the autorun file */
                  argv = g_new0 (gchar *, 2);
                  argv[0] = g_build_filename (mount_path, autorun_files[n], NULL);
                  argv[1] = NULL;

                  /* try to launch the autorun file */
                  result = g_spawn_async (mount_path, argv, NULL, 0, NULL, NULL, NULL,
                                          &err);
                  
                  /* free strings */
                  g_strfreev (argv);
                  g_free (mount_path);

                  if (err != NULL)
                    g_propagate_error (error, err);

                  return result;
                }
            }
        }

      /* check if wine is present */
      wine = g_find_program_in_path ("wine");
      if (wine != NULL)
        {
          /* check if we have an autorun.exe file */
          if (tvm_file_test (mount, "autorun.exe", G_FILE_TEST_IS_REGULAR))
            {
              /* prompt the user to execute this file */
              message = g_strdup_printf (_("Would you like to allow \"%s\" to run?"),
                                         "autorun.exe");
              response = tvm_prompt (context, "gnome-fs-executable", 
                                     _("Auto-Run Confirmation"),
                                     _("Auto-Run capability detected"), message,
                                     _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                     _("_Allow Auto-Run"), TVM_RESPONSE_AUTORUN,
                                     NULL);
              g_free (message);

              /* check if we should run autogen.exe */
              if (response == TVM_RESPONSE_AUTORUN)
                {
                  /* determine the mount point as a string */
                  mount_point = g_mount_get_root (mount);
                  mount_path = g_file_get_path (mount_point);
                  g_object_unref (mount_point);

                  /* prepare argv to launch the autorun file */
                  argv = g_new0 (gchar *, 3);
                  argv[0] = g_strdup (wine);
                  argv[1] = g_strdup ("autorun.exe");
                  argv[2] = NULL;

                  /* try to launch the autorun file */
                  result = g_spawn_async (mount_path, argv, NULL, 0, NULL, NULL, NULL,
                                          &err);
                  
                  /* free strings */
                  g_strfreev (argv);
                  g_free (mount_path);

                  /* free path to wine */
                  g_free (wine);

                  if (err != NULL)
                    g_propagate_error (error, err);

                  return result;
                }
            }

          /* free path to wine */
          g_free (wine);
        }
    }

 /* check if autoopen support is enabled */
  autoopen = xfconf_channel_get_bool (context->channel, "/autoopen/enabled", FALSE);
  if (autoopen)
    {
      /* "Autoopen Files" (Desktop Application Autostart Specification) */
      static const gchar *autoopen_files[] = { ".autoopen", "autoopen" };
      for (n = 0; n < G_N_ELEMENTS (autoopen_files); ++n)
        {
          /* determine the moint point path */
          mount_point = g_mount_get_root (mount);
          mount_path = g_file_get_path (mount_point);
          g_object_unref (mount_point);

          /* check if one of the autoopen files is present */
          path_autoopen = g_build_filename (mount_path, autoopen_files[n], NULL);
          fp = fopen (path_autoopen, "r");
          g_free (path_autoopen);

          /* check if the file could be opened */
          if (G_UNLIKELY (fp != NULL))
            {
              /* read the first line of the file (MUST NOT be an absolute path) */
              if (fgets (line, sizeof (line), fp) != NULL && !g_path_is_absolute (line))
                {
                  /* determine the absolute path of the file */
                  path_autoopen = g_build_filename (mount_path, g_strstrip (line), NULL);

                  /* the file must exist on exactly this device */
                  if (stat (mount_path, &statb_mount_point) == 0 
                      && stat (path_autoopen, &statb_autoopen) == 0
                      && S_ISREG (statb_autoopen.st_mode) 
                      && (statb_autoopen.st_mode & 0111) == 0
                      && (statb_mount_point.st_dev == statb_autoopen.st_dev))
                    {
                      /* prompt the user whether to autoopen this file */
                      message = g_strdup_printf (_("Would you like to open \"%s\"?"), 
                                                 autoopen_files[n]);
                      response = tvm_prompt (context, "gnome-fs-executable", 
                                             _("Auto-Open Confirmation"),
                                             _("Auto-Open capability detected"), message,
                                             _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                             _("_Open"), TVM_RESPONSE_AUTORUN,
                                             NULL);
                      g_free (message);

                      /* check if we should autoopen */
                      if (response == TVM_RESPONSE_AUTORUN)
                        {
                          /* prepare the command to autoopen */
                          argv = g_new (gchar *, 3);
                          argv[0] = g_strdup ("Thunar");
                          argv[1] = path_autoopen;
                          argv[2] = NULL;

                          /* let Thunar open the file */
                          result = g_spawn_async (mount_path, argv, NULL, 0, NULL, NULL, 
                                                  NULL, &err);

                          /* cleanup */
                          g_free (path_autoopen);
                          fclose (fp);
          
                          /* free the mount point path */
                          g_free (mount_path);

                          if (err != NULL)
                            g_propagate_error (error, err);

                          return result;
                        }
                    }

                  /* cleanup */
                  g_free (path_autoopen);
                }

              /* close the file handle */
              fclose (fp);
            }

          /* free the mount point path */
          g_free (mount_path);
        }
    }

  return FALSE;
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
  gboolean     success = FALSE;
  GError      *err = NULL;
  guint        n;
#ifdef HAVE_LIBNOTIFY
  const gchar *summary;
  const gchar *icon;
  const gchar *volume_name;
  gboolean     is_cdrom;
  gboolean     is_dvd;
  gchar       *message;
  gchar       *decoded_name;
#endif


  g_return_if_fail (context != NULL);
  g_return_if_fail (G_IS_MOUNT (mount));
  g_return_if_fail (error == NULL || *error == NULL);

#ifdef HAVE_LIBNOTIFY
  /* distinguish between CDs and DVDs */
  is_cdrom = g_udev_device_get_property_as_boolean (context->device, "ID_CDROM_MEDIA_CD");
  is_dvd = g_udev_device_get_property_as_boolean (context->device, "ID_CDROM_MEDIA_DVD");
  
  if (is_cdrom || is_dvd)
    {
      /* generate notification info */
      icon = "drive-optical";
      summary = is_cdrom ? _("CD mounted") : _("DVD mounted");
      message = g_strdup (is_cdrom 
                          ? _("The CD was mounted automatically") 
                          : _("The DVD was mounted automatically"));
    }
  else
    {
      /* fetch the volume name */
      volume_name = g_udev_device_get_property (context->device, "ID_FS_LABEL_ENC");
      decoded_name = tvm_notify_decode (volume_name);
  
      /* generate notification info */
      icon = "drive-removable-media";
      summary = _("Volume mounted");
      if (decoded_name != NULL)
        {
          message = g_strdup_printf (_("The volume \"%s\" was mounted automatically"), 
                                     decoded_name);
        }
      else
        {
          message = g_strdup_printf (_("The inserted volume was mounted automatically"));
        }

      g_free (decoded_name);
    }

  /* display the notification */
  tvm_notify (icon, summary, message);

  /* clean up */
  g_free (message);
#endif

  /* try block device handlers (iPod, cameras etc.) until one succeeds */
  for (n = 0; !success && err == NULL && n < G_N_ELEMENTS (block_device_handlers); ++n)
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
  gboolean     automount;
  gboolean     autoplay;
  guint64      audio_tracks;
  guint64      data_tracks;
  GError      *error = NULL;
  gint         response;

  g_return_if_fail (context != NULL);

  /* collect general device information */
  devtype = g_udev_device_get_property (context->device, "DEVTYPE");
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
          data_tracks =
            g_udev_device_get_property_as_uint64 (context->device,
                                                  "ID_CDROM_MEDIA_TRACK_COUNT_DATA");
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
          else if (audio_tracks > 0 && data_tracks > 0)
            {
              /* check if both autoplay and automounting of CDs/DVDs is enabled */
              automount = xfconf_channel_get_bool (context->channel, 
                                                   "/automount-media/enabled", FALSE);
              autoplay = xfconf_channel_get_bool (context->channel, 
                                                  "/autoplay-audio-cds/enabled", FALSE);
              if (automount && autoplay)
                {
                  /* ask what to do with the mixed audio/data disc */
                  response = tvm_prompt (context, "gnome-dev-cdrom-audio",
                                         _("Audio/Data CD"),
                                         _("The CD in the drive contains both music "
                                           "and files"),
                                         _("Would you like to listen to music or "
                                           "browse the files?"),
                                         _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                         _("_Browse Files"), TVM_RESPONSE_BROWSE,
                                         _("_Play CD"), TVM_RESPONSE_PLAY, 
                                         NULL);

                  switch (response)
                    {
                    case TVM_RESPONSE_PLAY:
                      goto autoplay_disc;
                      break;

                    case TVM_RESPONSE_BROWSE:
                      goto automount_disc;
                      break;

                    default:
                      /* finish processing the device */
                      tvm_device_handler_finished (context);
                      break;
                    }
                }
              else if (automount)
                {
                  /* just mount the disc automatically */
                  goto automount_disc;
                }
              else if (autoplay)
                {
                  /* just play the disc automatically */
                  goto autoplay_disc;
                }
              else
                {
                  /* finish processing the device */
                  tvm_device_handler_finished (context);
                }
            }
          else if (audio_tracks > 0)
            {
autoplay_disc:
              /* open the audio CD in the favorite CD player */
              tvm_run_cd_player (context, context->error);

              /* finish processing the device */
              tvm_device_handler_finished (context);
            }
          else if (data_tracks > 0)
            {
automount_disc:
              /* check if automounting media is enabled */
              automount = xfconf_channel_get_bool (context->channel, 
                                                   "/automount-media/enabled", FALSE);
              if (automount)
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
      else
        {
          /* finish processing the device */
          tvm_device_handler_finished (context);
        }
    }
  else if (is_partition || is_volume)
    {
      /* check if automounting drives is enabled */
      automount = xfconf_channel_get_bool (context->channel, "/automount-drives/enabled",
                                           FALSE);
      if (automount)
        {
          /* mount the partition and continue with inspecting its contents */
          tvm_block_device_mount (context);
        }
      else
        {
          /* finish processing the device */
          tvm_device_handler_finished (context);
        }
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
