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
        libkmod,
    ]
)

# Install man pages
install_man('rpminspect.1')
