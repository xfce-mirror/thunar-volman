/* $Id$ */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@xfce.org>
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

#include <thunar-vfs/thunar-vfs.h>

#include <thunar-volman/tvm-preferences.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_AUTOMOUNT_DRIVES,
  PROP_AUTOMOUNT_MEDIA,
  PROP_AUTOBROWSE,
  PROP_AUTORUN,
  PROP_AUTOOPEN,
  PROP_AUTOBURN,
  PROP_AUTOBURN_DATA_COMMAND,
  PROP_AUTOBURN_AUDIO_COMMAND,
  PROP_AUTOPLAY_AUDIO_CD,
  PROP_AUTOPLAY_AUDIO_CD_COMMAND,
  PROP_AUTOPLAY_VIDEO_CD,
  PROP_AUTOPLAY_VIDEO_CD_COMMAND,
  PROP_AUTOIPOD,
  PROP_AUTOIPOD_COMMAND,
  PROP_AUTOPHOTO,
  PROP_AUTOPHOTO_COMMAND,
  PROP_AUTOKEYBOARD,
  PROP_AUTOKEYBOARD_COMMAND,
  PROP_AUTOMOUSE,
  PROP_AUTOMOUSE_COMMAND,
  PROP_AUTOTABLET,
  PROP_AUTOTABLET_COMMAND,
  N_PROPERTIES,
};



static void     tvm_preferences_class_init         (TvmPreferencesClass    *klass);
static void     tvm_preferences_init               (TvmPreferences         *preferences);
static void     tvm_preferences_dispose            (GObject                *object);
static void     tvm_preferences_finalize           (GObject                *object);
static void     tvm_preferences_get_property       (GObject                *object,
                                                    guint                   prop_id,
                                                    GValue                 *value,
                                                    GParamSpec             *pspec);
static void     tvm_preferences_set_property       (GObject                *object,
                                                    guint                   prop_id,
                                                    const GValue           *value,
                                                    GParamSpec             *pspec);
static void     tvm_preferences_resume_monitor     (TvmPreferences         *preferences);
static void     tvm_preferences_suspend_monitor    (TvmPreferences         *preferences);
static void     tvm_preferences_monitor            (ThunarVfsMonitor       *monitor,
                                                    ThunarVfsMonitorHandle *handle,
                                                    ThunarVfsMonitorEvent   event,
                                                    ThunarVfsPath          *handle_path,
                                                    ThunarVfsPath          *event_path,
                                                    gpointer                user_data);
static void     tvm_preferences_queue_load         (TvmPreferences         *preferences);
static void     tvm_preferences_queue_store        (TvmPreferences         *preferences);
static gboolean tvm_preferences_load_idle          (gpointer                user_data);
static void     tvm_preferences_load_idle_destroy  (gpointer                user_data);
static gboolean tvm_preferences_store_idle         (gpointer                user_data);
static void     tvm_preferences_store_idle_destroy (gpointer                user_data);



struct _TvmPreferencesClass
{
  GObjectClass __parent__;
};

struct _TvmPreferences
{
  GObject                 __parent__;

  ThunarVfsMonitorHandle *handle;
  ThunarVfsMonitor       *monitor;

  GValue                  values[N_PROPERTIES];

  gboolean                loading_in_progress;

  gint                    load_idle_id;
  gint                    store_idle_id;
};



static GObjectClass *tvm_preferences_parent_class;



GType
tvm_preferences_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (TvmPreferencesClass),
        NULL,
        NULL,
        (GClassInitFunc) tvm_preferences_class_init,
        NULL,
        NULL,
        sizeof (TvmPreferences),
        0,
        (GInstanceInitFunc) tvm_preferences_init,
        NULL,
      };

      type = g_type_register_static (G_TYPE_OBJECT, I_("TvmPreferences"), &info, 0);
    }

  return type;
}



static void
transform_string_to_boolean (const GValue *src,
                             GValue       *dst)
{
  g_value_set_boolean (dst, strcmp (g_value_get_string (src), "FALSE") != 0);
}



