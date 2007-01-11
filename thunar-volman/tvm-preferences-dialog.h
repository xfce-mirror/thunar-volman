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

#ifndef __TVM_PREFERENCES_DIALOG_H__
#define __TVM_PREFERENCES_DIALOG_H__

#include <thunar-volman/tvm-preferences.h>

G_BEGIN_DECLS;

typedef struct _TvmPreferencesDialogClass TvmPreferencesDialogClass;
typedef struct _TvmPreferencesDialog      TvmPreferencesDialog;

#define TVM_TYPE_PREFERENCES_DIALOG            (tvm_preferences_dialog_get_type ())
#define TVM_PREFERENCES_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TVM_TYPE_PREFERENCES_DIALOG, TvmPreferencesDialog))
#define TVM_PREFERENCES_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TVM_TYPE_PREFERENCES_DIALOG, TvmPreferencesDialogClass))
#define TVM_IS_PREFERENCES_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TVM_TYPE_PREFERENCES_DIALOG))
#define TVM_IS_PREFERENCES_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TVM_TYPE_PREFERENCES_DIALOG))
#define TVM_PREFERENCES_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TVM_TYPE_PREFERENCES_DIALOG, TvmPreferencesDialogClass))

GType      tvm_preferences_dialog_get_type (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget *tvm_preferences_dialog_new      (void) G_GNUC_INTERNAL G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS;

#endif /* !__TVM_PREFERENCES_DIALOG_H__ */
