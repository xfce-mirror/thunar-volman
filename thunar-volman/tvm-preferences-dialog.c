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

#include <thunar-volman/tvm-command-entry.h>
#include <thunar-volman/tvm-pango-extensions.h>
#include <thunar-volman/tvm-preferences-dialog.h>
#include <thunar-volman/xfce-titled-dialog.h>



static void tvm_preferences_dialog_class_init (TvmPreferencesDialogClass *klass);
static void tvm_preferences_dialog_init       (TvmPreferencesDialog      *dialog);
static void tvm_preferences_dialog_finalize   (GObject                   *object);



struct _TvmPreferencesDialogClass
{
  XfceTitledDialogClass __parent__;
};

struct _TvmPreferencesDialog
{
  XfceTitledDialog __parent__;
  TvmPreferences  *preferences;
};



static GObjectClass *tvm_preferences_dialog_parent_class;



GType
tvm_preferences_dialog_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (TvmPreferencesDialogClass),
        NULL,
        NULL,
        (GClassInitFunc) tvm_preferences_dialog_class_init,
        NULL,
        NULL,
        sizeof (TvmPreferencesDialog),
        0,
        (GInstanceInitFunc) tvm_preferences_dialog_init,
        NULL,
      };

      type = g_type_register_static (XFCE_TYPE_TITLED_DIALOG, I_("TvmPreferencesDialog"), &info, 0);
    }

  return type;
}



static void
tvm_preferences_dialog_class_init (TvmPreferencesDialogClass *klass)
{
  GObjectClass *gobject_class;

  /* determine the parent type class */
  tvm_preferences_dialog_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = tvm_preferences_dialog_finalize;
}



