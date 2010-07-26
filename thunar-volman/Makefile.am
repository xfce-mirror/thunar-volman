# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
# -
# Copyright (c) 2010 Jannis Pohlmann <jannis@xfce.org>
# 
# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of 
# the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public 
# License along with this program; if not, write to the Free 
# Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

INCLUDES =								\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DBINDIR=\"$(bindir)\"						\
	-DG_LOG_DOMAIN=\"thunar-volman\"				\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS =								\
	thunar-volman

if HAVE_LIBNOTIFY
tvm_notify_sources =							\
	tvm-notify.c							\
	tvm-notify.h
endif

thunar_volman_SOURCES =							\
	main.c								\
	tvm-block-device.c						\
	tvm-block-device.h						\
	tvm-context.c							\
	tvm-context.h							\
	tvm-device.c							\
	tvm-device.h							\
	tvm-gio-extensions.c						\
	tvm-gio-extensions.h						\
	tvm-input-device.c						\
	tvm-input-device.h						\
	tvm-pango-extensions.c						\
	tvm-pango-extensions.h						\
	tvm-prompt.c							\
	tvm-prompt.h							\
	tvm-run.c							\
	tvm-run.h							\
	tvm-usb-device.c						\
	tvm-usb-device.h						\
	$(tvm_notify_sources)

thunar_volman_CFLAGS =							\
	$(GLIB_CFLAGS)							\
	$(GIO_CFLAGS)							\
	$(GTHREAD_CFLAGS)						\
	$(GTK_CFLAGS)							\
	$(GUDEV_CFLAGS)							\
	$(LIBNOTIFY_CFLAGS)						\
	$(LIBXFCE4UI_CFLAGS)						\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(XFCONF_CFLAGS)						\
	$(PLATFORM_CFLAGS)

thunar_volman_LDFLAGS =							\
	-no-undefined							\
	$(PLATFORM_LDFLAGS)

thunar_volman_LDADD =							\
	$(GLIB_LIBS)							\
	$(GIO_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GTK_LIBS)							\
	$(GUDEV_LIBS)							\
	$(LIBNOTIFY_LIBS)						\
	$(LIBXFCE4UI_LIBS)						\
	$(LIBXFCE4UTIL_LIBS)						\
	$(XFCONF_LIBS)
