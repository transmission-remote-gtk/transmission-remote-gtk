project('transmission-remote-gtk', 'c',
  license: 'GPL2+',
  version: '1.7.0',
  meson_version: '>= 0.60.0',
  default_options:[
    'b_pie=true',
    'c_std=gnu99',
    'warning_level=2',
    'buildtype=debugoptimized'
  ]
)

project_name = meson.project_name()
project_version = meson.project_version()
app_id = 'io.github.TransmissionRemoteGtk'
resource_prefix = '/' + app_id.replace('.', '/')
pretty_name = 'Transmission Remote GTK'
datadir = get_option('datadir')

# glib version info
glib_version = '2.70'
glib_min_version = glib_version
glib_version_str = '>= @0@'.format(glib_version)

# glib API configuration
maj_min = glib_min_version.split('.')
glib_min = 'GLIB_VERSION_@0@_@1@'.format(maj_min[0], maj_min[1])
glib_min_def = '-DGLIB_VERSION_MIN_REQUIRED=@0@'.format(glib_min)

add_project_arguments(glib_min_def, language: 'c')

# required dependencies
gtk_dep     = dependency('gtk+-3.0', version: '>= 3.22')
gio_dep     = dependency('gio-2.0', version: glib_version_str)
glib_dep    = dependency('glib-2.0', version: glib_version_str)
json_dep    = dependency('json-glib-1.0', version: '>= 1.2.8')
libsoup_dep = dependency('libsoup-3.0', version: '>= 3.0')
gthread_dep = dependency('gthread-2.0', version: glib_version_str)

trg_deps = [gtk_dep, glib_dep, gio_dep, json_dep, libsoup_dep, gthread_dep]

# optional dependencies
libappindicator_dep = dependency('ayatana-appindicator3-0.1',
  'appindicator3-0.1', required: get_option('libappindicator'))

trg_deps += [libappindicator_dep]

# compiler
cc = meson.get_compiler('c')

# compiler deps
trg_deps += cc.find_library('m', required: false)

# nl_langinfo check
nl_langinfo = cc.has_function('nl_langinfo', prefix: '#include <langinfo.h>')
nl_li_feature = get_option('nl_langinfo')
if nl_li_feature.enabled() and not nl_langinfo
  error('nl_langinfo() specified but not found')
endif
nl_langinfo = nl_langinfo and nl_li_feature.allowed()


# config data
conf_data = configuration_data()

conf_data.set_quoted('PACKAGE_NAME', project_name)
conf_data.set_quoted('PACKAGE_VERSION', project_version)
conf_data.set_quoted('GETTEXT_PACKAGE', project_name)
conf_data.set_quoted('APPLICATION_ID', app_id)
conf_data.set_quoted('LOCALEDIR',
  get_option('prefix') / get_option('localedir'))

# debug options
debug = get_option('debug')
conf_data.set('DEBUG', debug)
conf_data.set('G_ENABLE_DEBUG', debug)
conf_data.set('G_DISABLE_ASSERT', not debug)
conf_data.set('G_DISABLE_CHECKS', not debug)
conf_data.set('G_DISABLE_CAST_CHECKS', not debug)

# compile time features
conf_data.set10('ENABLE_NL_LANGINFO', nl_langinfo)

conf_data.set10('HAVE_LIBAPPINDICATOR', libappindicator_dep.found())
conf_data.set10('HAVE_LIBAYATANA_APPINDICATOR',
  libappindicator_dep.found() and libappindicator_dep.name() == 'ayatana-appindicator3-0.1')

# compiler/linker flags
flags = [
  '-funsigned-char',
  '-fstack-protector-strong',
  '-Winline',
  '-Wno-unused-parameter',
  '-Wstrict-prototypes',
  '-Wmissing-prototypes',
  '-Werror=format=2',
  '-Werror=implicit-function-declaration',
  '-Werror=pointer-arith',
  '-Werror=init-self',
  '-Werror=missing-include-dirs',
  '-Werror=date-time'
]

cflags = cc.get_supported_arguments(flags)
add_project_arguments(cflags, language: 'c')

# link arguments
ldflags = cc.get_supported_link_arguments('-Wl,-z,relro', '-Wl,-z,now')
add_project_link_arguments(ldflags, language: 'c')

# Windows compiler options
if host_machine.system() == 'windows' or host_machine.system() == 'cygwin'
  win_cflags = cc.get_supported_arguments(
    '-mms-bitfields', '-mwin32', '-mwindows')
  add_project_arguments(win_cflags, language: 'c')

  win_ldflags = cc.get_supported_link_arguments(
    '-Wl,--allow-multiple-definition', '-lws2_32', '-lintl', '-lssp')
  add_project_link_arguments(win_ldflags, language: 'c')
endif

subdir('po')
subdir('data')
subdir('src')
