project('rpminspect',
        'c',
        version : '0.5',
        default_options : [
            'c_std=c99',
            'warning_level=2',
            'buildtype=debugoptimized'
        ],
        license : 'GPLv3')

cc = meson.get_compiler('c')

# Pull in all the usual glibc defines
add_global_arguments('-D_GNU_SOURCE', language : 'c')
add_global_arguments('-DPACKAGE_VERSION="@@0@@"'.format(meson.project_version()), language : 'c')

# Dependencies that use pkg-config
jsonc = dependency('json-c', method : 'pkg-config', required : true)
xmlrpc = dependency('xmlrpc xmlrpc_util xmlrpc_client', method : 'pkg-config', required : true)
libxml = dependency('libxml-2.0', method : 'pkg-config', required : true)
rpm = dependency('rpm', method : 'pkg-config', required : true)
libarchive = dependency('libarchive', method : 'pkg-config', required : true)
libelf = dependency('libelf', method : 'pkg-config', required : true)
libkmod = dependency('libkmod', method : 'pkg-config', required : true)
libcurl = dependency('libcurl', method : 'pkg-config', required : true)
zlib = dependency('zlib', method : 'pkg-config', required : true)
yaml = dependency('yaml-0.1', method : 'pkg-config', required : true)
openssl = dependency('openssl', method : 'pkg-config', required : true)
cunit = dependency('cunit', method : 'pkg-config', required : false)

# Other dependencies

# libmandoc
if not cc.has_function('mparse_alloc', args : ['-lmandoc'], dependencies : [zlib])
    error('*** unable to find mparse_alloc() in libmandoc')
endif

if cc.has_header('mandoc_parse.h')
    error('*** mandoc >= 1.14.5 found, must use <= 1.14.4')
endif

mandoc = declare_dependency(link_args : ['-lmandoc'])

# libiniparser
if not cc.has_function('iniparser_load', args : ['-liniparser'], dependencies : [zlib])
    error('*** unable to find iniparser_load() in libiniparser')
endif

iniparser = declare_dependency(link_args : ['-liniparser'])

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

# Generate inspect_elf_bits.c
inspect_elf_bits_c = custom_target(
    'inspect_elf_bits.c',
    output : 'inspect_elf_bits.c',
    input : 'src/librpminspect/pic_bits.sh',
    command : ['@INPUT@', '@OUTPUT@'],
)

inspect_elf_bits_dep = declare_dependency(sources : [inspect_elf_bits_c])

# Build librpminspect
librpminspect_sources = [
    'src/librpminspect/badwords.c',
    'src/librpminspect/checksums.c',
    'src/librpminspect/copyfile.c',
    'src/librpminspect/files.c',
    'src/librpminspect/free.c',
    'src/librpminspect/init.c',
    'src/librpminspect/inspect.c',
    'src/librpminspect/inspect_addedfiles.c',
    'src/librpminspect/inspect_changedfiles.c',
    'src/librpminspect/inspect_desktop.c',
    'src/librpminspect/inspect_disttag.c',
    'src/librpminspect/inspect_elf.c',
    'src/librpminspect/inspect_emptyrpm.c',
    'src/librpminspect/inspect_javabytecode.c',
    'src/librpminspect/inspect_kernel.c',
    'src/librpminspect/inspect_license.c',
    'src/librpminspect/inspect_manpage.c',
    'src/librpminspect/inspect_metadata.c',
    'src/librpminspect/inspect_modularity.c',
    'src/librpminspect/inspect_removedfiles.c',
    'src/librpminspect/inspect_specname.c',
    'src/librpminspect/inspect_xml.c',
    'src/librpminspect/koji.c',
    'src/librpminspect/listfuncs.c',
    'src/librpminspect/local.c',
    'src/librpminspect/magic.c',
    'src/librpminspect/mkdirp.c',
    'src/librpminspect/output.c',
    'src/librpminspect/output_json.c',
    'src/librpminspect/output_text.c',
    'src/librpminspect/peers.c',
    'src/librpminspect/readelf.c',
    'src/librpminspect/results.c',
    'src/librpminspect/rmtree.c',
    'src/librpminspect/rpm.c',
    'src/librpminspect/runcmd.c',
    'src/librpminspect/strfuncs.c',
    'src/librpminspect/tty.c',
    'src/librpminspect/unpack.c',
]
librpminspect = library(
    'rpminspect',
    librpminspect_sources,
    version : '0.5.0',
    install : true,
    include_directories : include_directories('src/librpminspect'),
    dependencies : [
        inspect_elf_bits_dep,
        jsonc,
        xmlrpc,
        libxml,
        rpm,
        libarchive,
        libelf,
        libkmod,
        libcurl,
        zlib,
        yaml,
        openssl,
        mandoc,
        iniparser,
        magic,
        dl,
    ]
)