static void
tvm_preferences_class_init (TvmPreferencesClass *klass)
{
  GObjectClass *gobject_class;

  /* register transformation functions */
  if (!g_value_type_transformable (G_TYPE_STRING, G_TYPE_BOOLEAN))
    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_BOOLEAN, transform_string_to_boolean);

  /* determine the parent type class */
  tvm_preferences_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = tvm_preferences_dispose;
  gobject_class->finalize = tvm_preferences_finalize;
  gobject_class->get_property = tvm_preferences_get_property;
  gobject_class->set_property = tvm_preferences_set_property;

  /**
   * TvmPreferences:automount-drives:
   *
   * Mount removable drives when hot-plugged.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOMOUNT_DRIVES,
                                   g_param_spec_boolean ("automount-drives",
                                                         "automount-drives",
                                                         "automount-drives",
                                                         TRUE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:automount-media:
   *
   * Mount removable media when inserted.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOMOUNT_MEDIA,
                                   g_param_spec_boolean ("automount-media",
                                                         "automount-media",
                                                         "automount-media",
                                                         TRUE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autobrowse:
   *
   * Browse removable media when inserted.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOBROWSE,
                                   g_param_spec_boolean ("autobrowse",
                                                         "autobrowse",
                                                         "autobrowse",
                                                         TRUE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autorun:
   *
   * Auto-run programs on new drives and media.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTORUN,
                                   g_param_spec_boolean ("autorun",
                                                         "autorun",
                                                         "autorun",
                                                         FALSE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autoopen:
   *
   * Auto-open files on new drives and media.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOOPEN,
                                   g_param_spec_boolean ("autoopen",
                                                         "autoopen",
                                                         "autoopen",
                                                         FALSE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autoburn:
   *
   * Burn a CD or DVD if a blank disc is inserted.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOBURN,
                                   g_param_spec_boolean ("autoburn",
                                                         "autoburn",
                                                         "autoburn",
                                                         FALSE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autoburn-data-command:
   *
   * Command to burn a new data CD/DVD.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOBURN_DATA_COMMAND,
                                   g_param_spec_string ("autoburn-data-command",
                                                        "autoburn-data-command",
                                                        "autoburn-data-command",
                                                        "xfburn",
                                                        EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autoburn-audio-command:
   *
   * Command to burn a new audio CD.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOBURN_AUDIO_COMMAND,
                                   g_param_spec_string ("autoburn-audio-command",
                                                        "autoburn-audio-command",
                                                        "autoburn-audio-command",
                                                        "xfburn",
                                                        EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autoplay-audio-cd:
   *
   * Play audio discs when inserted.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOPLAY_AUDIO_CD,
                                   g_param_spec_boolean ("autoplay-audio-cd",
                                                         "autoplay-audio-cd",
                                                         "autoplay-audio-cd",
                                                         FALSE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autoplay-audio-cd-command:
   *
   * Command to run the preferred audio CD player.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOPLAY_AUDIO_CD_COMMAND,
                                   g_param_spec_string ("autoplay-audio-cd-command",
                                                        "autoplay-audio-cd-command",
                                                        "autoplay-audio-cd-command",
                                                        "totem %d",
                                                        EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autoplay-video-cd:
   *
   * Play video CDs and DVDs when inserted.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOPLAY_VIDEO_CD,
                                   g_param_spec_boolean ("autoplay-video-cd",
                                                         "autoplay-video-cd",
                                                         "autoplay-video-cd",
                                                         TRUE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autoplay-video-cd-command:
   *
   * Command to run the preferred video CD/DVD-Player.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOPLAY_VIDEO_CD_COMMAND,
                                   g_param_spec_string ("autoplay-video-cd-command",
                                                        "autoplay-video-cd-command",
                                                        "autoplay-video-cd-command",
                                                        "totem %d",
                                                        EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autoipod:
   *
   * Play music files when portable music player is connected.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOIPOD,
                                   g_param_spec_boolean ("autoipod",
                                                         "autoipod",
                                                         "autoipod",
                                                         FALSE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autoipod-command:
   *
   * Command to run when portable music player is connected.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOIPOD_COMMAND,
                                   g_param_spec_string ("autoipod-command",
                                                        "autoipod-command",
                                                        "autoipod-command",
                                                        "",
                                                        EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autophoto:
   *
   * Import photos when digital camera is connected.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOPHOTO,
                                   g_param_spec_boolean ("autophoto",
                                                         "autophoto",
                                                         "autophoto",
                                                         FALSE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autophoto-command:
   *
   * Command to run when digitcal camera is connected.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOPHOTO_COMMAND,
                                   g_param_spec_string ("autophoto-command",
                                                        "autophoto-command",
                                                        "autophoto-command",
                                                        "",
                                                        EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autokeyboard:
   *
   * Automatically run a program when an USB keyboard is connected.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOKEYBOARD,
                                   g_param_spec_boolean ("autokeyboard",
                                                         "autokeyboard",
                                                         "autokeyboard",
                                                         FALSE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autokeyboard-command:
   *
   * Command to run when an USB keyboard is connected.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOKEYBOARD_COMMAND,
                                   g_param_spec_string ("autokeyboard-command",
                                                        "autokeyboard-command",
                                                        "autokeyboard-command",
                                                        "",
                                                        EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:automouse:
   *
   * Automatically run a program when an USB mouse is connected.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOMOUSE,
                                   g_param_spec_boolean ("automouse",
                                                         "automouse",
                                                         "automouse",
                                                         FALSE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:automouse-command:
   *
   * Command to run when an USB mouse is connected.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOMOUSE_COMMAND,
                                   g_param_spec_string ("automouse-command",
                                                        "automouse-command",
                                                        "automouse-command",
                                                        "",
                                                        EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autotablet:
   *
   * Automatically run a program when a table is connected.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOTABLET,
                                   g_param_spec_boolean ("autotablet",
                                                         "autotablet",
                                                         "autotablet",
                                                         FALSE,
                                                         EXO_PARAM_READWRITE));

  /**
   * TvmPreferences:autotablet-command:
   *
   * Command to run when a table is connected.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_AUTOTABLET_COMMAND,
                                   g_param_spec_string ("autotablet-command",
                                                        "autotablet-command",
                                                        "autotablet-command",
                                                        "",
                                                        EXO_PARAM_READWRITE));
}



static void
tvm_preferences_init (TvmPreferences *preferences)
{
  /* grab a reference on the VFS monitor */
  preferences->monitor = thunar_vfs_monitor_get_default ();

  /* load the settings */
  tvm_preferences_load_idle (preferences);

  /* launch the file monitor */
  tvm_preferences_resume_monitor (preferences);
}



