/* $Id$ */
/*-
 * Copyright (c) 2008 Benedikt Meurer <benny@xfce.org>.
 * Copyright (c) 2008 Colin Leroy <colin@colino.net>.
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
#include <errno.h>

#include <dbus/dbus-glib-lowlevel.h>
#include <thunar-volman/tvm-block-device.h>
#include <thunar-volman/tvm-crypto-volume.h>
#include <libhal-storage.h>

static void 
tvm_crypto_volume_prompt_update_text (GtkEditable *editable,
                        gchar *new_text,
			gint new_text_len,
			gint *position,
			gpointer data)
{
  gchar **response = (gchar **)data;
  if (response == NULL)
    return;
  g_free(*response);
  *response = gtk_editable_get_chars(editable, 0, -1);    
}
 
typedef struct _CryptoDialogValidate {
  GtkWidget *dialog;
  gint       response;
} CryptoDialogValidate;

void
tvm_crypto_volume_prompt_validate_text (GtkEntry *entry,
                          gpointer data)
{
  CryptoDialogValidate *validate_data = (CryptoDialogValidate *)data;
  gtk_dialog_response(GTK_DIALOG(validate_data->dialog), validate_data->response);
}

static void
tvm_crypto_volume_hal_device_removed (LibHalContext *context,
                        const gchar   *udi)
{
  const gchar *dialog_udi;
  GtkWidget   *dialog = libhal_ctx_get_user_data (context);

  /* check if the active UDI of the dialog was removed */
  dialog_udi = g_object_get_data (G_OBJECT (dialog), "udi");
  if (exo_str_is_equal (dialog_udi, udi))
    {
      /* cancel the dialog */
      gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
    }
}

 /**
 * tvm_crypto_volume_ask_password:
 * @context           : a #LibHalContext.
 * @udi               : the UDI of the device being added, which is watched for removal.
 * @icon              : the icon or %NULL.
 * @title             : the prompt title.
 * @response_text     : the response text.
 * @visible           : whether text typed should be visible.
 * @default_response  : Which response to return on Enter.
 * @primary_text      : the primary prompt text.
 * @secondary_text    : the secondary prompt text.
 * @first_button_text : the first button text.
 * @...               : %NULL-terminated list of button text, response id pairs.
 *
 * Return value: the selected response.
 **/
static gint
tvm_crypto_volume_ask_password (LibHalContext *context,
		  const gchar   *udi,
		  const gchar   *icon,
		  const gchar   *title,
		  gchar        **response_text,
		  gboolean       visible,
		  gint           default_response,
		  const gchar   *primary_text,
		  const gchar   *secondary_text,
		  const gchar   *first_button_text,
		  ...)
{
  GtkWidget *dialog;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *entry;
  DBusError  derror;
  va_list    args;
  gint       response;
  CryptoDialogValidate validate_data;

  g_return_val_if_fail (exo_hal_udi_validate (udi, -1, NULL), 0);
  g_return_val_if_fail (context != NULL, GTK_RESPONSE_CANCEL);

  /* allocate a new dialog */
  dialog = gtk_dialog_new ();
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
  g_object_set_data_full (G_OBJECT (dialog), "udi", g_strdup (udi), g_free);
  gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), 6);
  gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area), 12);

  /* setup the specified title */
  if (G_LIKELY (title != NULL))
    gtk_window_set_title (GTK_WINDOW (dialog), title);

  /* setup the specified buttons */
  if (G_LIKELY (first_button_text != NULL))
    {
      va_start (args, first_button_text);
      for (response = va_arg (args, gint); first_button_text != NULL; )
        {
          /* insert the button */
          gtk_dialog_add_button (GTK_DIALOG (dialog), first_button_text, response);
          first_button_text = va_arg (args, const gchar *);
          if (G_UNLIKELY (first_button_text == NULL))
            break;
          response = va_arg (args, gint);
        }
      va_end (args);
    }

  /* setup the hbox */
  hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  /* setup the specified icon */
  if (G_LIKELY (icon != NULL))
    {
      /* setup an image for the icon */
      image = gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_DIALOG);
      gtk_misc_set_alignment (GTK_MISC (image), 0.0f, 0.0f);
      gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
      gtk_widget_show (image);
    }

  /* setup the vbox */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);
  g_object_set_data(G_OBJECT(dialog), "main_vbox", vbox);

  /* setup the primary text */
  label = gtk_label_new (primary_text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  /* setup the secondary text */
  if (G_LIKELY (secondary_text != NULL))
    {
      label = gtk_label_new (secondary_text);
      gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
      gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
      gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
      gtk_widget_show (label);
      g_object_set_data(G_OBJECT(dialog), "secondary_text", label);
    }

  entry = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(entry), visible);
  gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
  gtk_widget_show (entry);
  g_object_set_data(G_OBJECT(dialog), "pass_entry", entry);
  
  /* initialize D-Bus error */
  dbus_error_init (&derror);

  /* setup HAL to watch the UDI for removal */
  libhal_ctx_set_user_data (context, dialog);
  libhal_ctx_set_device_removed (context, tvm_crypto_volume_hal_device_removed);
  libhal_device_property_watch_all (context, &derror);

  /* setup handler for the entered text */
  if (response_text)
    *response_text = NULL;
  g_signal_connect_after(G_OBJECT(entry), "insert-text",
       G_CALLBACK(tvm_crypto_volume_prompt_update_text),
       (gpointer) response_text);
  validate_data.dialog = dialog;
  validate_data.response = default_response;
  g_signal_connect(G_OBJECT(entry), "activate",
       G_CALLBACK(tvm_crypto_volume_prompt_validate_text),
       (gpointer) &validate_data);

  /* run the dialog */
  response = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);

  /* cleanup */
  libhal_ctx_set_device_removed (context, NULL);
  libhal_ctx_set_user_data (context, NULL);
  dbus_error_free (&derror);

  return response;
}

