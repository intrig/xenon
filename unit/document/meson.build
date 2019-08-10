spec_exe = executable('spec', 'specunit.cpp', 'fieldtypes.cpp', xddl_h,
    include_directories : inc,
    link_with : xenon_lib)

test('spec', spec_exe,
    workdir : meson.source_root() + '/unit/document')

boost_dep = dependency('boost', modules : ['system', 'filesystem'])
if boost_dep.found()
    load_exe = executable('load', 'loadunit.cpp', xddl_h,
        include_directories : inc,
        dependencies : boost_dep,
        link_with : xenon_lib)
    test('load', load_exe)
endif
