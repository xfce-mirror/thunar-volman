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

#ifndef __TVM_COMMAND_ENTRY_H__
#define __TVM_COMMAND_ENTRY_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _TvmCommandEntryClass TvmCommandEntryClass;
typedef struct _TvmCommandEntry      TvmCommandEntry;

#define TVM_TYPE_COMMAND_ENTRY            (tvm_command_entry_get_type ())
#define TVM_COMMAND_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TVM_TYPE_COMMAND_ENTRY, TvmCommandEntry))
#define TVM_COMMAND_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TVM_TYPE_COMMAND_ENTRY, TvmCommandEntryClass))
#define TVM_IS_COMMAND_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TVM_TYPE_COMMAND_ENTRY))
#define TVM_IS_COMMAND_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TVM_TYPE_COMMAND_ENTRY))
#define TVM_COMMAND_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TVM_TYPE_COMMAND_ENTRY, TvmCommandEntryClass))

struct _TvmCommandEntryClass
{
  GtkHBoxClass __parent__;
};

struct _TvmCommandEntry
{
  GtkHBox    __parent__;
  GtkWidget *entry;
  GtkWidget *label;
  gchar     *command;
};

GType        tvm_command_entry_get_type       (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget   *tvm_command_entry_new            (void) G_GNUC_INTERNAL G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
GtkWidget   *tvm_command_entry_new_with_label (const gchar     *label) G_GNUC_INTERNAL G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

const gchar *tvm_command_entry_get_command    (TvmCommandEntry *command_entry) G_GNUC_INTERNAL;
void         tvm_command_entry_set_command    (TvmCommandEntry *command_entry,
                                               const gchar     *command) G_GNUC_INTERNAL;

G_END_DECLS

#endif /* !__TVM_COMMAND_ENTRY_H__ */
