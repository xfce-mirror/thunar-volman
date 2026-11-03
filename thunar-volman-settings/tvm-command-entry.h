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

#define TVM_TYPE_COMMAND_ENTRY (tvm_command_entry_get_type ())
G_DECLARE_FINAL_TYPE (TvmCommandEntry, tvm_command_entry, TVM, COMMAND_ENTRY, GtkBox)

GtkWidget   *tvm_command_entry_new            (void) G_GNUC_INTERNAL G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
GtkWidget   *tvm_command_entry_new_with_label (const gchar     *label) G_GNUC_INTERNAL G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

const gchar *tvm_command_entry_get_command    (TvmCommandEntry *command_entry) G_GNUC_INTERNAL;
void         tvm_command_entry_set_command    (TvmCommandEntry *command_entry,
                                               const gchar     *command) G_GNUC_INTERNAL;

GtkWidget   *tvm_command_entry_get_label      (TvmCommandEntry *command_entry) G_GNUC_INTERNAL;

G_END_DECLS

#endif /* !__TVM_COMMAND_ENTRY_H__ */