static gboolean 
progress_pulse(gpointer data)
{
  GtkProgressBar *progress = GTK_PROGRESS_BAR(data);
  g_return_val_if_fail(progress != NULL, FALSE);
  
  gtk_progress_bar_pulse(progress);
  
  return TRUE;
}

static void 
tvm_crypto_volume_dbus_call_done (DBusPendingCall *call, 
                                 void *user_data)
{
  gtk_main_quit();
}

static gchar *
tvm_crypto_volume_mount_install_crypto_volume (LibHalContext *context,
				 const gchar *udi, 
				 LibHalVolume *volume,
				 const gchar *password,
				 gboolean *pass_error,
				 GError **error)
{
  gchar *plain_udi = NULL;
  DBusMessage *message = NULL;
  DBusMessage *result = NULL;
  DBusConnection *dbus_connection = NULL;
  DBusPendingCall *call = NULL;
  DBusError derror;
  dbus_bool_t send_err;
  *pass_error = FALSE;
  GtkWidget *window = NULL;
  GtkWidget *label, *vbox, *hbox;
  GtkWidget *wait_progress;
  GtkWidget *image;
  guint animator_id;
  g_clear_error(error);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 8);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(window), _("Encrypted volume"));
  gtk_window_set_modal(GTK_WINDOW(window), TRUE);
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

  vbox = gtk_vbox_new(FALSE, 6);
  hbox = gtk_hbox_new(FALSE, 6);
  image = gtk_image_new_from_icon_name ("gtk-dialog-authentication", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.0f, 0.0f);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  gtk_widget_show (image);

  label = gtk_label_new(_("<span weight=\"bold\" size=\"larger\">Mounting encrypted volume...</span>"));
  gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
  gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, FALSE, 0);

  label = gtk_label_new(_("Setting up the crypto layer..."));
  gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, FALSE, 0);

  wait_progress = gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(vbox), wait_progress, TRUE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(window), hbox);
  gtk_widget_show_all(hbox);
  animator_id = g_timeout_add(100, progress_pulse, wait_progress);

  gtk_widget_show_now(window);

  message = dbus_message_new_method_call ("org.freedesktop.Hal", udi,
              "org.freedesktop.Hal.Device.Volume.Crypto",
              "Setup");

  if (G_UNLIKELY (message == NULL))
    {
      /* out of memory */
oom:  g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_NOMEM, g_strerror (ENOMEM));
      goto out;
    }

  if (!dbus_message_append_args (message, 
               DBUS_TYPE_STRING, &password,
               DBUS_TYPE_INVALID)) {
      dbus_message_unref (message);
      goto oom;
  }
  
  dbus_error_init (&derror);

  dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &derror);
  if (G_UNLIKELY (dbus_connection == NULL))
    {
      /* propagate the error */
      dbus_set_g_error (error, &derror);
      goto out;
    }

  send_err = dbus_connection_send_with_reply (dbus_connection, message, &call, -1);
  if (G_UNLIKELY (send_err == FALSE))
    {
      /* release the result */
      dbus_message_unref (message);
      goto out;
    }

  dbus_pending_call_set_notify(call, tvm_crypto_volume_dbus_call_done, NULL, NULL);
  gtk_main();

  result = dbus_pending_call_steal_reply(call);
  /* release the message */
  dbus_message_unref (message);

  /* release the connection */
  dbus_connection_unref (dbus_connection);

  if (G_UNLIKELY (dbus_set_error_from_message(&derror, result)))
    {
      /* try to translate the error appropriately */
      if (strcmp (derror.name, "org.freedesktop.Hal.Device.Volume.PermissionDenied") == 0)
        {
          /* TRANSLATORS: The user tried to setup a crypto layer (LUKS-encrpyted volume) but got Permission denied. */
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, _("You are not privileged to setup the crypto layer"));
        }
      else if (strcmp (derror.name, "org.freedesktop.Hal.Device.Volume.Crypto.SetupPasswordError") == 0)
        {
          /* TRANSLATORS: The user tried to setup a crypto layer (LUKS-encrpyted volume) and typed the wrong password. */
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, _("Wrong password"));
          *pass_error = TRUE;
        }
      else
        {
          /* no precise error message, use the HAL one */
          dbus_set_g_error (error, &derror);
        }

      /* release the DBus error */
      dbus_error_free (&derror);
      dbus_message_unref (result);
      plain_udi = NULL;
      goto out;
    }
  else
    {
      dbus_message_unref (result);
      plain_udi = libhal_volume_crypto_get_clear_volume_udi (context, volume);
    }

