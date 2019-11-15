# Tools with just one source file are in this directory.  Otherwise they are in
# a subdirectory.
subdir('asnx')

idm_exe = executable('idm', 'idm.cpp', xddl_h,
    include_directories : [inc, xenon_inc],
    install : true,
    link_with : xenon_lib)

xml_patch_exe = executable('xml-patch', 'xmlpatch.cpp',
    include_directories : inc,
    link_with : xenon_lib)

xml_pp_exe = executable('xml-pp', 'xmlpp.cpp',
    include_directories : inc,
    link_with : xenon_lib)

xv_exe = executable('xv', 'xv.cpp', xddl_h,
    include_directories : [inc, xenon_inc],
    install : true,
    link_with : xenon_lib)

recombobulator_exe = executable('recombobulator', 'recombobulator.cpp', xddl_h,
    include_directories : [inc, xenon_inc],
    link_with : xenon_lib)

# We're using experimental/filesytems and it doesn't work on Travis currently.
# TODO: eventually remove this dependency
if build_machine.system() == 'linux'
    fs_link = '-lstdc++fs'

    asn_strip_exe = executable('asn-strip', 'asnstrip.cpp',
        include_directories : inc,
        link_args : fs_link,
        link_with : xenon_lib)

    procadoc_exe = executable('procadoc', 'procadoc.cpp', xddl_h,
        include_directories : inc,
        link_args : fs_link)
endif
