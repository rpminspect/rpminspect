project('rpminspect',
        'c',
        version : '0.6',
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
libcap_ng = dependency('libcap-ng', method : 'pkg-config', required : true)

# Test suite dependencies
cunit = dependency('cunit', method : 'pkg-config', required : false)
pymod = import('python')
python = pymod.find_installation('python3', required: false)

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
    input : 'lib/pic_bits.sh',
    command : ['@INPUT@', '@OUTPUT@'],
)

inspect_elf_bits_dep = declare_dependency(sources : [inspect_elf_bits_c])

# Build librpminspect
librpminspect_sources = [
    'lib/badwords.c',
    'lib/checksums.c',
    'lib/copyfile.c',
    'lib/files.c',
    'lib/free.c',
    'lib/init.c',
    'lib/inspect.c',
    'lib/inspect_addedfiles.c',
    'lib/inspect_changedfiles.c',
    'lib/inspect_desktop.c',
    'lib/inspect_disttag.c',
    'lib/inspect_elf.c',
    'lib/inspect_emptyrpm.c',
    'lib/inspect_javabytecode.c',
    'lib/inspect_kernel.c',
    'lib/inspect_license.c',
    'lib/inspect_manpage.c',
    'lib/inspect_metadata.c',
    'lib/inspect_modularity.c',
    'lib/inspect_ownership.c',
    'lib/inspect_removedfiles.c',
    'lib/inspect_shellsyntax.c',
    'lib/inspect_specname.c',
    'lib/inspect_upstream.c',
    'lib/inspect_xml.c',
    'lib/koji.c',
    'lib/listfuncs.c',
    'lib/local.c',
    'lib/magic.c',
    'lib/mkdirp.c',
    'lib/output.c',
    'lib/output_json.c',
    'lib/output_text.c',
    'lib/peers.c',
    'lib/readelf.c',
    'lib/results.c',
    'lib/rmtree.c',
    'lib/rpm.c',
    'lib/runcmd.c',
    'lib/strfuncs.c',
    'lib/tty.c',
    'lib/unpack.c',
]
librpminspect = library(
    'rpminspect',
    librpminspect_sources,
    version : '0.5.0',
    install : true,
    include_directories : include_directories('lib'),
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
        libcap_ng,
        mandoc,
        iniparser,
        magic,
        dl,
    ]
)

# Build rpminspect
rpminspect_prog_sources = [
    'src/rpminspect.c',
    'src/builds.c',
]
rpminspect_prog = executable(
    'rpminspect',
    rpminspect_prog_sources,
    install : true,
    include_directories : [
        include_directories('lib'),
        include_directories('src'),
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
    incdirs = [include_directories('lib'),
               include_directories('src'),
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
if python.found()
    test_env = environment()
    test_env.set('RPMINSPECT', rpminspect_prog.full_path())
    test_env.set('RPMINSPECTCONF', meson.source_root() + '/src/rpminspect.conf')

    test('rpminspect-test-suite',
         python,
         args : ['-B', '-m', 'unittest', 'discover', '-v', meson.source_root() + '/tests'],
         env : test_env
        )
else
    warning('Python not found, skipping integration test suite')
endif

# Install header files
install_headers(
    'lib/constants.h',
    'lib/inspect.h',
    'lib/output.h',
    'lib/readelf.h',
    'lib/results.h',
    'lib/rpminspect.h',
    'lib/types.h',
    subdir : 'librpminspect'
)

# Install man pages
install_man('src/rpminspect.1')

# Install data files
install_subdir(
    'data',
    install_dir : get_option('datadir') / 'rpminspect',
    strip_directory : true
)

# Install the configuration file
install_data(
    sources : 'src/rpminspect.conf',
    install_dir : get_option('sysconfdir') / 'rpminspect'
)