out:
  g_source_remove(animator_id);
  gtk_widget_destroy(window);
  return plain_udi;
}

/**
 * tvm_crypto_volume_setup:
 * @preferences : a #TvmPreferences.
 * @context     : a #LibHalContext.
 * @udi         : the HAL volume UDI of the newly added crypto volume.
 * @error       : return location for errors or %NULL.
 *
 * Tries to setup the crypto layer for the volume with the
 * specified @udi.
 *
 * Return value: %TRUE if handled, %FALSE if not handled or an
 *               unrecoverable error occurred.
 */
gboolean
tvm_crypto_volume_setup (TvmPreferences *preferences,
                         LibHalContext  *context,
                         const gchar    *udi,
                         GError        **error)
{
  gchar *password = NULL;
  gboolean pass_error = FALSE;
  gint num_tries = 0;
  gchar *plain_udi = NULL;
  LibHalVolume *volume = NULL;
  gboolean result = FALSE;

  g_return_val_if_fail (exo_hal_udi_validate (udi, -1, NULL), FALSE);
  g_return_val_if_fail (TVM_IS_PREFERENCES (preferences), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (context != NULL, FALSE);

  volume = libhal_volume_from_udi (context, udi);
  g_return_val_if_fail(volume != NULL, FALSE);
  plain_udi = libhal_volume_crypto_get_clear_volume_udi (context, volume);
  if (plain_udi != NULL) {
    libhal_volume_free(volume);
    libhal_free_string(plain_udi);
    return TRUE;
  }
  
  for (num_tries = 0; num_tries < 3; num_tries++) {
    gint response,len;
    response = tvm_crypto_volume_ask_password (context, udi, "gtk-dialog-authentication", _("Encrypted volume"),
               &password, FALSE, GTK_RESPONSE_OK,
                                             pass_error? _("<span weight=\"bold\" size=\"larger\">Wrong password.</span>")
                                :_("<span weight=\"bold\" size=\"larger\">This volume is encrypted.</span>"),
                                             _("Please enter your password to decrypt and mount the volume."),
                                             _("Ig_nore"), GTK_RESPONSE_CANCEL,
                                             _("_Mount"), GTK_RESPONSE_OK,
                                             NULL);
    if (response == GTK_RESPONSE_CANCEL || response == GTK_RESPONSE_DELETE_EVENT) {
      g_clear_error(error);
      num_tries = 3;
    } else if (password) {
      plain_udi = tvm_crypto_volume_mount_install_crypto_volume(context, udi, volume, password, &pass_error, error);
    }
    if (password) {
      len = strlen(password);
      memset(password, 0, len);
      g_debug("zeroed password %p", password);
      g_free(password);
    }
    if (*error != NULL && !pass_error) {
      /* display an error message to the user (exo-mount won't do it as it won't see the clear device */
      GtkWidget *message = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_CLOSE,
                                        "%s.", _("Failed to setup the encrypted volume"));
      gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (message), "%s.", (*error)->message);
      gtk_dialog_run (GTK_DIALOG (message));
      gtk_widget_destroy (message);
      break;
    }
    if (plain_udi != NULL) {
      libhal_free_string(plain_udi);
      result = TRUE;
      break;
    }
  }
  return result;  
}


