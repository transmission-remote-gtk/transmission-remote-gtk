# icons
icon_dir = datadir / 'icons' / 'hicolor'

# scalable
scalable = 'icons' / project_name + '.svg'
install_data(scalable, install_dir: icon_dir / 'scalable' / 'apps')

# regular icons
foreach size: ['16x16', '22x22', '24x24', '32x32', '48x48']
  icon_name = 'icons' / project_name + '-@0@.png'.format(size)
  install_data(icon_name,
    install_dir: icon_dir / size / 'apps',
    rename: project_name + '.png')
endforeach

# man pages
rst2man = find_program('rst2man', required: false)
if rst2man.found()
  man_rst_in = '@0@.1.rst.in'.format(project_name)
  man_rst_out = '@0@.1.rst'.format(project_name)
  man_page = '@0@.1'.format(project_name)

  man_rst = configure_file(input: man_rst_in, output: man_rst_out,
    configuration:
      {'PRETTY_NAME': pretty_name,
       'PROJECT_VERSION': project_version})

  manpages = custom_target('manpages',
    input: man_rst, output: man_page,
    command: [rst2man, '--strict', '--no-raw',
      '--no-generator', '--no-datestamp',
      '@INPUT@', '@OUTPUT@'],
      install: true,
      install_dir: get_option('mandir') / 'man1')
endif

# gresources.xml
gnome = import('gnome')
gresource_xml ='@0@.gresource.xml'.format(app_id)
gresources = gnome.compile_resources('trg-resources', gresource_xml)

# desktop file
po_dir = meson.project_source_root() / 'po'
desktop_file = i18n.merge_file(input: '@0@.desktop.in'.format(app_id),
  output: '@0@.desktop'.format(app_id),
  install: true,
  install_dir: datadir / 'applications',
  po_dir: po_dir,
  type: 'desktop')

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
  test('Validate desktop file', desktop_utils, args: [desktop_file])
endif

# appdata
appdata_file = i18n.merge_file(input: '@0@.appdata.xml.in'.format(app_id),
  output: '@0@.appdata.xml'.format(app_id),
  install: true,
  install_dir: datadir / 'metainfo',
  po_dir: po_dir)

appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
  test('Validate appdata.xml', appstream_util, args: ['validate-relax', '--nonet', appdata_file])
endif
