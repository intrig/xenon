project('xenon', 'cpp',
    default_options : ['cpp_std=c++17', 'default_library=shared'])

inc = include_directories(['include'])
xenon_inc = include_directories(['src'])
subdir('xspx')
subdir('include')
subdir('src')
subdir('tools')
subdir('xddl')
subdir('unit')
subdir('examples')
subdir('perf')

install_subdir('xddl',
    exclude_directories : ['logs', 'IP/logs'],
    exclude_files : ['README.txt', '3GPP/36.331.asn', '3GPP/25.331.asn'],
    install_dir : get_option('datadir'))
