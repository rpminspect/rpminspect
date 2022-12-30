# Install data files
install_subdir(
    'abi',
    install_dir : get_option('datadir') + '/rpminspect/abi',
    strip_directory : true
)

install_subdir(
    'capabilities',
    install_dir : get_option('datadir') + '/rpminspect/capabilities',
    strip_directory : true
)

install_subdir(
    'licenses',
    install_dir : get_option('datadir') + '/rpminspect/licenses',
    strip_directory : true
)

install_subdir(
    'politics',
    install_dir : get_option('datadir') + '/rpminspect/politics',
    strip_directory : true
)

install_subdir(
    'fileinfo',
    install_dir : get_option('datadir') + '/rpminspect/fileinfo',
    strip_directory : true
)

install_subdir(
    'rebaseable',
    install_dir : get_option('datadir') + '/rpminspect/rebaseable',
    strip_directory : true
)

install_subdir(
    'icons',
    install_dir : get_option('datadir') + '/rpminspect/icons',
    strip_directory : true
)

# Install the configuration file
install_data(
    sources : [ 'generic.yaml', 'generic.json', 'generic.dson' ],
    install_dir : get_option('datadir') + '/rpminspect'
)
