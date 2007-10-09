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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <dbus/dbus-glib-lowlevel.h>

#include <thunar-volman/tvm-block-device.h>
#include <thunar-volman/tvm-prompt.h>
#include <thunar-volman/tvm-run.h>



static gboolean tvm_file_test               (const gchar    *directory,
                                             const gchar    *filename,
                                             GFileTest       test);
static gboolean tvm_block_device_autoipod   (TvmPreferences *preferences,
                                             LibHalContext  *context,
                                             const gchar    *udi,
                                             const gchar    *device_file,
                                             const gchar    *mount_point,
                                             GError        **error);
static gboolean tvm_block_device_autophoto  (TvmPreferences *preferences,
                                             LibHalContext  *context,
                                             const gchar    *udi,
                                             const gchar    *device_file,
                                             const gchar    *mount_point,
                                             GError        **error);
static gboolean tvm_block_device_autorun    (TvmPreferences *preferences,
                                             LibHalContext  *context,
                                             const gchar    *udi,
                                             const gchar    *device_file,
                                             const gchar    *mount_point,
                                             GError        **error);
static gboolean tvm_block_device_mount      (TvmPreferences *preferences,
                                             LibHalContext  *context,
                                             const gchar    *udi,
                                             GError        **error);
static gboolean tvm_block_device_mounted    (TvmPreferences *preferences,
                                             LibHalContext  *context,
                                             const gchar    *udi,
                                             const gchar    *device_file,
                                             const gchar    *mount_point,
                                             GError        **error);



static gboolean
tvm_file_test (const gchar *directory,
               const gchar *filename,
               GFileTest    test)
{
  const gchar *name;
  gboolean     result = FALSE;
  gchar       *path;
  GDir        *dp;

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

  return result;
}



static gboolean
tvm_block_device_autoipod (TvmPreferences *preferences,
                           LibHalContext  *context,
                           const gchar    *udi,
                           const gchar    *device_file,
                           const gchar    *mount_point,
                           GError        **error)
{
  gboolean result = FALSE;
  gboolean autoipod;
  gchar   *autoipod_command;
  gchar   *autophoto_command;
  gchar   *storage_udi;
  gchar   *path_dcim = NULL;
  gchar   *product;
  gint     response;

  /* check if music players should be handled automatically */
  g_object_get (G_OBJECT (preferences), "autoipod", &autoipod, "autoipod-command", &autoipod_command, NULL);
  if (G_LIKELY (autoipod && autoipod_command != NULL && *autoipod_command != '\0'))
    {
      /* determine the storage device UDI */
      storage_udi = libhal_device_get_property_string (context, udi, "block.storage_device", NULL);
      if (G_LIKELY (storage_udi != NULL))
        {
          /* check if we have a portable audio player here */
          if (libhal_device_query_capability (context, storage_udi, "portable_audio_player", NULL))
            {
              /* check if we have an iPod here */
              product = libhal_device_get_property_string (context, storage_udi, "info.product", NULL);
              if (product != NULL && strcmp (product, "iPod") != 0)
                {
                  /* an iPod may have photos */
                  path_dcim = g_build_filename (mount_point, "dcim", NULL);
                  if (!g_file_test (path_dcim, G_FILE_TEST_IS_DIR))
                    {
                      /* no photos */
                      g_free (path_dcim);
                      path_dcim = NULL;
                    }
                  else
                    {
                      /* add the "content.photos" capability to this device */
                      libhal_device_add_capability (context, udi, "content.photos", NULL);
                    }
                }
              libhal_free_string (product);
            }

          /* check if autophoto command is specified, else we cannot handle the photos on the iPod anyway */
          g_object_get (G_OBJECT (preferences), "autophoto-command", &autophoto_command, NULL);
          if (G_UNLIKELY (autophoto_command == NULL || *autophoto_command == '\0'))
            {
              /* cannot handle photos */
              g_free (path_dcim);
              path_dcim = NULL;
            }

          /* iPods can carry both music and photos... */
          if (G_LIKELY (path_dcim != NULL))
            {
              /* ...so we need to prompt what to do */
              response = tvm_prompt (context, udi, "gnome-dev-ipod", _("Photos and Music"),
                                     _("Photos were found on your portable music player."),
                                     _("Would you like to import the photos or manage the music?"),
                                     _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                     _("Import _Photos"), TVM_RESPONSE_PHOTOS,
                                     _("Manage _Music"), TVM_RESPONSE_MUSIC,
                                     NULL);
            }
          else
            {
              /* no photos, so we can manage only music */
              response = TVM_RESPONSE_MUSIC;
            }

          /* check what to do */
          if (response == TVM_RESPONSE_MUSIC)
            {
              /* run the preferred application to manage music players */
              result = tvm_run_command (context, udi, autoipod_command, device_file, mount_point, error);
            }
          else if (response == TVM_RESPONSE_PHOTOS)
            {
              /* run the preferred application to manage photos */
              result = tvm_run_command (context, udi, autophoto_command, device_file, mount_point, error);
            }

          /* cleanup */
          libhal_free_string (storage_udi);
          g_free (autophoto_command);
          g_free (path_dcim);
        }
    }

  /* cleanup */
  g_free (autoipod_command);

  return result;
}



