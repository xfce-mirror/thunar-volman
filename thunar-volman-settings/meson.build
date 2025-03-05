volman_settings_sources = [
  '../thunar-volman/tvm-pango-extensions.c',
  '../thunar-volman/tvm-pango-extensions.h',
  'main.c',
  'tvm-command-entry.c',
  'tvm-command-entry.h',
  'tvm-preferences-dialog.c',
  'tvm-preferences-dialog.h',
]

volman_settings = executable(
  'thunar-volman-settings',
  volman_settings_sources,
  sources: xfce_revision_h,
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    glib,
    gthread,
    gtk,
    exo,
    libxfce4util,
    libxfce4ui,
    xfconf,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('bindir'),
)

i18n.merge_file(
    input: 'thunar-volman-settings.desktop.in.in',
    output: 'thunar-volman-settings.desktop',
    po_dir: '..' / 'po',
    type: 'desktop',
    install: true,
    install_dir: get_option('prefix') / get_option('datadir') / 'applications',
)
