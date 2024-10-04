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



static void tvm_preferences_dialog_response (GtkWidget *dialog,
                                             gint response_id);



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
  GtkWidget     *grid;
  GtkWidget     *vbox;

  channel = xfconf_channel_get ("thunar-volman");

  /* configure the dialog properties */
  gtk_window_set_icon_name (GTK_WINDOW (dialog), "org.xfce.volman");
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Removable Drives and Media"));

  g_signal_connect (dialog, "response", G_CALLBACK (tvm_preferences_dialog_response), NULL);

#if !LIBXFCE4UI_CHECK_VERSION (4, 19, 3)
  xfce_titled_dialog_create_action_area (XFCE_TITLED_DIALOG (dialog));
#endif

  /* add "Help" button */
  button = gtk_button_new_from_icon_name ("help-browser", GTK_ICON_SIZE_BUTTON);
  gtk_button_set_label (GTK_BUTTON (button), _("Help"));
  xfce_titled_dialog_add_action_widget (XFCE_TITLED_DIALOG (dialog), button, GTK_RESPONSE_HELP);
  gtk_widget_show (button);

  /* add "Close" button */
  button = gtk_button_new_from_icon_name ("window-close", GTK_ICON_SIZE_BUTTON);
  gtk_button_set_label (GTK_BUTTON (button), _("Close"));
  xfce_titled_dialog_add_action_widget (XFCE_TITLED_DIALOG (dialog), button, GTK_RESPONSE_CLOSE);
  gtk_widget_show (button);

  notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 6);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), notebook, TRUE, TRUE, 0);
  gtk_widget_show (notebook);

  /*
     Storage
   */
  label = gtk_label_new (_("Storage"));
  vbox = g_object_new (GTK_TYPE_BOX, "orientation", GTK_ORIENTATION_VERTICAL, "border-width", 12, "spacing", 18, NULL);
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

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 6);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 12);
  gtk_container_add (GTK_CONTAINER (frame), grid);
  gtk_widget_show (grid);

  image = gtk_image_new_from_icon_name ("media-removable", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 3);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("_Mount removable drives when "
                                                 "hot-plugged"));
  xfconf_g_property_bind (channel, "/automount-drives/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("Mount removable media when "
                                                 "_inserted"));
  xfconf_g_property_bind (channel, "/automount-media/enabled", G_TYPE_BOOLEAN,
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 1, 1, 1);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("B_rowse removable media when "
                                                 "inserted"));
  xfconf_g_property_bind (channel, "/autobrowse/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 2, 1, 1);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("_Auto-run programs on new drives "
                                                 "and media"));
  xfconf_g_property_bind (channel, "/autorun/enabled", G_TYPE_BOOLEAN, button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 3, 1, 1);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("Auto-open files on new drives "
                                                 "and media"));
  xfconf_g_property_bind (channel, "/autoopen/enabled", G_TYPE_BOOLEAN, button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 4, 1, 1);
  gtk_widget_show (button);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Blank CDs and DVDs"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 6);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 12);
  gtk_container_add (GTK_CONTAINER (frame), grid);
  gtk_widget_show (grid);

  image = gtk_image_new_from_icon_name ("media-optical", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 3);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("_Burn a CD or DVD when a blank disc "
                                                 "is inserted"));
  xfconf_g_property_bind (channel, "/autoburn/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);
  gtk_widget_show (button);

  /* use a size group to make sure both labels request the same width */
  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  entry = tvm_command_entry_new_with_label (_("Command for _Data CDs:"));
  xfconf_g_property_bind (channel, "/autoburn/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoburn/data-cd-command", G_TYPE_STRING, 
                          entry, "command");
  gtk_grid_attach (GTK_GRID (grid), entry, 1, 1, 1, 1);
  gtk_size_group_add_widget (size_group, TVM_COMMAND_ENTRY (entry)->label);
  gtk_widget_show (entry);

  entry = tvm_command_entry_new_with_label (_("Command for A_udio CDs:"));
  xfconf_g_property_bind (channel, "/autoburn/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoburn/audio-cd-command", G_TYPE_STRING, 
                          entry, "command");
  gtk_grid_attach (GTK_GRID (grid), entry, 1, 2, 1, 1);
  gtk_size_group_add_widget (size_group, TVM_COMMAND_ENTRY (entry)->label);
  gtk_widget_show (entry);

  /* release the size group */
  g_object_unref (G_OBJECT (size_group));

  /*
     Multimedia
   */
  label = gtk_label_new (_("Multimedia"));
  vbox = g_object_new (GTK_TYPE_BOX, "orientation", GTK_ORIENTATION_VERTICAL, "border-width", 12, "spacing", 18, NULL);
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

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 6);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 12);
  gtk_container_add (GTK_CONTAINER (frame), grid);
  gtk_widget_show (grid);

  image = gtk_image_new_from_icon_name ("media-optical", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 3);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Play _audio CDs when inserted"));
  xfconf_g_property_bind (channel, "/autoplay-audio-cds/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  xfconf_g_property_bind (channel, "/autoplay-audio-cds/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoplay-audio-cds/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_grid_attach (GTK_GRID (grid), entry, 1, 1, 1, 1);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Video CDs/DVDs/Blu-rays"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 6);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 12);
  gtk_container_add (GTK_CONTAINER (frame), grid);
  gtk_widget_show (grid);

  image = gtk_image_new_from_icon_name ("drive-optical", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 3);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Play _video CDs, DVDs, and "
                                                 "Blu-rays when inserted"));
  xfconf_g_property_bind (channel, "/autoplay-video-cds/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("C_ommand:"));
  xfconf_g_property_bind (channel, "/autoplay-video-cds/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoplay-video-cds/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_grid_attach (GTK_GRID (grid), entry, 1, 1, 1, 1);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Portable Music Players"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 6);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 12);
  gtk_container_add (GTK_CONTAINER (frame), grid);
  gtk_widget_show (grid);

  image = gtk_image_new_from_icon_name ("multimedia-player", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 3);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Play _music files when connected"));
  xfconf_g_property_bind (channel, "/autoipod/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("C_ommand:"));
  xfconf_g_property_bind (channel, "/autoipod/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoipod/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_grid_attach (GTK_GRID (grid), entry, 1, 1, 1, 1);
  gtk_widget_show (entry);

  /*
     Cameras
   */
  label = gtk_label_new (_("Cameras"));
  vbox = g_object_new (GTK_TYPE_BOX, "orientation", GTK_ORIENTATION_VERTICAL, "border-width", 12, "spacing", 18, NULL);
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

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 6);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 12);
  gtk_container_add (GTK_CONTAINER (frame), grid);
  gtk_widget_show (grid);

  image = gtk_image_new_from_icon_name ("camera-photo", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 3);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Import digital photographs "
                                                 "when connected"));
  xfconf_g_property_bind (channel, "/autophoto/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  xfconf_g_property_bind (channel, "/autophoto/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autophoto/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_grid_attach (GTK_GRID (grid), entry, 1, 1, 1, 1);
  gtk_widget_show (entry);

  /*
     Printers
   */
  label = gtk_label_new (_("Printers"));
  vbox = g_object_new (GTK_TYPE_BOX, "orientation", GTK_ORIENTATION_VERTICAL, "border-width", 12, "spacing", 18, NULL);
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

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 6);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 12);
  gtk_container_add (GTK_CONTAINER (frame), grid);
  gtk_widget_show (grid);

  image = gtk_image_new_from_icon_name ("printer", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 3);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Automatically run a program "
                                                 "when a _printer is connected"));
  xfconf_g_property_bind (channel, "/autoprinter/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  xfconf_g_property_bind (channel, "/autoprinter/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autoprinter/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_grid_attach (GTK_GRID (grid), entry, 1, 1, 1, 1);
  gtk_widget_show (entry);

  /*
     Input Devices
   */
  label = gtk_label_new (_("Input Devices"));
  vbox = g_object_new (GTK_TYPE_BOX, "orientation", GTK_ORIENTATION_VERTICAL, "border-width", 12, "spacing", 18, NULL);
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

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 6);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 12);
  gtk_container_add (GTK_CONTAINER (frame), grid);
  gtk_widget_show (grid);

  image = gtk_image_new_from_icon_name ("input-keyboard", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 3);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Automatically run a program "
                                                 "when a USB _keyboard is connected"));
  xfconf_g_property_bind (channel, "/autokeyboard/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("_Command:"));
  xfconf_g_property_bind (channel, "/autokeyboard/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autokeyboard/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_grid_attach (GTK_GRID (grid), entry, 1, 1, 1, 1);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Mice"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 6);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 12);
  gtk_container_add (GTK_CONTAINER (frame), grid);
  gtk_widget_show (grid);

  image = gtk_image_new_from_icon_name ("input-mouse", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 3);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Automatically run a program "
                                                 "when a USB _mouse is connected"));
  xfconf_g_property_bind (channel, "/automouse/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("C_ommand:"));
  xfconf_g_property_bind (channel, "/automouse/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/automouse/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_grid_attach (GTK_GRID (grid), entry, 1, 1, 1, 1);
  gtk_widget_show (entry);

  frame = g_object_new (GTK_TYPE_FRAME, "border-width", 0, "shadow-type", 
                        GTK_SHADOW_NONE, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (_("Tablet"));
  gtk_label_set_attributes (GTK_LABEL (label), tvm_pango_attr_list_bold ());
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_widget_show (label);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 6);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 12);
  gtk_container_add (GTK_CONTAINER (frame), grid);
  gtk_widget_show (grid);

  image = gtk_image_new_from_icon_name ("input-tablet", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 3);
  gtk_widget_show (image);

  button = gtk_check_button_new_with_mnemonic (_("Automatically run a program "
                                                 "when a _tablet is connected"));
  xfconf_g_property_bind (channel, "/autotablet/enabled", G_TYPE_BOOLEAN, 
                          button, "active");
  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);
  gtk_widget_show (button);

  entry = tvm_command_entry_new_with_label (_("Com_mand:"));
  xfconf_g_property_bind (channel, "/autotablet/enabled", G_TYPE_BOOLEAN, 
                          entry, "sensitive");
  xfconf_g_property_bind (channel, "/autotablet/command", G_TYPE_STRING, 
                          entry, "command");
  gtk_grid_attach (GTK_GRID (grid), entry, 1, 1, 1, 1);
  gtk_widget_show (entry);
}



static void
tvm_preferences_dialog_response (GtkWidget *dialog,
                                 gint response_id)
{
  g_return_if_fail (TVM_IS_PREFERENCES_DIALOG (dialog));

  if (response_id != GTK_RESPONSE_HELP)
    {
      gtk_widget_destroy (dialog);
      gtk_main_quit ();
      return;
    }

  xfce_dialog_show_help_with_version (GTK_WINDOW (dialog), "thunar",
                                      "using-removable-media", NULL,
                                      TVM_VERSION_HELP);
}



GtkWidget *
tvm_preferences_dialog_new (void)
{
  return g_object_new (TVM_TYPE_PREFERENCES_DIALOG, NULL);
}
