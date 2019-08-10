if boost_dep.found()
    decodeperf_exe = executable('decodeperf', 'decode.cpp', xddl_h,
        include_directories : inc,
        dependencies : boost_dep,
        link_with : xenon_lib)
endif
