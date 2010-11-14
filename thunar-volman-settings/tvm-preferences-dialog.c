/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@xfce.org>
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

#include <glib.h>
#include <glib-object.h>

#include <libxfce4ui/libxfce4ui.h>

#include <xfconf/xfconf.h>

#include <thunar-volman/tvm-pango-extensions.h>

#include <thunar-volman-settings/tvm-command-entry.h>
#include <thunar-volman-settings/tvm-preferences-dialog.h>



/* property identifiers */
enum
{
  PROP_0,
};



static void tvm_preferences_dialog_help_clicked (GtkWidget            *button,
                                                 TvmPreferencesDialog *dialog);



struct _TvmPreferencesDialogClass
{
  XfceTitledDialogClass __parent__;
};

struct _TvmPreferencesDialog
{
  XfceTitledDialog __parent__;
};

struct _TvmPreferencesDialogPrivate
{
};



G_DEFINE_TYPE (TvmPreferencesDialog, tvm_preferences_dialog, XFCE_TYPE_TITLED_DIALOG)



static void
tvm_preferences_dialog_class_init (TvmPreferencesDialogClass *klass)
{
  /* Determine the parent type class */
  tvm_preferences_dialog_parent_class = g_type_class_peek_parent (klass);
}



static void
tvm_preferences_dialog_init (TvmPreferencesDialog *dialog)
{
  XfconfChannel *channel;
  GtkSizeGroup  *size_group;
  GtkWidget     *button;
  GtkWidget     *entry;
  GtkWidget     *frame;
  GtkWidget     *label;
  GtkWidget     *image;
  GtkWidget     *notebook;
  GtkWidget     *table;
  GtkWidget     *vbox;

  channel = xfconf_channel_get ("thunar-volman");

  /* configure the dialog properties */
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
  gtk_window_set_icon_name (GTK_WINDOW (dialog), "drive-removable-media");
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Removable Drives and Media"));
  xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG (dialog), 
                                   _("Configure management of removable drives "
                                     "and media"));

  /* add "Help" button */
  button = gtk_button_new_from_stock (GTK_STOCK_HELP);
  g_signal_connect (G_OBJECT (button), "clicked", 
                    G_CALLBACK (tvm_preferences_dialog_help_clicked), dialog);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area), button, 
                      FALSE, FALSE, 0);
  gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (GTK_DIALOG (dialog)->action_area),
                                      button, TRUE);
  gtk_widget_show (button);

  /* add "Close" button */
  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
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

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
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

  image = gtk_image_new_from_icon_name ("drive-removable-media", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("_Mount removable drives when "
                                                 "hot-plugged"));
  xfconf_g_property_bind (channel, "/automount-drives/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("Mount removable media when "
                                                 "_inserted"));
  xfconf_g_property_bind (channel, "/automount-media/enabled", G_TYPE_BOOLEAN,
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("B_rowse removable media when "
                                                 "inserted"));
  xfconf_g_property_bind (channel, "/autobrowse/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("_Auto-run programs on new drives "
                                                 "and media"));
  xfconf_g_property_bind (channel, "/autorun/enabled", G_TYPE_BOOLEAN, button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 3, 4, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("Auto-open files on new drives "
                                                 "and media"));
  xfconf_g_property_bind (channel, "/autoopen/enabled", G_TYPE_BOOLEAN, button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 4, 5, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
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

  button = gtk_check_button_new_with_mnemonic (_("_Burn a CD or DVD when a blank disc "
                                                 "is inserted"));
  xfconf_g_property_bind (channel, "/autoburn/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  /* use a size group to make sure both labels request the same width */
  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  entry = tvm_command_entry_new_with_label (_("Command for _Data CDs:"));
  xfconf_g_property_bind (channel, "/autoburn/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoburn/data-cd-command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_size_group_add_widget (size_group, TVM_COMMAND_ENTRY (entry)->label);
  gtk_widget_show (entry);

  entry = tvm_command_entry_new_with_label (_("Command for A_udio CDs:"));
  xfconf_g_property_bind (channel, "/autoburn/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoburn/audio-cd-command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
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

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
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

  image = gtk_image_new_from_icon_name ("media-optical", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Play _audio CDs when inserted"));
  xfconf_g_property_bind (channel, "/autoplay-audio-cds/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  xfconf_g_property_bind (channel, "/autoplay-audio-cds/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoplay-audio-cds/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
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

  image = gtk_image_new_from_icon_name ("drive-optical", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Play _video CDs and DVDs when "
                                                 "inserted"));
  xfconf_g_property_bind (channel, "/autoplay-video-cds/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("C_ommand:"));
  xfconf_g_property_bind (channel, "/autoplay-video-cds/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoplay-video-cds/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL,
                    GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
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

  image = gtk_image_new_from_icon_name ("multimedia-player", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Play _music files when connected"));
  xfconf_g_property_bind (channel, "/autoipod/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("C_ommand:"));
  xfconf_g_property_bind (channel, "/autoipod/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoipod/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  /*
     Cameras
   */
  label = gtk_label_new (_("Cameras"));
  vbox = g_object_new (GTK_TYPE_VBOX, "border-width", 12, "spacing", 12, NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
  gtk_widget_show (label);
  gtk_widget_show (vbox);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
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

  button = gtk_check_button_new_with_mnemonic (_("Import digital photographs "
                                                 "when connected"));
  xfconf_g_property_bind (channel, "/autophoto/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  xfconf_g_property_bind (channel, "/autophoto/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autophoto/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  /*
     PDAs
   */
  label = gtk_label_new (_("PDAs"));
  vbox = g_object_new (GTK_TYPE_VBOX, "border-width", 12, "spacing", 12, NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
  gtk_widget_show (label);
  gtk_widget_show (vbox);

  /* FIXME make the vbox sensitive again as soon as there is PDA support */
  gtk_widget_set_sensitive (vbox, FALSE);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Palm™"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("pda", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Sync _Palm™ devices "
                                                 "when connected"));
  xfconf_g_property_bind (channel, "/autopalm/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  xfconf_g_property_bind (channel, "/autopalm/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autopalm/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Pocket PCs"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("tvm-dev-pocketpc", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Sync Pocket P_C devices "
                                                 "when connected"));
  xfconf_g_property_bind (channel, "/autopocketpc/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("C_ommand:"));
  xfconf_g_property_bind (channel, "/autopocketpc/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autopocketpc/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL,
                    GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  /*
     Printers
   */
  label = gtk_label_new (_("Printers"));
  vbox = g_object_new (GTK_TYPE_VBOX, "border-width", 12, "spacing", 12, NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
  gtk_widget_show (label);
  gtk_widget_show (vbox);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Printers"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  image = gtk_image_new_from_icon_name ("printer", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Automatically run a program "
                                                 "when a _printer is connected"));
  xfconf_g_property_bind (channel, "/autoprinter/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  xfconf_g_property_bind (channel, "/autoprinter/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoprinter/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL,
                    GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  /*
     Input Devices
   */
  label = gtk_label_new (_("Input Devices"));
  vbox = g_object_new (GTK_TYPE_VBOX, "border-width", 12, "spacing", 12, NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
  gtk_widget_show (label);
  gtk_widget_show (vbox);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
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

  image = gtk_image_new_from_icon_name ("input-keyboard", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Automatically run a program "
                                                 "when a USB _keyboard is connected"));
  xfconf_g_property_bind (channel, "/autokeyboard/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  xfconf_g_property_bind (channel, "/autokeyboard/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autokeyboard/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL,
                    GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
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

  image = gtk_image_new_from_icon_name ("input-mouse", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Automatically run a program "
                                                 "when a USB _mouse is connected"));
  xfconf_g_property_bind (channel, "/automouse/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL,
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("C_ommand:"));
  xfconf_g_property_bind (channel, "/automouse/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/automouse/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL,
                    GTK_FILL, 0, 0);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
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

  image = gtk_image_new_from_icon_name ("input-tablet", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), image, 0, 1, 0, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Automatically run a program "
                                                 "when a _tablet is connected"));
  xfconf_g_property_bind (channel, "/autotablet/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL,
                    GTK_FILL, 0, 0);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("Com_mand:"));
  xfconf_g_property_bind (channel, "/autotablet/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autotablet/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 
                    GTK_FILL, 0, 0);
  gtk_widget_show (entry);
}



static void
tvm_preferences_dialog_help_clicked (GtkWidget            *button,
                                     TvmPreferencesDialog *dialog)
{
  GtkWidget *message;
  GError    *err = NULL;
  gchar    **argv;
  gchar     *bindir;
  gchar     *prefix;
  gchar     *path;

  g_return_if_fail (GTK_IS_BUTTON (button));
  g_return_if_fail (TVM_IS_PREFERENCES_DIALOG (dialog));

  /* try to locate Thunar in the $PATH */
  path = g_find_program_in_path ("Thunar");
  if (G_UNLIKELY (path == NULL))
    path = g_find_program_in_path ("thunar");
  if (G_LIKELY (path != NULL))
    {
      bindir = g_path_get_dirname (path);
      prefix = g_path_get_dirname (bindir);
      g_free (bindir);
      g_free (path);

      /* now check if ThunarHelp is in $prefix/libexec */
      path = g_build_filename (prefix, "libexec", "ThunarHelp", NULL);
      if (!g_file_test (path, G_FILE_TEST_IS_EXECUTABLE))
        {
          /* release path */
          g_free (path);

          /* try to support Debian weirdness */
          path = g_build_filename (prefix, "lib", "thunar", "ThunarHelp", NULL);
          if (!g_file_test (path, G_FILE_TEST_IS_EXECUTABLE))
            {
              /* release path */
              g_free (path);
              path = NULL;
            }
        }
    }

  /* no ThunarHelp, weird! */
  if (G_UNLIKELY (path == NULL))
    path = g_strdup ("ThunarHelp");

  /* prepare command to run help */
  argv = g_new (gchar *, 4);
  argv[0] = path;
  argv[1] = g_strdup ("using-removable-media");
  argv[2] = g_strdup ("management-of-removable-drives-and-media");
  argv[3] = NULL;

  /* try to open the user manual */
  if (!gdk_spawn_on_screen (gtk_widget_get_screen (button), NULL, argv, NULL, 
                            G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &err))
    {
      /* display an error message to the user */
      message = gtk_message_dialog_new (GTK_WINDOW (dialog),
                                        GTK_DIALOG_DESTROY_WITH_PARENT
                                        | GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_CLOSE,
                                        _("Failed to open the documentation browser"));
      gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (message), "%s", 
                                                err->message);
      gtk_dialog_run (GTK_DIALOG (message));
      gtk_widget_destroy (message);
      g_error_free (err);
    }

  /* cleanup */
  g_strfreev (argv);
}



GtkWidget *
tvm_preferences_dialog_new (void)
{
  return g_object_new (TVM_TYPE_PREFERENCES_DIALOG, NULL);
}
