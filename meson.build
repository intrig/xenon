project('xenon', 'cpp',
    default_options : ['cpp_std=c++17', 'default_library=shared'])

inc = include_directories(['include'])
xenon_inc = include_directories(['src'])
boost_dep = dependency('boost', modules : ['system', 'filesystem'])

if not boost_dep.found()
    error('no boost')
endif

subdir('xspx')
subdir('include')
subdir('src')
subdir('tools')
subdir('xddl')
subdir('unit')
subdir('examples')
subdir('perf')