# Build rpminspect
rpminspect_prog_sources = [
    'src/rpminspect/rpminspect.c',
    'src/rpminspect/builds.c',
]
rpminspect_prog = executable(
    'rpminspect',
    rpminspect_prog_sources,
    install : true,
    include_directories : [
        include_directories('src/librpminspect'),
        include_directories('src/rpminspect'),
    ],
    link_with : [
        librpminspect,
    ],
    dependencies : [
        libcurl,
        yaml,
        rpm,
    ]
)

# Test setups
add_test_setup(
    'valgrind',
    exe_wrapper : ['valgrind', '--verbose', '--error-exitcode=1', '--leak-check=full', '--track-origins=yes'])

# Unit test suite (disabled if we do not have CUnit installed)
if cunit.found()
    incdirs = [include_directories('src/librpminspect'),
               include_directories('src/rpminspect'),
               include_directories('tests/lib')]

    test_badwords = executable(
        'test-badwords',
        ['tests/lib/test-badwords.c',
         'tests/lib/test-main.c'],
        include_directories : incdirs,
        dependencies : [ cunit ],
        c_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_koji = executable(
        'test-koji',
        ['tests/lib/test-koji.c',
         'tests/lib/test-main.c'],
        include_directories : incdirs,
        dependencies : [ cunit ],
        c_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_tty = executable(
        'test-tty',
        ['tests/lib/test-tty.c',
         'tests/lib/test-main.c'],
        include_directories : incdirs,
        dependencies : [ cunit ],
        c_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_strfuncs = executable(
        'test-strfuncs',
        ['tests/lib/test-strfuncs.c',
         'tests/lib/test-main.c'],
        include_directories : incdirs,
        dependencies : [ cunit ],
        c_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_init = executable(
        'test-init',
        ['tests/lib/test-init.c',
         'tests/lib/test-main.c'],
        include_directories : incdirs,
        dependencies : [ cunit ],
        c_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_inspect_elf = executable(
        'test-inspect_elf',
        ['tests/lib/test-inspect_elf.c',
         'tests/lib/test-main.c'],
        include_directories : incdirs,
        dependencies : [
            cunit,
            libelf,
        ],
        c_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    # Support program used by test-inspect-elf
    execstack_prog = executable(
        'execstack',
        ['tests/lib/elftest.c'],
        link_args : ['-Wl,-z,execstack', '-Wl,-z,relro']
    )

    noexecstack_prog = executable(
        'noexecstack',
        ['tests/lib/elftest.c'],
        link_args : ['-Wl,-z,noexecstack', '-Wl,-z,relro']
    )

    # Unit tests
    test('test-badwords', test_badwords)
    test('test-koji', test_koji)
    test('test-tty', test_tty)
    test('test-strfuncs', test_strfuncs)
    test('test-init', test_init)
    test('test-inspect_elf',
         test_inspect_elf,
         depends : [execstack_prog, noexecstack_prog]
    )
else
    warning('CUnit not found, skipping unit test suite')
endif

# Integration test suite
test_env = environment()
test_env.set('RPMINSPECT', rpminspect_prog.full_path())

help_test = find_program('tests/rpminspect-help.sh')
segv_test = find_program('tests/rpminspect-segv.sh')

test('rpminspect-help', help_test, env : test_env)
test('rpminspect-segv', segv_test, env : test_env)

# Install header files
install_headers(
    'src/librpminspect/constants.h',
    'src/librpminspect/inspect.h',
    'src/librpminspect/output.h',
    'src/librpminspect/readelf.h',
    'src/librpminspect/results.h',
    'src/librpminspect/rpminspect.h',
    'src/librpminspect/types.h',
    subdir : 'librpminspect'
)

# Install man pages
install_man('src/rpminspect/rpminspect.1')

# Install data files
install_subdir(
    'data',
    install_dir : get_option('datadir') / 'rpminspect',
    strip_directory : true
)

# Install the configuration file
install_data(
    sources : 'src/rpminspect/rpminspect.conf',
    install_dir : get_option('sysconfdir') / 'rpminspect'
)
