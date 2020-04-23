# Build rpminspect
rpminspect_prog_sources = [
    'rpminspect.c',
    'builds.c',
]
rpminspect_prog = executable(
    'rpminspect',
    rpminspect_prog_sources,
    include_directories : inc,
    install : true,
    link_with : [
        librpminspect,
    ],
    dependencies : [
        libcurl,
        yaml,
        rpm,
    ]
)

# Install man pages
install_man('rpminspect.1')

# Install the configuration file
install_data(
    sources : 'rpminspect.conf',
    install_dir : get_option('sysconfdir') + '/rpminspect'
)
