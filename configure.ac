dnl Copyright (c) 2004-2007 Benedikt Meurer <benny@xfce.org>
dnl Copyright (c) 2010-2011 Jannis Pohlmann <jannis@xfce.org>
dnl
dnl This program is free software; you can redistribute it and/or 
dnl modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation; either version 2 of 
dnl the License, or (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public 
dnl License along with this program; if not, write to the Free 
dnl Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
dnl Boston, MA 02110-1301, USA.

dnl ***************************
dnl *** Version information ***
dnl ***************************
XDT_VERSION_INIT([4.20.0])

dnl ***************************
dnl *** Initialize autoconf ***
dnl ***************************
AC_COPYRIGHT([Copyright (c) 2004-2024 The Thunar development team. All rights reserved.
        
Written for thunar-volman by Benedikt Meurer <benny@xfce.org>
and Jannis Pohlmann <jannis@xfce.org>.])
AC_INIT([thunar-volman], [xdt_version], [https://gitlab.xfce.org/xfce/thunar-volman])
AC_PREREQ([2.69])
AC_CONFIG_MACRO_DIRS([m4])
AC_CANONICAL_TARGET()
AC_REVISION([xdt_version_build])

dnl ***************************
dnl *** Initialize automake ***
dnl ***************************
AM_INIT_AUTOMAKE([1.11 dist-bzip2 tar-ustar no-dist-gzip foreign subdir-objects])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE()
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl *******************************
dnl *** Check for UNIX variants ***
dnl *******************************
AC_USE_SYSTEM_EXTENSIONS
AC_SEARCH_LIBS([strerror],[cposix])

dnl ********************************
dnl *** Check for basic programs ***
dnl ********************************
AC_PROG_CC()
AC_PROG_CC_C_O()
LT_PATH_LD()
AC_PROG_INSTALL()

dnl **************************
dnl *** Initialize libtool ***
dnl **************************
LT_PREREQ([2.4])
LT_INIT([disable-static])

dnl **********************************
dnl *** Check for standard headers ***
dnl **********************************
AC_CHECK_HEADERS([errno.h path.h stdarg.h sys/types.h sys/wait.h])

dnl ************************************
dnl *** Check for standard functions ***
dnl ************************************
AC_CHECK_FUNCS([])

dnl ******************************
dnl *** Check for i18n support ***
dnl ******************************
GETTEXT_PACKAGE="$PACKAGE"
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of default gettext domain])
AC_SUBST([GETTEXT_PACKAGE])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

dnl ***********************************
dnl *** Check for required packages ***
dnl ***********************************
XDT_CHECK_PACKAGE([EXO], [exo-2], [0.10.0])
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.72.0])
XDT_CHECK_PACKAGE([GIO], [gio-2.0], [2.72.0])
XDT_CHECK_PACKAGE([GTHREAD], [gthread-2.0], [2.72.0])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.24.0])
XDT_CHECK_PACKAGE([GUDEV], [gudev-1.0], [145])
XDT_CHECK_PACKAGE([LIBXFCE4UI], [libxfce4ui-2], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4UTIL], [libxfce4util-1.0], [4.12.0])
XDT_CHECK_PACKAGE([XFCONF], [libxfconf-0], [4.12.0])

dnl *******************************************************
dnl *** Optional mount notification support (libnotify) ***
dnl *******************************************************
XDT_CHECK_OPTIONAL_PACKAGE([LIBNOTIFY], [libnotify], [0.7.0], [notifications],
                           [Notifications support], [yes])

dnl ***************************************************
dnl *** define a version string for the help dialog ***
dnl ***************************************************
m4_append([tvm_version_help], ["])
m4_append([tvm_version_help], [xdt_version_major])
m4_append([tvm_version_help], [.])
m4_append([tvm_version_help], [xdt_version_minor])
m4_append([tvm_version_help], ["])
AC_DEFINE([TVM_VERSION_HELP], [tvm_version_help], [Version string for help dialog])

dnl ***********************************
dnl *** Check for debugging support ***
dnl ***********************************
XDT_FEATURE_DEBUG([xdt_debug_default])

dnl **************************************
dnl *** Check for linker optimizations ***
dnl **************************************
XDT_FEATURE_LINKER_OPTS()

dnl *********************************
dnl *** Substitute platform flags ***
dnl *********************************
AC_MSG_CHECKING([PLATFORM_CPPFLAGS])
AC_MSG_RESULT([$PLATFORM_CPPFLAGS])
AC_SUBST([PLATFORM_CPPFLAGS])
AC_MSG_CHECKING([PLATFORM_CFLAGS])
AC_MSG_RESULT([$PLATFORM_CFLAGS])
AC_SUBST([PLATFORM_CFLAGS])
AC_MSG_CHECKING([PLATFORM_LDFLAGS])
AC_MSG_RESULT([$PLATFORM_LDFLAGS])
AC_SUBST([PLATFORM_LDFLAGS])

AC_CONFIG_FILES([
Makefile
icons/Makefile
icons/16x16/Makefile
icons/24x24/Makefile
icons/32x32/Makefile
icons/48x48/Makefile
icons/64x64/Makefile
icons/96x96/Makefile
icons/128x128/Makefile
icons/scalable/Makefile
po/Makefile.in
thunar-volman/Makefile
thunar-volman-settings/Makefile
])
AC_OUTPUT

dnl ***************************
dnl *** Print configuration ***
dnl ***************************
echo
echo "Build Configuration:"
echo
if test x"$LIBNOTIFY_FOUND" = x"yes"; then
echo "* Mount notifications: yes"
else
echo "* Mount notifications: no"
fi
echo "* Debug Support:       $enable_debug"
echo
