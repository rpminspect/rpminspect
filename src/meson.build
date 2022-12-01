# Build rpminspect
deps = [ rpm ]

if libiberty.found()
    deps += [ libiberty ]
endif

rpminspect_prog = executable(
    'rpminspect',
    [ 'rpminspect.c' ],
    include_directories : inc,
    install : true,
    link_with : [
        librpminspect,
    ],
    dependencies : deps
)

# Install man pages
install_man('rpminspect.1')
