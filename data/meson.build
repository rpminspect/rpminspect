# Install data files
install_subdir(
    'abi-checking-whitelist',
    install_dir : get_option('datadir') + '/rpminspect/abi-checking-whitelist',
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
    'stat-whitelist',
    install_dir : get_option('datadir') + '/rpminspect/stat-whitelist',
    strip_directory : true
)

install_subdir(
    'version-whitelist',
    install_dir : get_option('datadir') + '/rpminspect/version-whitelist',
    strip_directory : true
)

# Install the configuration file
install_data(
    sources : 'rpminspect.conf',
    install_dir : get_option('sysconfdir') + '/rpminspect'
)
