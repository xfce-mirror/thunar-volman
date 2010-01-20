# $Id$

INCLUDES =								\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DBINDIR=\"$(bindir)\"						\
	-DEXO_API_SUBJECT_TO_CHANGE					\
	-DEXO_DISABLE_DEPRECATED					\
	-DG_LOG_DOMAIN=\"thunar-volman\"				\
	-DG_UDEV_API_IS_SUBJECT_TO_CHANGE				\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS =								\
	thunar-volman

thunar_volman_SOURCES =							\
	main.c								\
	tvm-block-device.h						\
	tvm-block-device.c						\
	tvm-device.h							\
	tvm-device.c							\
	tvm-input-device.h						\
	tvm-input-device.c						\
	tvm-pango-extensions.h						\
	tvm-pango-extensions.c						\
	tvm-prompt.h							\
	tvm-prompt.c							\
	tvm-run.h							\
	tvm-run.c

thunar_volman_CFLAGS =							\
	$(DBUS_CFLAGS)							\
	$(GIO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GTK_CFLAGS)							\
	$(GTHREAD_CFLAGS)						\
	$(GUDEV_CFLAGS)							\
	$(LIBXFCE4UI_CFLAGS)						\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(XFCONF_CFLAGS)						\
	$(PLATFORM_CFLAGS)

thunar_volman_LDFLAGS =							\
	-no-undefined							\
	$(PLATFORM_LDFLAGS)

thunar_volman_LDADD =							\
	$(DBUS_LIBS)							\
	$(GIO_LIBS)							\
	$(GLIB_LIBS)							\
	$(GTK_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GUDEV_LIBS)							\
	$(LIBXFCE4UI_LIBS)						\
	$(LIBXFCE4UTIL_LIBS)						\
	$(XFCONF_LIBS)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