static void
tvm_preferences_dispose (GObject *object)
{
  TvmPreferences *preferences = TVM_PREFERENCES (object);

  /* flush preferences */
  if (G_UNLIKELY (preferences->store_idle_id != 0))
    {
      tvm_preferences_store_idle (preferences);
      g_source_remove (preferences->store_idle_id);
    }

  (*G_OBJECT_CLASS (tvm_preferences_parent_class)->dispose) (object);
}



static void
tvm_preferences_finalize (GObject *object)
{
  TvmPreferences *preferences = TVM_PREFERENCES (object);
  guint           n;

  /* stop any pending load idle source */
  if (G_UNLIKELY (preferences->load_idle_id != 0))
    g_source_remove (preferences->load_idle_id);

  /* stop the file monitor */
  if (G_LIKELY (preferences->monitor != NULL))
    {
      tvm_preferences_suspend_monitor (preferences);
      g_object_unref (G_OBJECT (preferences->monitor));
    }

  /* release the property values */
  for (n = 1; n < N_PROPERTIES; ++n)
    if (G_IS_VALUE (preferences->values + n))
      g_value_unset (preferences->values + n);

  (*G_OBJECT_CLASS (tvm_preferences_parent_class)->finalize) (object);
}



static void
tvm_preferences_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  TvmPreferences *preferences = TVM_PREFERENCES (object);
  GValue         *src;

  src = preferences->values + prop_id;
  if (G_IS_VALUE (src))
    g_value_copy (src, value);
  else
    g_param_value_set_default (pspec, value);
}



static void
tvm_preferences_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  TvmPreferences *preferences = TVM_PREFERENCES (object);
  GValue         *dst;

  dst = preferences->values + prop_id;
  if (G_UNLIKELY (!G_IS_VALUE (dst)))
    g_value_init (dst, pspec->value_type);

  if (g_param_values_cmp (pspec, value, dst) != 0)
    {
      g_value_copy (value, dst);
      tvm_preferences_queue_store (preferences);
    }
}



