# Tools with just one source file are in this directory.  Otherwise they are in a subdirectory.
subdir('asnx')
asn_strip_exe = executable('asn-strip', 'asnstrip.cpp',
    include_directories : inc,
    link_with : xenon_lib)

idm_exe = executable('idm', 'idm.cpp', xddl_h,
    include_directories : inc,
    link_with : xenon_lib)

xddl2wd_exe = executable('xddl2wd', 'xddl2wd.cpp', xddl_h,
    include_directories : [inc, xenon_inc],
    link_with : xenon_lib)

xml_patch_exe = executable('xml-patch', 'xmlpatch.cpp',
    include_directories : inc,
    link_with : xenon_lib)

xml_pp_exe = executable('xml-pp', 'xmlpp.cpp',
    include_directories : inc,
    link_with : xenon_lib)

xv_exe = executable('xv', 'xv.cpp', xddl_h,
    include_directories : [inc, xenon_inc],
    link_args : '-lstdc++fs',
    link_with : xenon_lib)

recombobulator_exe = executable('recombobulator', 'recombobulator.cpp', xddl_h,
    include_directories : [inc, xenon_inc],
    link_with : xenon_lib)
