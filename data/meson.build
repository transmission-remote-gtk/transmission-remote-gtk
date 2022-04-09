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


po_dir = meson.project_source_root() / 'po'
# desktop file
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
  test('Validate appdata.xml', appstream_util, args: ['validate-relax', appdata_file])
endif