static void
tvm_preferences_resume_monitor (TvmPreferences *preferences)
{
  ThunarVfsPath *path;
  gchar      *filename;

  /* verify that the monitor is suspended */
  if (G_LIKELY (preferences->handle == NULL))
    {
      /* determine the save location for tvmrc to monitor */
      filename = xfce_resource_save_location (XFCE_RESOURCE_CONFIG, "Thunar/volmanrc", TRUE);
      if (G_LIKELY (filename != NULL))
        {
          /* determine the VFS path for the filename */
          path = thunar_vfs_path_new (filename, NULL);
          if (G_LIKELY (path != NULL))
            {
              /* add the monitor handle for the file */
              preferences->handle = thunar_vfs_monitor_add_file (preferences->monitor, path, tvm_preferences_monitor, preferences);
              thunar_vfs_path_unref (path);
            }

          /* release the filename */
          g_free (filename);
        }
    }
}



static void
tvm_preferences_suspend_monitor (TvmPreferences *preferences)
{
  /* verify that the monitor is active */
  if (G_LIKELY (preferences->handle != NULL))
    {
      /* disconnect the handle from the monitor */
      thunar_vfs_monitor_remove (preferences->monitor, preferences->handle);
      preferences->handle = NULL;
    }
}



static void
tvm_preferences_monitor (ThunarVfsMonitor       *monitor,
                         ThunarVfsMonitorHandle *handle,
                         ThunarVfsMonitorEvent   event,
                         ThunarVfsPath          *handle_path,
                         ThunarVfsPath          *event_path,
                         gpointer                user_data)
{
  TvmPreferences *preferences = TVM_PREFERENCES (user_data);

  g_return_if_fail (TVM_IS_PREFERENCES (preferences));
  g_return_if_fail (THUNAR_VFS_IS_MONITOR (monitor));
  g_return_if_fail (preferences->monitor == monitor);
  g_return_if_fail (preferences->handle == handle);

  /* schedule a reload whenever the file is created/changed */
  if (event == THUNAR_VFS_MONITOR_EVENT_CHANGED || event == THUNAR_VFS_MONITOR_EVENT_CREATED)
    tvm_preferences_queue_load (preferences);
}



static void
tvm_preferences_queue_load (TvmPreferences *preferences)
{
  if (preferences->load_idle_id == 0 && preferences->store_idle_id == 0)
    {
      preferences->load_idle_id = g_idle_add_full (G_PRIORITY_LOW, tvm_preferences_load_idle,
                                                   preferences, tvm_preferences_load_idle_destroy);
    }
}



static void
tvm_preferences_queue_store (TvmPreferences *preferences)
{
  if (preferences->store_idle_id == 0 && !preferences->loading_in_progress)
    {
      preferences->store_idle_id = g_idle_add_full (G_PRIORITY_LOW, tvm_preferences_store_idle,
                                                    preferences, tvm_preferences_store_idle_destroy);
    }
}



static gchar*
property_name_to_option_name (const gchar *property_name)
{
  const gchar *s;
  gboolean     upper = TRUE;
  gchar       *option;
  gchar       *t;

  option = g_new (gchar, strlen (property_name) + 1);
  for (s = property_name, t = option; *s != '\0'; ++s)
    {
      if (*s == '-')
        {
          upper = TRUE;
        }
      else if (upper)
        {
          *t++ = g_ascii_toupper (*s);
          upper = FALSE;
        }
      else
        {
          *t++ = *s;
        }
    }
  *t = '\0';

  return option;
}



