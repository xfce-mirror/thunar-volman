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

#ifndef __TVM_DEVICE_H__
#define __TVM_DEVICE_H__

#include <libhal.h>

#include <exo-hal/exo-hal.h>

#include <thunar-volman/tvm-preferences.h>

G_BEGIN_DECLS

/**
 * TvmDeviceCallback:
 * @preferences : a #TvmPreferences.
 * @context     : a #LibHalContext.
 * @udi         : the HAL device UDI of the newly added device.
 * @capability  : the capability that caused this callback to be invoked.
 * @error       : return location for errors or %NULL.
 *
 * Prototype for device callbacks, which are invoked if a device is inserted,
 * with a capability they claim to be able to handle. Returns %TRUE if the
 * device was handled, %FALSE if the device cannot be handled or an unrecoverable
 * error occurred (in which case the @error should be set).
 *
 * Return value: 
 **/
typedef gboolean (*TvmDeviceCallback) (TvmPreferences *preferences,
                                       LibHalContext  *context,
                                       const gchar    *udi,
                                       const gchar    *capability,
                                       GError        **error);

gboolean tvm_device_added (TvmPreferences *preferences,
                           const gchar    *udi,
                           GError        **error) G_GNUC_INTERNAL;

G_END_DECLS

#endif /* !__TVM_DEVICE_H__ */
