/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@xfce.org>
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

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>

#include <gtk/gtk.h>

#include <exo/exo.h>

#include <thunar-volman-settings/tvm-command-entry.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_COMMAND,
};



static void tvm_command_entry_class_init    (TvmCommandEntryClass *klass);
static void tvm_command_entry_init          (TvmCommandEntry      *command_entry);
static void tvm_command_entry_finalize      (GObject              *object);
static void tvm_command_entry_get_property  (GObject              *object,
                                             guint                 prop_id,
                                             GValue               *value,
                                             GParamSpec           *pspec);
static void tvm_command_entry_set_property  (GObject              *object,
                                             guint                 prop_id,
                                             const GValue         *value,
                                             GParamSpec           *pspec);
static void tvm_command_entry_clicked       (GtkWidget            *button,
                                             TvmCommandEntry      *command_entry);



static GObjectClass *tvm_command_entry_parent_class;



GType
tvm_command_entry_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (TvmCommandEntryClass),
        NULL,
        NULL,
        (GClassInitFunc) tvm_command_entry_class_init,
        NULL,
        NULL,
        sizeof (TvmCommandEntry),
        0,
        (GInstanceInitFunc) tvm_command_entry_init,
        NULL,
      };

      type = g_type_register_static (GTK_TYPE_HBOX, I_("TvmCommandEntry"), &info, 0);
    }

  return type;
}



static void
tvm_command_entry_class_init (TvmCommandEntryClass *klass)
{
  GObjectClass *gobject_class;

  /* determine the parent type class */
  tvm_command_entry_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = tvm_command_entry_finalize;
  gobject_class->get_property = tvm_command_entry_get_property;
  gobject_class->set_property = tvm_command_entry_set_property;

  /**
   * TvmCommandEntry:command:
   *
   * The command currently entered into this command entry widget.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_COMMAND,
                                   g_param_spec_string ("command",
                                                        "command",
                                                        "command",
                                                        NULL,
                                                        EXO_PARAM_READWRITE));
}



static void
tvm_command_entry_init (TvmCommandEntry *command_entry)
{
  GtkWidget *button;
  GtkWidget *align;
  GtkWidget *image;

  gtk_box_set_spacing (GTK_BOX (command_entry), 2);

  align = g_object_new (GTK_TYPE_ALIGNMENT, "width-request", 10, NULL);
  gtk_box_pack_start (GTK_BOX (command_entry), align, FALSE, FALSE, 0);
  gtk_widget_show (align);

  command_entry->label = g_object_new (GTK_TYPE_LABEL, "use-underline", TRUE, "xalign", 0.0f, NULL);
  gtk_box_pack_start (GTK_BOX (command_entry), command_entry->label, FALSE, FALSE, 10);
  gtk_widget_show (command_entry->label);

  command_entry->entry = gtk_entry_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (command_entry->label), command_entry->entry);
  exo_mutual_binding_new (G_OBJECT (command_entry->entry), "text", G_OBJECT (command_entry), "command");
  gtk_box_pack_start (GTK_BOX (command_entry), command_entry->entry, TRUE, TRUE, 0);
  gtk_widget_show (command_entry->entry);

  button = gtk_button_new ();
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (tvm_command_entry_clicked), command_entry);
  gtk_box_pack_start (GTK_BOX (command_entry), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  image = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_BUTTON);
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_widget_show (image);
}



static void
tvm_command_entry_finalize (GObject *object)
{
  TvmCommandEntry *command_entry = TVM_COMMAND_ENTRY (object);

  /* cleanup properties */
  g_free (command_entry->command);

  (*G_OBJECT_CLASS (tvm_command_entry_parent_class)->finalize) (object);
}



