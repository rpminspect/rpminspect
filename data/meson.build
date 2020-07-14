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
    'political-whitelist',
    install_dir : get_option('datadir') + '/rpminspect/political-whitelist',
    strip_directory : true
)

install_subdir(
    'fileinfo',
    install_dir : get_option('datadir') + '/rpminspect/fileinfo',
    strip_directory : true
)

install_subdir(
    'version-whitelist',
    install_dir : get_option('datadir') + '/rpminspect/version-whitelist',
    strip_directory : true
)

# Install the configuration file
install_data(
    sources : 'generic.yaml',
    install_dir : get_option('datadir') + '/rpminspect'
)