static gboolean
tvm_block_device_autophoto (TvmPreferences *preferences,
                            LibHalContext  *context,
                            const gchar    *udi,
                            const gchar    *device_file,
                            const gchar    *mount_point,
                            GError        **error)
{
  gboolean result = FALSE;
  gboolean autophoto;
  gchar   *autophoto_command;
  gint     response;

  /* check autophoto support is enabled */
  g_object_get (G_OBJECT (preferences), "autophoto", &autophoto, "autophoto-command", &autophoto_command, NULL);
  if (G_LIKELY (autophoto && autophoto_command != NULL && *autophoto_command != '\0'))
    {
      /* check if we have photos on the volume */
      if (tvm_file_test (mount_point, "dcim", G_FILE_TEST_IS_DIR))
        {
          /* add the "content.photos" capability to this device */
          libhal_device_add_capability (context, udi, "content.photos", NULL);

          /* ask the user whether to import photos */
          response = tvm_prompt (context, udi, "camera-photo", _("Photo Import"),
                                 _("A photo card has been detected."),
                                 _("There are photos on the card. Would you like to add these photos to your album?"),
                                 _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                 _("Import _Photos"), TVM_RESPONSE_PHOTOS,
                                 NULL);
          if (G_LIKELY (response == TVM_RESPONSE_PHOTOS))
            {
              /* run the preferred application to manage photos */
              result = tvm_run_command (context, udi, autophoto_command, device_file, mount_point, error);
            }
          else
            {
              /* pretend that we handled the device */
              result = TRUE;
            }
        }
    }
  g_free (autophoto_command);

  return result;
}



