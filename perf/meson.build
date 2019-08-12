if boost_dep.found()
    decodeperf_exe = executable('decodeperf', 'decode.cpp', xddl_h,
        include_directories : inc,
        dependencies : boost_dep,
        link_with : xenon_lib)

    benchmark('loading xddl', decodeperf_exe,
        workdir : meson.source_root(),
        args : ['-i', '5', '--load', 'xddl']
        )

    benchmark('decoding message', decodeperf_exe,
        workdir : meson.source_root(),
        args : ['-i', '10000', 'xddl']
        )

    benchmark('flat xml', decodeperf_exe,
        workdir : meson.source_root(),
        args : ['--xml', 'xddl']
        )

    benchmark('pretty xml', decodeperf_exe,
        workdir : meson.source_root(),
        args : ['--pretty', 'xddl']
        )
endif