static gboolean
tvm_preferences_load_idle (gpointer user_data)
{
  TvmPreferences *preferences = TVM_PREFERENCES (user_data);
  const gchar    *string;
  GParamSpec    **specs;
  GParamSpec     *spec;
  XfceRc         *rc;
  GValue          dst = { 0, };
  GValue          src = { 0, };
  gchar          *option;
  guint           nspecs;
  guint           n;

  rc = xfce_rc_config_open (XFCE_RESOURCE_CONFIG, "Thunar/volmanrc", TRUE);
  if (G_UNLIKELY (rc == NULL))
    {
      g_warning ("Failed to load tvm preferences.");
      return FALSE;
    }

  g_object_freeze_notify (G_OBJECT (preferences));

  xfce_rc_set_group (rc, "Configuration");

  preferences->loading_in_progress = TRUE;

  specs = g_object_class_list_properties (G_OBJECT_GET_CLASS (preferences), &nspecs);
  for (n = 0; n < nspecs; ++n)
    {
      spec = specs[n];

      option = property_name_to_option_name (spec->name);
      string = xfce_rc_read_entry (rc, option, NULL);
      g_free (option);

      if (G_UNLIKELY (string == NULL))
        continue;

      g_value_init (&src, G_TYPE_STRING);
      g_value_set_static_string (&src, string);

      if (spec->value_type == G_TYPE_STRING)
        {
          g_object_set_property (G_OBJECT (preferences), spec->name, &src);
        }
      else if (g_value_type_transformable (G_TYPE_STRING, spec->value_type))
        {
          g_value_init (&dst, spec->value_type);
          if (g_value_transform (&src, &dst))
            g_object_set_property (G_OBJECT (preferences), spec->name, &dst);
          g_value_unset (&dst);
        }
      else
        {
          g_warning ("Failed to load property \"%s\"", spec->name);
        }

      g_value_unset (&src);
    }
  g_free (specs);

  preferences->loading_in_progress = FALSE;

  xfce_rc_close (rc);

  g_object_thaw_notify (G_OBJECT (preferences));

  return FALSE;
}



static void
tvm_preferences_load_idle_destroy (gpointer user_data)
{
  TVM_PREFERENCES (user_data)->load_idle_id = 0;
}



static gboolean
tvm_preferences_store_idle (gpointer user_data)
{
  TvmPreferences *preferences = TVM_PREFERENCES (user_data);
  const gchar    *string;
  GParamSpec    **specs;
  GParamSpec     *spec;
  XfceRc         *rc;
  GValue          dst = { 0, };
  GValue          src = { 0, };
  gchar          *option;
  guint           nspecs;
  guint           n;

  rc = xfce_rc_config_open (XFCE_RESOURCE_CONFIG, "Thunar/volmanrc", FALSE);
  if (G_UNLIKELY (rc == NULL))
    {
      g_warning ("Failed to store thunar-volman preferences.");
      return FALSE;
    }

  /* suspend the monitor (hopefully tricking FAM to avoid unnecessary reloads) */
  tvm_preferences_suspend_monitor (preferences);

  xfce_rc_set_group (rc, "Configuration");

  specs = g_object_class_list_properties (G_OBJECT_GET_CLASS (preferences), &nspecs);
  for (n = 0; n < nspecs; ++n)
    {
      spec = specs[n];

      g_value_init (&dst, G_TYPE_STRING);

      if (spec->value_type == G_TYPE_STRING)
        {
          g_object_get_property (G_OBJECT (preferences), spec->name, &dst);
        }
      else
        {
          g_value_init (&src, spec->value_type);
          g_object_get_property (G_OBJECT (preferences), spec->name, &src);
          g_value_transform (&src, &dst);
          g_value_unset (&src);
        }

      /* determine the option name for the spec */
      option = property_name_to_option_name (spec->name);

      /* store the setting */
      string = g_value_get_string (&dst);
      if (G_LIKELY (string != NULL))
        xfce_rc_write_entry (rc, option, string);

      /* cleanup */
      g_value_unset (&dst);
      g_free (option);
    }

  /* cleanup */
  xfce_rc_close (rc);
  g_free (specs);

  /* restart the monitor */
  tvm_preferences_resume_monitor (preferences);

  return FALSE;
}



static void
tvm_preferences_store_idle_destroy (gpointer user_data)
{
  TVM_PREFERENCES (user_data)->store_idle_id = 0;
}



/**
 * tvm_preferences_get:
 *
 * Queries the global #TvmPreferences instance, which is shared
 * by all modules. The function automatically takes a reference
 * for the caller, so you'll need to call g_object_unref() when
 * you're done with it.
 *
 * Return value: the global #TvmPreferences instance.
 **/
TvmPreferences*
tvm_preferences_get (void)
{
  static TvmPreferences *preferences = NULL;

  if (G_UNLIKELY (preferences == NULL))
    {
      preferences = g_object_new (TVM_TYPE_PREFERENCES, NULL);
      g_object_add_weak_pointer (G_OBJECT (preferences), (gpointer) &preferences);
    }
  else
    {
      g_object_ref (G_OBJECT (preferences));
    }

  return preferences;
}


