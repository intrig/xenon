decodeperf_exe = executable('decodeperf', 'decode.cpp', xddl_h,
    include_directories : inc,
    link_args : '-lstdc++fs',
    link_with : xenon_lib)

loadperf_exe = executable('loadperf', 'loadperf.cpp', xddl_h,
    include_directories : inc,
    link_args : '-lstdc++fs',
    link_with : xenon_lib)

benchmark('loading xddl', decodeperf_exe,
    workdir : meson.source_root(),
    args : ['-i', '5', '--load', 'xddl'])

benchmark('decoding message', decodeperf_exe,
    workdir : meson.source_root(),
    args : ['-i', '10000', 'xddl'])

benchmark('flat xml', decodeperf_exe,
    workdir : meson.source_root(),
    args : ['--xml', 'xddl'])

benchmark('pretty xml', decodeperf_exe,
    workdir : meson.source_root(),
    args : ['--pretty', 'xddl'])

benchmark('load xddl', loadperf_exe,
    workdir : meson.source_root(),
    args : 'xddl')

