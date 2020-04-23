# Build rpminspect
rpminspect_prog = executable(
    'rpminspect',
    [ 'rpminspect.c' ],
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
