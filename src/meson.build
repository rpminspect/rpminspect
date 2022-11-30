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
        rpm,
    ]
)

# Install man pages
install_man('rpminspect.1')
