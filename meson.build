project('rpminspect',
        'c',
        version : '1.0',
        default_options : [
            'c_std=c99',
            'warning_level=2',
            'buildtype=debugoptimized'
        ],
        license : 'GPLv3+')

cc = meson.get_compiler('c')

# Pull in all the usual glibc defines
add_global_arguments('-D_GNU_SOURCE', language : 'c')
add_global_arguments('-DPACKAGE_VERSION="@0@"'.format(meson.project_version()), language : 'c')

# Translations
if find_program('xgettext', required : get_option('nls')).found()
    add_global_arguments('-DGETTEXT_DOMAIN="' + meson.project_name() + '"', language : 'c')
    subdir('po')
endif

# Dependencies that use pkg-config
jsonc = dependency('json-c', method : 'pkg-config', required : true)
xmlrpc = dependency('xmlrpc xmlrpc_util xmlrpc_client', method : 'pkg-config', version : '>=1.32.5', required : true)
libxml = dependency('libxml-2.0', method : 'pkg-config', required : true)
rpm = dependency('rpm', method : 'pkg-config', required : true)
libarchive = dependency('libarchive', method : 'pkg-config', required : true)
libelf = dependency('libelf', method : 'pkg-config', required : true)
libkmod = dependency('libkmod', method : 'pkg-config', required : true)
libcurl = dependency('libcurl', method : 'pkg-config', required : true)
zlib = dependency('zlib', method : 'pkg-config', required : true)
yaml = dependency('yaml-0.1', method : 'pkg-config', required : true)
openssl = dependency('openssl', method : 'pkg-config', required : true)
libcap = dependency('libcap', method : 'pkg-config', required : true)

# Test suite dependencies
run_tests = get_option('tests')
if run_tests
    cunit = dependency('cunit', method : 'pkg-config', required : false)
    pymod = import('python')
    python = pymod.find_installation('python3', required: false)
else
    cunit = disabler()
    python = disabler()
endif

# Other dependencies

# libmandoc (favor static one over shared one)
mandoc = cc.find_library('mandoc', required : false, static : true)

if mandoc.found()
    message('found static libmandoc')
else
    mandoc = cc.find_library('mandoc', required : false, static : false)

    if mandoc.found()
        message('found shared libmandoc')
    else
        error('*** unable to find libmandoc')
    endif
endif

if not cc.has_function('mparse_alloc', dependencies : [mandoc, zlib])
    error('*** unable to find mparse_alloc() in libmandoc')
endif

if cc.has_header('mandoc_parse.h')
    add_project_arguments('-DNEWLIBMANDOC', language : 'c')
else
    message('using libmandoc < 1.14.5 API')
endif

# libmagic
if not cc.has_function('magic_open', args : ['-lmagic'])
    error('*** unable to find magic_open() in libmagic')
endif

magic = declare_dependency(link_args : ['-lmagic'])

# dlopen
if not cc.has_function('dlopen', args : ['-ldl'], dependencies : [zlib])
    error('*** unable to find dlopen() in libdl')
endif

dl = declare_dependency(link_args : ['-ldl'])

# Header files for builds
inc = include_directories('include')

# Include all of the relevant subdirectories of the source tree
subdir('lib')
subdir('src')
subdir('include')
subdir('data')
subdir('test')