static gboolean
tvm_block_device_autorun (TvmPreferences *preferences,
                          LibHalContext  *context,
                          const gchar    *udi,
                          const gchar    *device_file,
                          const gchar    *mount_point,
                          GError        **error)
{
  struct stat statb_mount_point;
  struct stat statb_autoopen;
  gboolean    result;
  gboolean    autoopen;
  gboolean    autoplay;
  gboolean    autorun;
  gchar      *autoplay_command;
  gchar      *path_autoopen;
  gchar       line[1024];
  gchar      *message;
  gchar      *wine;
  gchar     **argv;
  FILE       *fp;
  gint        response;
  gint        n;

  /* check if autoplay video CDs/DVDs is enabled */
  g_object_get (G_OBJECT (preferences), "autoplay-video-cd", &autoplay, "autoplay-video-cd-command", &autoplay_command, NULL);
  if (G_LIKELY (autoplay))
    {
      /* check if we have a video CD or video DVD here */
      if (tvm_file_test (mount_point, "vcd", G_FILE_TEST_IS_DIR) || tvm_file_test (mount_point, "video_ts", G_FILE_TEST_IS_DIR))
        {
          /* try to spawn the preferred video CD/DVD-Player */
          result = tvm_run_command (context, udi, autoplay_command, device_file, mount_point, error);
          g_free (autoplay_command);
          return result;
        }
    }
  g_free (autoplay_command);

  /* check if autorun support is enabled */
  g_object_get (G_OBJECT (preferences), "autorun", &autorun, NULL);
  if (G_LIKELY (autorun))
    {
      /* "Autostart Files" (Desktop Application Autostart Specification) */
      static const gchar *AUTORUN[] = { ".autorun", "autorun", "autorun.sh" };
      for (n = 0; n < G_N_ELEMENTS (AUTORUN); ++n)
        {
          /* check if one of the autorun files is present and executable */
          if (tvm_file_test (mount_point, AUTORUN[n], G_FILE_TEST_IS_EXECUTABLE)
              && tvm_file_test (mount_point, AUTORUN[n], G_FILE_TEST_IS_REGULAR))
            {
              /* prompt the user whether to execute this file */
              message = g_strdup_printf (_("Would you like to allow \"%s\" to run?"), AUTORUN[n]);
              response = tvm_prompt (context, udi, "gnome-fs-executable", _("Auto-Run Confirmation"),
                                     _("Auto-Run capability detected"), message,
                                     _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                     _("_Allow Auto-Run"), TVM_RESPONSE_AUTORUN,
                                     NULL);
              g_free (message);

              /* check if we should autorun */
              if (response == TVM_RESPONSE_AUTORUN)
                {
                  /* prepare argv to launch autorun file */
                  argv = g_new (gchar *, 2);
                  argv[0] = g_build_filename (mount_point, AUTORUN[n], NULL);
                  argv[1] = NULL;

                  /* try to launch the autorun file */
                  result = g_spawn_async (mount_point, argv, NULL, 0, NULL, NULL, NULL, error);

                  /* cleanup */
                  g_strfreev (argv);

                  /* outa here */
                  return result;
                }
            }
        }

      /* check if wine is present */
      wine = g_find_program_in_path ("wine");
      if (G_UNLIKELY (wine != NULL))
        {
          /* check if we have an autorun.exe file */
          if (tvm_file_test (mount_point, "autorun.exe", G_FILE_TEST_IS_REGULAR))
            {
              /* prompt the user whether to execute this file */
              message = g_strdup_printf (_("Would you like to allow \"%s\" to run?"), "autorun.exe");
              response = tvm_prompt (context, udi, "gnome-fs-executable", _("Auto-Run Confirmation"),
                                     _("Auto-Run capability detected"), message,
                                     _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                     _("_Allow Auto-Run"), TVM_RESPONSE_AUTORUN,
                                     NULL);
              g_free (message);

              /* check if we should autorun */
              if (response == TVM_RESPONSE_AUTORUN)
                {
                  /* prepare argv to launch autorun.exe file */
                  argv = g_new (gchar *, 3);
                  argv[0] = wine;
                  argv[1] = g_strdup ("autorun.exe");
                  argv[2] = NULL;

                  /* try to launch the autorun.exe file via wine */
                  result = g_spawn_async (mount_point, argv, NULL, 0, NULL, NULL, NULL, error);

                  /* cleanup */
                  g_strfreev (argv);

                  /* outa here */
                  return result;
                }
            }
        }
      g_free (wine);
    }

  /* check if autoopen support is enabled */
  g_object_get (G_OBJECT (preferences), "autoopen", &autoopen, NULL);
  if (G_LIKELY (autoopen))
    {
      /* "Autoopen Files" (Desktop Application Autostart Specification) */
      static const gchar *AUTOOPEN[] = { ".autoopen", "autoopen" };
      for (n = 0; n < G_N_ELEMENTS (AUTOOPEN); ++n)
        {
          /* check if one of the autoopen files is present */
          path_autoopen = g_build_filename (mount_point, AUTOOPEN[n], NULL);
          fp = fopen (path_autoopen, "r");
          g_free (path_autoopen);

          /* check if the file could be opened */
          if (G_UNLIKELY (fp != NULL))
            {
              /* read the first line of the file (MUST NOT be an absolute path) */
              if (fgets (line, sizeof (line), fp) != NULL && !g_path_is_absolute (line))
                {
                  /* determine the absolute path of the file */
                  path_autoopen = g_build_filename (mount_point, line, NULL);

                  /* the file must exist on exactly this device */
                  if (stat (mount_point, &statb_mount_point) == 0 && stat (path_autoopen, &statb_autoopen) == 0
                      && S_ISREG (statb_autoopen.st_mode) && (statb_autoopen.st_mode & 0111) == 0
                      && (statb_mount_point.st_dev == statb_autoopen.st_dev))
                    {
                      /* prompt the user whether to autoopen this file */
                      message = g_strdup_printf (_("Would you like to open \"%s\"?"), AUTOOPEN[n]);
                      response = tvm_prompt (context, udi, "gnome-fs-executable", _("Auto-Open Confirmation"),
                                             _("Auto-Open capability detected"), message,
                                             _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                             _("_Open"), TVM_RESPONSE_AUTOOPEN,
                                             NULL);
                      g_free (message);

                      /* check if we should autoopen */
                      if (response == TVM_RESPONSE_AUTOOPEN)
                        {
                          /* prepare the command to autoopen */
                          argv = g_new (gchar *, 3);
                          argv[0] = g_strdup ("Thunar");
                          argv[1] = path_autoopen;
                          argv[2] = NULL;

                          /* let Thunar open the file */
                          result = g_spawn_async (mount_point, argv, NULL, 0, NULL, NULL, NULL, error);

                          /* cleanup */
                          g_free (path_autoopen);
                          fclose (fp);
                          return result;
                        }
                    }

                  /* cleanup */
                  g_free (path_autoopen);
                }

              /* close the file handle */
              fclose (fp);
            }
        }
    }

  /* not handled */
  return FALSE;
}



