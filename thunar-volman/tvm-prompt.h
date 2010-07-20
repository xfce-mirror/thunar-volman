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

#ifndef __TVM_PROMPT_H__
#define __TVM_PROMPT_H__

#include <glib.h>

#include <thunar-volman/tvm-context.h>

G_BEGIN_DECLS

enum
{
  TVM_RESPONSE_NONE,
  TVM_RESPONSE_PLAY,
  TVM_RESPONSE_MUSIC,
  TVM_RESPONSE_BROWSE,
  TVM_RESPONSE_PHOTOS,
  TVM_RESPONSE_AUTORUN,
  TVM_RESPONSE_BURN_DATA_CD,
  TVM_RESPONSE_BURN_AUDIO_CD,
};

gint tvm_prompt (TvmContext  *context,
                 const gchar *icon,
                 const gchar *title,
                 const gchar *primary_text,
                 const gchar *secondary_text,
                 const gchar *first_button_text,
                 ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS

#endif /* !__TVM_PROMPT_H__ */
