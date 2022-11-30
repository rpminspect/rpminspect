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

# Make sure the test suite can find this built rpminspect
test_env.set('RPMINSPECT', rpminspect_prog.full_path())

# Install man pages
install_man('rpminspect.1')