static gboolean
tvm_block_device_mount (TvmPreferences *preferences,
                        LibHalContext  *context,
                        const gchar    *udi,
                        GError        **error)
{
  DBusError derror;
  gboolean  result = FALSE;
  GSList   *mount_points;
  gchar    *device_file;
  gchar    *argv[4];
  gint      status;

  /* check if we should ignore the volume, if so, pretend that we succeed */
  if (libhal_device_get_property_bool (context, udi, "volume.ignore", NULL))
    return TRUE;

  /* generate the command to mount the device */
  argv[0] = (gchar *) "exo-mount";
  argv[1] = (gchar *) "--hal-udi";
  argv[2] = (gchar *) udi;
  argv[3] = NULL;

  /* let exo-mount mount the device */
  if (!g_spawn_sync (NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, &status, error))
    {
      /* failed to spawn the exo-mount command */
      return FALSE;
    }
  else if (!WIFEXITED (status) || WEXITSTATUS (status) != 0)
    {
      /* exo-mount failed, but already displayed an error */
      return TRUE;
    }

  /* initalize D-Bus error */
  dbus_error_init (&derror);

  /* successfully mounted the device, determine the device file */
  device_file = libhal_device_get_property_string (context, udi, "block.device", &derror);
  if (G_UNLIKELY (device_file == NULL))
    {
      /* propagate the error */
      dbus_set_g_error (error, &derror);
      return FALSE;
    }

  /* determine the active mount point(s) for the device from the kernel */
  mount_points = exo_mount_point_list_matched (EXO_MOUNT_POINT_MATCH_ACTIVE | EXO_MOUNT_POINT_MATCH_DEVICE, device_file, NULL, NULL, error);
  if (G_LIKELY (mount_points != NULL))
    {
      /* try to handled the mounted volume */
      result = tvm_block_device_mounted (preferences, context, udi, device_file, ((ExoMountPoint *) mount_points->data)->folder, error);

      /* release the mount points */
      g_slist_foreach (mount_points, (GFunc) exo_mount_point_free, NULL);
      g_slist_free (mount_points);
    }

  /* release the device file */
  libhal_free_string (device_file);

  return result;
}




