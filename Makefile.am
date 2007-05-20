# $Id$

SUBDIRS =								\
	icons								\
	po								\
	thunar-volman

distclean-local:
	rm -rf *.spec *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

libexec_SCRIPTS =							\
	thunar-volman-settings

thunar-volman-settings: thunar-volman-settings.in Makefile
	rm -f thunar-volman-settings.gen thunar-volman-settings
	sed -e "s,\@bindir\@,$(bindir),g"				\
		< $(srcdir)/thunar-volman-settings.in			\
		> thunar-volman-settings.gen
	mv thunar-volman-settings.gen thunar-volman-settings

desktopdir = $(datadir)/applications
desktop_in_in_files = thunar-volman-settings.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
%.desktop.in: %.desktop.in.in
	sed -e "s,\@libexecdir\@,$(libexecdir),g" < $< > $@
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	THANKS								\
	intltool-extract.in						\
	intltool-merge.in						\
	intltool-update.in						\
	thunar-volman-settings.in					\
	$(desktop_in_in_files)

DISTCLEANFILES =							\
	intltool-extract						\
	intltool-merge							\
	intltool-update							\
	thunar-volman-settings						\
	$(desktop_in_files)						\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
