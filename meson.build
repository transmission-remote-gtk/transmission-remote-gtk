project('transmission-remote-gtk', 'c',
        license: 'GPL2+',
        version: '1.4.2',
        meson_version: '>= 0.59.0',
        default_options: ['b_pie=true',
                          'c_std=gnu99',
                          'warning_level=2',
                          'buildtype=debugoptimized']
)

project_name = meson.project_name()
project_version = meson.project_version()
app_id = 'io.github.TransmissionRemoteGtk'
datadir = get_option('datadir')

# i18n module
i18n = import('i18n')


# glib version info
glib_version = '2.44'
glib_max_version = glib_version
glib_min_version = glib_version
glib_version_str = '>= @0@'.format(glib_version)

# required dependencies
gtk_dep     = dependency('gtk+-3.0', version: '>= 3.16')
gio_dep     = dependency('gio-2.0', version: glib_version_str)
glib_dep    = dependency('glib-2.0', version: glib_version_str)
json_dep    = dependency('json-glib-1.0', version: '>= 0.8')
libcurl_dep = dependency('libcurl')
gthread_dep = dependency('gthread-2.0', version: glib_version_str)

trg_deps = [gtk_dep, glib_dep, gio_dep, json_dep, libcurl_dep, gthread_dep]

# optional dependencies
rss_dep             = dependency('mrss', version: '>=0.18', required: get_option('mrss'))
geoip_dep           = dependency('geoip', required: get_option('geoip'))
libproxy_dep        = dependency('libproxy', required: get_option('libproxy'))
libnotify_dep       = dependency('libnotify', required: get_option('libnotify'))
libappindicator_dep = dependency('appindicator3-0.1', required: get_option('libappindicator'))

trg_deps += [geoip_dep, libproxy_dep, libnotify_dep, libappindicator_dep]

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
conf_data.set_quoted('LOCALEDIR', get_option('localedir'))

# debug options
debug = get_option('debug')
conf_data.set('DEBUG', debug)
conf_data.set('G_ENABLE_DEBUG', debug)
conf_data.set('G_DISABLE_ASSERT', not debug)
conf_data.set('G_DISABLE_CHECKS', not debug)
conf_data.set('G_DISABLE_CAST_CHECKS', not debug)

# glib API configuration
maj_min = glib_max_version.split('.')
conf_data.set('GLIB_VERSION_MAX_ALLOWED',
              'GLIB_VERSION_@0@_@1@'.format(maj_min[0], maj_min[1]))
maj_min = glib_min_version.split('.')
conf_data.set('GLIB_VERSION_MIN_REQUIRED',
              'GLIB_VERSION_@0@_@1@'.format(maj_min[0], maj_min[1]))

# compile time features
conf_data.set('HAVE_RSS', rss_dep.found())
# TODO: why??
conf_data.set10('HAVE_GEOIP', geoip_dep.found())
conf_data.set('HAVE_LIBPROXY', libproxy_dep.found())
conf_data.set('HAVE_LIBNOTIFY', libnotify_dep.found())
conf_data.set('ENABLE_NL_LANGINFO', nl_langinfo)
conf_data.set('HAVE_LIBAPPINDICATOR', libappindicator_dep.found())

# compiler/linker flags
flags = ['-funsigned-char',
         '-fstack-protector-strong',
         '-Winline',
         '-Wno-padded',
         '-Wno-unused-parameter',
         '-Wstrict-prototypes',
         '-Wmissing-prototypes',
         '-Werror=format=2',
         '-Werror=implicit-function-declaration',
         '-Werror=pointer-arith',
         '-Werror=format-security',
         '-Werror=init-self',
         '-Werror=missing-include-dirs',
         '-Werror=date-time',
         '-Wno-conversion',
         '-Wno-sign-compare',
         '-Wno-deprecated-declarations',
         '-Wno-deprecated',
         '-Wno-error=format-nonliteral']

cflags = cc.get_supported_arguments(flags)
add_project_arguments(cflags, language: 'c')
add_project_arguments('-DHAVE_CONFIG_H', language: 'c')

# link arguments
ldflags = cc.get_supported_link_arguments('-Wl,-z,relro',
                                          '-Wl,-z,now')
add_project_link_arguments(ldflags, language: 'c')

# Windows compiler options
win32 = host_machine.system() == 'windows' or host_machine.system() == 'cgywin'
conf_data.set('WIN32', win32)
if win32
  win_cflags = cc.get_supported_arguments('-mms-bitfields',
                                          '-mwin32',
                                          '-mwindows')
  add_project_arguments(win_cflags, language: 'c')

  win_ldflags = cc.get_supported_link_arguments('-Wl,--allow-multiple-definitions',
                                                '-lws2_32',
                                                '-lintl')
  add_project_link_arguments(win_ldflags, language: 'c')
endif

subdir('po')
subdir('data')
if rss_dep.found()
  subdir('extern/rss-glib')
endif
subdir('src')