static gboolean
tvm_block_device_mounted (TvmPreferences *preferences,
                          LibHalContext  *context,
                          const gchar    *udi,
                          const gchar    *device_file,
                          const gchar    *mount_point,
                          GError        **error)
{
  gboolean autobrowse;
  gboolean result;
  GError  *err = NULL;

  /* check if we have a portable media player here */
  result = tvm_block_device_autoipod (preferences, context, udi, device_file, mount_point, &err);
  if (G_LIKELY (!result && err == NULL))
    {
      /* check if we have a digital photo camera here */
      result = tvm_block_device_autophoto (preferences, context, udi, device_file, mount_point, &err);
      if (G_LIKELY (!result && err == NULL))
        {
          /* try autorun (video CD/DVD, autoopen, etc.) first */
          result = tvm_block_device_autorun (preferences, context, udi, device_file, mount_point, &err);
          if (G_LIKELY (!result && err == NULL))
            {
              /* check if we should autobrowse the mount point folder */
              g_object_get (G_OBJECT (preferences), "autobrowse", &autobrowse, NULL);
              if (G_LIKELY (autobrowse))
                {
                  /* open the mount point folder in Thunar */
                  result = tvm_run_command (context, udi, "Thunar %m", device_file, mount_point, &err);
                }
            }
        }
    }

  /* check if we need to propagate an error */
  if (G_UNLIKELY (err != NULL))
    {
      /* propagate the error */
      g_propagate_error (error, err);
      result = FALSE;
    }

  return result;
}



/**
 * tvm_block_device_added:
 * @preferences : a #TvmPreferences.
 * @context     : a #LibHalContext.
 * @udi         : the HAL device UDI of the newly added block device.
 * @capability  : the capability, which caused this handler to be run.
 * @error       : return location for errors or %NULL.
 *
 * See #TvmDeviceCallback for further information.
 *
 * Return value: %TRUE if handled, %FALSE if not handled or an
 *               unrecoverable error occurred.
 **/
