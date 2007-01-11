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

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include <thunar-volman/tvm-pango-extensions.h>
#include <thunar-volman/tvm-prompt.h>



static void
hal_device_removed (LibHalContext *context,
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
 * tvm_prompt:
 * @context           : a #LibHalContext.
 * @udi               : the UDI of the device being added, which is watched for removal.
 * @icon              : the icon or %NULL.
 * @title             : the prompt title.
 * @primary_text      : the primary prompt text.
 * @secondary_text    : the secondary prompt text.
 * @first_button_text : the first button text.
 * @...               : %NULL-terminated list of button text, response id pairs.
 *
 * Return value: the selected response.
 **/
gint
tvm_prompt (LibHalContext *context,
            const gchar   *udi,
            const gchar   *icon,
            const gchar   *title,
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
  DBusError  derror;
  va_list    args;
  gint       response;

  g_return_val_if_fail (exo_hal_udi_validate (udi, -1, NULL), GTK_RESPONSE_CANCEL);
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

  /* setup the primary text */
  label = gtk_label_new (primary_text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_big_bold ());
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
    }

  /* initialize D-Bus error */
  dbus_error_init (&derror);

  /* setup HAL to watch the UDI for removal */
  libhal_ctx_set_user_data (context, dialog);
  libhal_ctx_set_device_removed (context, hal_device_removed);
  libhal_device_property_watch_all (context, &derror);

  /* run the dialog */
  response = gtk_dialog_run (GTK_DIALOG (dialog));

  /* cleanup */
  libhal_ctx_set_device_removed (context, NULL);
  libhal_ctx_set_user_data (context, NULL);
  gtk_widget_destroy (dialog);
  dbus_error_free (&derror);

  return response;
}


