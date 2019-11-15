# asn.1 to xddl is no longer automated, keeping this around for reference

# cp_exe =find_program('rsync')

#temp_36_331 = custom_target(
#    'temp_36_331',
#    output : '36.331-temp.xddl',
#    input : '36.331.asn',
#    command : [asnx_exe, '-o', '@OUTPUT@', '-t', '@INPUT@'])

#x36_331 = custom_target(
#    'x36_331',
#    output : 'TS-36.331.xddl',
#    input : [temp_36_331, '36.331-sub.txt'],
#    command : [xml_patch_exe, '-o', '@OUTPUT@', '@INPUT0@', '@INPUT1@'])

#temp_25_331 = custom_target(
#    'temp_25_331',
#    output : '25.331-temp.xddl',
#    input : '25.331.asn',
#    command : [asnx_exe, '-o', '@OUTPUT@', '-t', '@INPUT@'])

#x25_331 = custom_target(
#    'x25_331',
#    output : 'TS-25.331.xddl',
#    input : [temp_25_331, '25.331-sub.txt'],
#    command : [xml_patch_exe, '-o', '@OUTPUT@', '@INPUT0@', '@INPUT1@'])

#   copy_xddl = custom_target(
#       'cp_xddl',
#       output : 'nothing',
#       depends : [x36_331, x25_331],
#       command : [cp_exe,
#           [meson.current_build_dir() + '/TS-36.331.xddl',
#           meson.current_build_dir() + '/TS-25.331.xddl'],
#           meson.current_source_dir()])