gboolean
tvm_block_device_added (TvmPreferences *preferences,
                        LibHalContext  *context,
                        const gchar    *udi,
                        const gchar    *capability,
                        GError        **error)
{
  DBusError derror;
  gboolean  disc_has_audio;
  gboolean  disc_has_data;
  gboolean  automount;
  gboolean  autoplay;
  gboolean  is_cdrom;
  gboolean  has_filesystem;
  gchar    *storage_udi;
  gchar    *drive_type;
  gchar    *fsusage;
  gint      response;

  g_return_val_if_fail (exo_hal_udi_validate (udi, -1, NULL), FALSE);
  g_return_val_if_fail (TVM_IS_PREFERENCES (preferences), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (capability != NULL, FALSE);
  g_return_val_if_fail (context != NULL, FALSE);

  /* initialize D-Bus error */
  dbus_error_init (&derror);

  /* verify that we have a mountable volume here */
  if (!libhal_device_get_property_bool (context, udi, "block.is_volume", &derror))
    {
err0: /* check if we have an error to propagate */
      if (dbus_error_is_set (&derror))
        {
          /* propagate the error */
          dbus_set_g_error (error, &derror);
          dbus_error_free (&derror);
        }

      return FALSE;
    }

  /* determine the HAL UDI of the backing storage device */
  storage_udi = libhal_device_get_property_string (context, udi, "block.storage_device", &derror);
  if (G_UNLIKELY (storage_udi == NULL))
    goto err0;

  /* if the partition_table_changed flag is set, we don't want to mount as a partitioning tool might be modifying this device */
  if (libhal_device_get_property_bool (context, storage_udi, "storage.partition_table_changed", NULL))
    {
err1: libhal_free_string (storage_udi);
      goto err0;
    }

  /* check if this device supports removable media */
  if (libhal_device_get_property_bool (context, storage_udi, "storage.removable", NULL))
    {
      /* check if the device is locked */
      if (libhal_device_get_property_bool (context, storage_udi, "info.locked", NULL))
        {
          /* pretend that we handled the device */
          libhal_free_string (storage_udi);
          return TRUE;
        }

      /* determine the drive type */
      drive_type = libhal_device_get_property_string (context, storage_udi, "storage.drive_type", &derror);
      if (G_UNLIKELY (drive_type == NULL))
        goto err1;

      /* check if we have a CD-ROM here */
      is_cdrom = (strcmp (drive_type, "cdrom") == 0);

      /* we don't need the storage UDI any more */
      libhal_free_string (storage_udi);

      /* free the drive type */
      libhal_free_string (drive_type);

      /* CD-ROMs deserve special handling */
      if (G_LIKELY (is_cdrom))
        {
          /* check for blank discs */
          if (libhal_device_get_property_bool (context, udi, "volume.disc.is_blank", NULL))
            {
              /* run the preferred CD-Burner application */
              return tvm_run_cdburner (preferences, context, udi, error);
            }
          else
            {
              /* check if we have DATA/AUDIO tracks */
              disc_has_audio = libhal_device_get_property_bool (context, udi, "volume.disc.has_audio", NULL);
              disc_has_data = libhal_device_get_property_bool (context, udi, "volume.disc.has_data", NULL);
              if (G_UNLIKELY (disc_has_audio && disc_has_data))
                {
                  /* check if we need to prompt the user */
                  g_object_get (G_OBJECT (preferences), "automount-media", &automount, "autoplay-audio-cd", &autoplay, NULL);
                  if (G_LIKELY (automount && autoplay))
                    {
                      /* ask what do with the mixed audio/data disc */
                      response = tvm_prompt (context, udi, "gnome-dev-cdrom-audio", _("Audio / Data CD"),
                                             _("The CD in the drive contains both music and files."),
	                                           _("Would you like to listen to music or browse the files?"),
                                             _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                             _("_Browse Files"), TVM_RESPONSE_BROWSE,
                                             _("_Play CD"), TVM_RESPONSE_PLAY,
                                             NULL);
                      switch (response)
                        {
                        case TVM_RESPONSE_PLAY:
                          goto autoplay_disc;

                        case TVM_RESPONSE_BROWSE:
                          goto automount_disc;

                        default:
                          break;
                        }
                    }
                  else if (automount)
                    {
                      /* just automount the media */
                      goto automount_disc;
                    }
                  else if (autoplay)
                    {
                      /* just autoplay the disc */
                      goto autoplay_disc;
                    }
                }
              else if (G_LIKELY (disc_has_audio))
                {
autoplay_disc:    /* run the preferred CD-Player application */
                  return tvm_run_cdplayer (preferences, context, udi, error);
                }
              else if (G_LIKELY (disc_has_data))
                {
automount_disc:   /* check if we should automount removable media */
                  g_object_get (G_OBJECT (preferences), "automount-media", &automount, NULL);
                  if (G_LIKELY (automount))
                    {
                      /* try to mount the CD-ROM in the disc */
                      return tvm_block_device_mount (preferences, context, udi, error);
                    }
                }
            }
        }
    }
  else
    {
      /* we don't need the storage UDI any more */
      libhal_free_string (storage_udi);
    }

  /* make sure the volume has a mountable filesystem */
  fsusage = libhal_device_get_property_string (context, udi, "volume.fsusage", NULL);
  has_filesystem = (G_LIKELY (fsusage && strcmp (fsusage, "filesystem") == 0));
  libhal_free_string (fsusage);
  if (G_UNLIKELY (!has_filesystem))
    return FALSE;

  /* check if we should automount drives, otherwise, we're done here */
  g_object_get (G_OBJECT (preferences), "automount-drives", &automount, NULL);
  if (G_UNLIKELY (!automount))
    return FALSE;

  /* try to mount the block device */
  return tvm_block_device_mount (preferences, context, udi, error);
}




