project('rpminspect',
        'c',
        version : '1.8',
        default_options : [
            'c_std=c99',
            'warning_level=2',
            'werror=true',
            'buildtype=debugoptimized'
        ],
        license : 'GPL-3.0-or-later AND LGPL-3.0-or-later AND Apache-2.0 AND MIT')

cc = meson.get_compiler('c')

# Compiler options
add_global_arguments('-D_GNU_SOURCE', language : 'c')
add_global_arguments('-DPACKAGE_VERSION="@0@"'.format(meson.project_version()), language : 'c')

# Translations
if find_program('xgettext', required : get_option('nls')).found()
    add_global_arguments('-DGETTEXT_DOMAIN="' + meson.project_name() + '"', language : 'c')
    subdir('po')
endif

# Dependencies that use pkg-config
jsonc = dependency('json-c', required : true)
libxml = dependency('libxml-2.0', required : true)
libkmod = dependency('libkmod', required : true)
libcurl = dependency('libcurl', required : true)
zlib = dependency('zlib', required : true)
yaml = dependency('yaml-0.1', required : true)
clamav = dependency('libclamav', required : true)
icu_uc = dependency('icu-uc', required : true)
icu_io = dependency('icu-io', required : true)

# If we're on an older API, note that for the build
rpm = dependency('rpm', required : true)
if rpm.version().version_compare('<4.15.0')
    add_project_arguments('-D_HAVE_OLD_RPM_API', language : 'c')
endif

# librpmbuild
if not cc.has_function('rpmSpecParse', args : ['-lrpmbuild'])
    error('*** unable to find rpmSpecParse() in librpmbuild')
else
    rpmbuild = declare_dependency(link_args : ['-lrpmbuild'])
endif

# libarchive (need to check for specific functions)
libarchive = dependency('libarchive', required : true)

if cc.has_function('archive_version_details', dependencies : [ libarchive ])
    add_project_arguments('-D_HAVE_ARCHIVE_VERSION_DETAILS', language : 'c')
elif cc.has_function('archive_version_string', dependencies : [ libarchive ])
    add_project_arguments('-D_HAVE_ARCHIVE_VERSION_STRING', language : 'c')
endif

# openssl (need to check for the version info function)
openssl = dependency('openssl', required : true)

if not cc.has_function('OpenSSL_version', dependencies : [ openssl ]) and cc.has_function('SSLeay_version', dependencies : [ openssl ])
    add_project_arguments('-DOpenSSL_version=SSLeay_version', language : 'c')
elif not cc.has_function('OpenSSL_version', dependencies : [ openssl ]) and not cc.has_function('SSLeay_version', dependencies : [ openssl ])
    add_project_arguments('-D_NO_OPENSSL_VERSION_FUNCTION', language : 'c')
endif

# Test suite dependencies
run_tests = get_option('tests')
if run_tests
    cunit = dependency('cunit', method : 'pkg-config', required : true)
    python = find_program('python3', required : false)
else
    cunit = disabler()
    python = disabler()
endif

# Other dependencies

# libelf
libelf = dependency('libelf', method : 'pkg-config', required : false)

if not libelf.found() and cc.has_function('elf_begin', args : ['-lelf'])
    libelf = declare_dependency(link_args : ['-lelf'])
endif

if not libelf.found()
    error('*** unable to find libelf')
endif

# xmlrpc-c
xmlrpc_cmd = find_program('xmlrpc-c-config', required : true)
xmlrpc_libs_cmd = run_command(xmlrpc_cmd, '--libs')
xmlrpc_cflags_cmd = run_command(xmlrpc_cmd, '--cflags')

if xmlrpc_libs_cmd.returncode() == 0 and xmlrpc_cflags_cmd.returncode() == 0
    xmlrpc = declare_dependency(compile_args: xmlrpc_cflags_cmd.stdout().strip().split(),
                                link_args: xmlrpc_libs_cmd.stdout().strip().split())
    message('Declared dependency xmlrpc')
endif

if not xmlrpc.found()
    error('*** unable to find libxmlrpc')
endif

xmlrpc_cmd = find_program('xmlrpc-c-config', required : true)
xmlrpc_client_libs_cmd = run_command(xmlrpc_cmd, 'client', '--libs')
xmlrpc_client_cflags_cmd = run_command(xmlrpc_cmd, 'client', '--cflags')

if xmlrpc_client_libs_cmd.returncode() == 0 and xmlrpc_client_cflags_cmd.returncode() == 0
    xmlrpc_client = declare_dependency(compile_args: xmlrpc_client_cflags_cmd.stdout().strip().split(),
                                       link_args: xmlrpc_client_libs_cmd.stdout().strip().split())
    message('Declared dependency xmlrpc_client')
endif

if not xmlrpc_client.found()
    error('*** unable to find libxmlrpc_client')
endif

# libmandoc (favor static one over shared one)
mandoc = cc.find_library('mandoc',
                         dirs : [ '/usr/lib', '/usr/lib64', '/usr/local/lib' ],
                         required : false,
                         static : true)

if mandoc.found()
    message('found static libmandoc')
else
    mandoc = cc.find_library('mandoc',
                             dirs : [ '/usr/lib', '/usr/lib64', '/usr/local/lib' ],
                             required : false,
                             static : false)

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
    if cc.has_header('mandoc/mandoc_parse.h')
        add_project_arguments('-DNEWLIBMANDOC', language : 'c')
        add_project_arguments('-DMANDOC_INCLUDE_SUBDIR', language : 'c')
    else
        message('using libmandoc < 1.14.5 API')
    endif
endif

# libmagic
if not cc.has_function('magic_open', args : ['-lmagic'])
    error('*** unable to find magic_open() in libmagic')
endif

if cc.has_function('magic_version', args : ['-lmagic'])
    add_project_arguments('-D_HAVE_MAGIC_VERSION', language : 'c')
endif

magic = declare_dependency(link_args : ['-lmagic'])

# libcap
libcap = dependency('libcap', method : 'pkg-config', required : false)

if not libcap.found() and cc.has_function('cap_to_text', args : ['-lcap'])
    libcap = declare_dependency(link_args : ['-lcap'])
endif

if not libcap.found()
    error('*** unable to find libcap')
endif

# dlopen
if not cc.has_function('dlopen', args : ['-ldl'], dependencies : [zlib])
    error('*** unable to find dlopen() in libdl')
endif

dl = declare_dependency(link_args : ['-ldl'])

# Check for sys/queue.h
if not cc.has_header('sys/queue.h')
    message('<sys/queue.h> not found, using bundled copy')
    add_project_arguments('-D_COMPAT_QUEUE', language : 'c')
endif

# Header files for builds
inc = include_directories('include')

# Include all of the relevant subdirectories of the source tree
subdir('lib')
subdir('src')
subdir('include')
subdir('data')
subdir('test')
