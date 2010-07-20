/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2007 Benedikt Meurer <benny@xfce.org>
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

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include <gudev/gudev.h>

#include <gtk/gtk.h>

#include <libxfce4ui/libxfce4ui.h>

#include <thunar-volman/tvm-context.h>
#include <thunar-volman/tvm-pango-extensions.h>
#include <thunar-volman/tvm-prompt.h>



static void
tvm_prompt_uevent (GUdevClient *client,
                   const gchar *action,
                   GUdevDevice *device,
                   GtkWidget   *dialog)
{
  GUdevDevice *dialog_device;

  g_return_if_fail (G_UDEV_IS_CLIENT (client));
  g_return_if_fail (action != NULL && *action != '\0');
  g_return_if_fail (G_UDEV_IS_DEVICE (device));
  g_return_if_fail (GTK_IS_DIALOG (dialog));

  /* ignore "move" and "add" actions, "change" might only be correct 
   * for CDs/DVDs though */
  if (g_strcmp0 (action, "remove") != 0 && g_strcmp0 (action, "change") != 0)
    return;

  /* determine the device belonging to the dialog */
  dialog_device = g_object_get_data (G_OBJECT (dialog), "device");

  /* the device should be set, otherwise it's a bug in the application */
  g_assert (dialog_device != NULL);

  /* check if the removed/changed device belongs to the dialog */
  if (g_strcmp0 (g_udev_device_get_sysfs_path (device),
                 g_udev_device_get_sysfs_path (dialog_device)) == 0)
    {
      /* cancel the dialog */
      gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
    }
}



gint
tvm_prompt (TvmContext  *context,
            const gchar *icon,
            const gchar *title,
            const gchar *primary_text,
            const gchar *secondary_text,
            const gchar *first_button_text,
            ...)
{
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *vbox;
  va_list    args;
  gint       response;

  g_return_val_if_fail (context != NULL, GTK_RESPONSE_CANCEL);
  
  /* create a new dialog */
  dialog = gtk_dialog_new ();
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  g_object_set_data_full (G_OBJECT (dialog), "device", g_object_ref (context->device), 
                          g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), 6);
  gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area), 12);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);

  /* apply the specified title */
  if (title != NULL && *title != '\0')
    gtk_window_set_title (GTK_WINDOW (dialog), title);

  /* create the specified buttons */
  if (first_button_text != NULL)
    {
      va_start (args, first_button_text);
      for (response = va_arg (args, gint); first_button_text != NULL; )
        {
          /* insert the button */
          gtk_dialog_add_button (GTK_DIALOG (dialog), first_button_text, response);
          first_button_text = va_arg (args, const gchar *);
          if (first_button_text == NULL)
            break;
          response = va_arg (args, gint);
        }
      va_end (args);
    }

  /* create the hbox */
  hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  /* apply the specified icon */
  if (G_LIKELY (icon != NULL))
    {
      gtk_window_set_icon_name (GTK_WINDOW (dialog), icon);

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

  g_signal_connect (context->client, "uevent", G_CALLBACK (tvm_prompt_uevent), dialog);

  /* run the dialog */
  response = gtk_dialog_run (GTK_DIALOG (dialog));

  /* clean up */
  gtk_widget_destroy (dialog);

  return response;
}
