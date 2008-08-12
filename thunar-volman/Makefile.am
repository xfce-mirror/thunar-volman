# $Id$

INCLUDES =								\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DBINDIR=\"$(bindir)\"						\
	-DEXO_API_SUBJECT_TO_CHANGE					\
	-DEXO_DISABLE_DEPRECATED					\
	-DG_LOG_DOMAIN=\"thunar-volman\"				\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS =								\
	thunar-volman

thunar_volman_SOURCES =							\
	main.c								\
	tvm-block-device.c						\
	tvm-block-device.h						\
	tvm-camera-device.c						\
	tvm-camera-device.h						\
	tvm-command-entry.c						\
	tvm-command-entry.h						\
	tvm-crypto-volume.c						\
	tvm-crypto-volume.h						\
	tvm-device.c							\
	tvm-device.h							\
	tvm-input-device.c						\
	tvm-input-device.h						\
	tvm-pango-extensions.c						\
	tvm-pango-extensions.h						\
	tvm-pda-device.c						\
	tvm-pda-device.h						\
	tvm-preferences-dialog.c					\
	tvm-preferences-dialog.h					\
	tvm-preferences.c						\
	tvm-preferences.h						\
	tvm-printer-device.c						\
	tvm-printer-device.h						\
	tvm-prompt.c							\
	tvm-prompt.h							\
	tvm-run.c							\
	tvm-run.h							\
	xfce-heading.c							\
	xfce-heading.h							\
	xfce-titled-dialog.c						\
	xfce-titled-dialog.h

thunar_volman_CFLAGS =							\
	$(DBUS_CFLAGS)							\
	$(EXO_HAL_CFLAGS)						\
	$(HAL_CFLAGS)							\
	$(THUNAR_VFS_CFLAGS)						\
	$(PLATFORM_CFLAGS)

thunar_volman_LDFLAGS =							\
	-no-undefined							\
	$(PLATFORM_LDFLAGS)

thunar_volman_LDADD =							\
	$(DBUS_LIBS)							\
	$(EXO_HAL_LIBS)							\
	$(HAL_LIBS)							\
	$(THUNAR_VFS_LIBS)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