static void
tvm_command_entry_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  TvmCommandEntry *command_entry = TVM_COMMAND_ENTRY (object);

  switch (prop_id)
    {
    case PROP_COMMAND:
      g_value_set_string (value, tvm_command_entry_get_command (command_entry));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
tvm_command_entry_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  TvmCommandEntry *command_entry = TVM_COMMAND_ENTRY (object);

  switch (prop_id)
    {
    case PROP_COMMAND:
      tvm_command_entry_set_command (command_entry, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}




static void
tvm_command_entry_clicked (GtkWidget       *button,
                           TvmCommandEntry *command_entry)
{
  GtkFileFilter *filter;
  GtkWidget     *toplevel;
  GtkWidget     *chooser;
  gchar         *filename;
  gchar         *s;

  g_return_if_fail (GTK_IS_BUTTON (button));
  g_return_if_fail (TVM_IS_COMMAND_ENTRY (command_entry));

  /* determine the toplevel widget */
  toplevel = gtk_widget_get_toplevel (button);
  if (toplevel == NULL || !GTK_WIDGET_TOPLEVEL (toplevel))
    return;

  chooser = gtk_file_chooser_dialog_new (_("Select an Application"),
                                         GTK_WINDOW (toplevel),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);
  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (chooser), TRUE);

  /* add file chooser filters */
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("All Files"));
  gtk_file_filter_add_pattern (filter, "*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Executable Files"));
  gtk_file_filter_add_mime_type (filter, "application/x-csh");
  gtk_file_filter_add_mime_type (filter, "application/x-executable");
  gtk_file_filter_add_mime_type (filter, "application/x-perl");
  gtk_file_filter_add_mime_type (filter, "application/x-python");
  gtk_file_filter_add_mime_type (filter, "application/x-ruby");
  gtk_file_filter_add_mime_type (filter, "application/x-shellscript");
  gtk_file_filter_add_pattern (filter, "*.pl");
  gtk_file_filter_add_pattern (filter, "*.py");
  gtk_file_filter_add_pattern (filter, "*.rb");
  gtk_file_filter_add_pattern (filter, "*.sh");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Perl Scripts"));
  gtk_file_filter_add_mime_type (filter, "application/x-perl");
  gtk_file_filter_add_pattern (filter, "*.pl");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Python Scripts"));
  gtk_file_filter_add_mime_type (filter, "application/x-python");
  gtk_file_filter_add_pattern (filter, "*.py");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Ruby Scripts"));
  gtk_file_filter_add_mime_type (filter, "application/x-ruby");
  gtk_file_filter_add_pattern (filter, "*.rb");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Shell Scripts"));
  gtk_file_filter_add_mime_type (filter, "application/x-csh");
  gtk_file_filter_add_mime_type (filter, "application/x-shellscript");
  gtk_file_filter_add_pattern (filter, "*.sh");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  /* use the bindir as default folder */
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), BINDIR);

  /* setup the currently selected file */
  g_object_get (G_OBJECT (command_entry), "command", &filename, NULL);
  if (G_LIKELY (filename != NULL))
    {
      /* use only the first argument */
      s = strchr (filename, ' ');
      if (G_UNLIKELY (s != NULL))
        *s = '\0';

      /* check if we have a file name */
      if (G_LIKELY (*filename != '\0'))
        {
          /* check if the filename is not an absolute path */
          if (G_LIKELY (!g_path_is_absolute (filename)))
            {
              /* try to lookup the filename in $PATH */
              s = g_find_program_in_path (filename);
              if (G_LIKELY (s != NULL))
                {
                  /* use the absolute path instead */
                  g_free (filename);
                  filename = s;
                }
            }

          /* check if we have an absolute path now */
          if (G_LIKELY (g_path_is_absolute (filename)))
            gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (chooser), filename);
        }

      /* release the filename */
      g_free (filename);
    }

  /* run the chooser dialog */
  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
      tvm_command_entry_set_command (command_entry, filename);
      g_free (filename);
    }

  gtk_widget_destroy (chooser);
}



/**
 * tvm_command_entry_new:
 *
 * Allocates a new #TvmCommandEntry instance.
 *
 * Return value: the newly allocated #TvmCommandEntry.
 **/
GtkWidget*
tvm_command_entry_new (void)
{
  return g_object_new (TVM_TYPE_COMMAND_ENTRY, NULL);
}



/**
 * tvm_command_entry_new:
 * @label : the label for the command entry.
 *
 * Allocates a new #TvmCommandEntry instance with the @label.
 *
 * Return value: the newly allocated #TvmCommandEntry.
 **/
GtkWidget*
tvm_command_entry_new_with_label (const gchar *label)
{
  TvmCommandEntry *entry;

  g_return_val_if_fail (label == NULL || g_utf8_validate (label, -1, NULL), NULL);

  entry = g_object_new (TVM_TYPE_COMMAND_ENTRY, NULL);
  if (G_LIKELY (label != NULL))
    g_object_set (G_OBJECT (entry->label), "label", label, "use-underline", TRUE, NULL);
  return GTK_WIDGET (entry);
}



/**
 * tvm_command_entry_get_command:
 * @command_entry : a #TvmCommandEntry.
 *
 * Returns the command of the @command_entry.
 *
 * Return value: the command in the @command_entry.
 **/
const gchar*
tvm_command_entry_get_command (TvmCommandEntry *command_entry)
{
  g_return_val_if_fail (TVM_IS_COMMAND_ENTRY (command_entry), NULL);
  return command_entry->command;
}



/**
 * tvm_command_entry_set_command:
 * @command_entry : a #TvmCommandEntry.
 * @command       : the new command.
 *
 * Sets the command in @command_entry to @command.
 **/
void
tvm_command_entry_set_command (TvmCommandEntry *command_entry,
                               const gchar     *command)
{
  g_return_if_fail (TVM_IS_COMMAND_ENTRY (command_entry));
  g_return_if_fail (g_utf8_validate (command, -1, NULL));

  /* update to the new command */
  g_free (command_entry->command);
  command_entry->command = g_strdup (command);
  g_object_notify (G_OBJECT (command_entry), "command");
}