static void
tvm_preferences_dialog_init (TvmPreferencesDialog *dialog)
{
  GtkSizeGroup *size_group;
  GtkWidget    *notebook;
  GtkWidget    *button;
  GtkWidget    *entry;
  GtkWidget    *frame;
  GtkWidget    *image;
  GtkWidget    *label;
  GtkWidget    *table;
  GtkWidget    *vbox;

  /* grab a reference on the preferences */
  dialog->preferences = tvm_preferences_get ();

  /* configure the dialog properties */
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
  gtk_window_set_icon_name (GTK_WINDOW (dialog), "gnome-dev-removable");
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Removable Drives and Media"));
  xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG (dialog), _("Configure management of removable drives and media"));

  /* add "Help" and "Close" buttons */
  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                          GTK_STOCK_HELP, GTK_RESPONSE_HELP,
                          NULL);

  notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), notebook, TRUE, TRUE, 0);
  gtk_widget_show (notebook);


  /*
     Storage
   */
  label = gtk_label_new (_("Storage"));
  vbox = g_object_new (GTK_TYPE_VBOX, "border-width", 12, "spacing", 12, NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
  gtk_widget_show (label);
  gtk_widget_show (vbox);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Removable Storage"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (5, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("gnome-dev-removable", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("_Mount removable drives when hot-plugged"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "automount-drives", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("Mount removable media when _inserted"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "automount-media", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("B_rowse removable media when inserted"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autobrowse", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("_Auto-run programs on new drives and media"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autorun", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 3, 4, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("Auto-open files on new drives and media"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autoopen", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 4, 5, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Blank CDs and DVDs"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("tvm-burn-cd", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("_Burn a CD or DVD when a blank disc is inserted"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autoburn", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  /* use a size group to make sure both labels request the same width */
  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  entry = tvm_command_entry_new_with_label (_("Command for _Data CDs:"));
  exo_binding_new (G_OBJECT (button), "active", G_OBJECT (entry), "sensitive");
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autoburn-data-command", G_OBJECT (entry), "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_size_group_add_widget (size_group, TVM_COMMAND_ENTRY (entry)->label);
  gtk_widget_show (entry);

  entry = tvm_command_entry_new_with_label (_("Command for A_udio CDs:"));
  exo_binding_new (G_OBJECT (button), "active", G_OBJECT (entry), "sensitive");
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autoburn-audio-command", G_OBJECT (entry), "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_size_group_add_widget (size_group, TVM_COMMAND_ENTRY (entry)->label);
  gtk_widget_show (entry);

  /* release the size group */
  g_object_unref (G_OBJECT (size_group));


  /*
     Multimedia
   */
  label = gtk_label_new (_("Multimedia"));
  vbox = g_object_new (GTK_TYPE_VBOX, "border-width", 12, "spacing", 12, NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
  gtk_widget_show (label);
  gtk_widget_show (vbox);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Audio CDs"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("gnome-dev-cdrom-audio", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Play _audio CDs when inserted"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autoplay-audio-cd", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  exo_binding_new (G_OBJECT (button), "active", G_OBJECT (entry), "sensitive");
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autoplay-audio-cd-command", G_OBJECT (entry), "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Video CDs/DVDs"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("gnome-dev-cdrom", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Play _video CDs and DVDs when inserted"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autoplay-video-cd", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("C_ommand:"));
  exo_binding_new (G_OBJECT (button), "active", G_OBJECT (entry), "sensitive");
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autoplay-video-cd-command", G_OBJECT (entry), "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Portable Music Players"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("gnome-dev-ipod", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Play _music files when connected"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autoipod", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("C_ommand:"));
  exo_binding_new (G_OBJECT (button), "active", G_OBJECT (entry), "sensitive");
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autoipod-command", G_OBJECT (entry), "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (entry);


  /*
     Cameras
   */
  label = gtk_label_new (_("Cameras"));
  vbox = g_object_new (GTK_TYPE_VBOX, "border-width", 12, "spacing", 12, NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
  gtk_widget_show (label);
  gtk_widget_show (vbox);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Digital Cameras"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("camera-photo", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Import digital photographs when connected"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autophoto", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  exo_binding_new (G_OBJECT (button), "active", G_OBJECT (entry), "sensitive");
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autophoto-command", G_OBJECT (entry), "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  /*
     Input Devices
   */
  label = gtk_label_new (_("Input Devices"));
  vbox = g_object_new (GTK_TYPE_VBOX, "border-width", 12, "spacing", 12, NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
  gtk_widget_show (label);
  gtk_widget_show (vbox);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Keyboards"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("gnome-dev-keyboard", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Automatically run a program when an USB _keyboard is connected"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autokeyboard", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  exo_binding_new (G_OBJECT (button), "active", G_OBJECT (entry), "sensitive");
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autokeyboard-command", G_OBJECT (entry), "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Mice"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("gnome-dev-mouse-optical", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Automatically run a program when an USB _mouse is connected"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "automouse", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("C_ommand:"));
  exo_binding_new (G_OBJECT (button), "active", G_OBJECT (entry), "sensitive");
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "automouse-command", G_OBJECT (entry), "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Tablet"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("tvm-dev-tablet", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Automatically run a program when a _tablet is connected"));
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autotablet", G_OBJECT (button), "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("Com_mand:"));
  exo_binding_new (G_OBJECT (button), "active", G_OBJECT (entry), "sensitive");
  exo_mutual_binding_new (G_OBJECT (dialog->preferences), "autotablet-command", G_OBJECT (entry), "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (entry);
}



static void
tvm_preferences_dialog_finalize (GObject *object)
{
  TvmPreferencesDialog *dialog = TVM_PREFERENCES_DIALOG (object);

  /* release our reference on the preferences */
  g_object_unref (G_OBJECT (dialog->preferences));

  (*G_OBJECT_CLASS (tvm_preferences_dialog_parent_class)->finalize) (object);
}



/**
 * tvm_preferences_dialog_new:
 * @parent : a #GtkWindow or %NULL.
 *
 * Allocates a new #TvmPreferencesDialog widget.
 *
 * Return value: the newly allocated #TvmPreferencesDialog.
 **/
GtkWidget*
tvm_preferences_dialog_new (void)
{
  return g_object_new (TVM_TYPE_PREFERENCES_DIALOG, NULL);
}

