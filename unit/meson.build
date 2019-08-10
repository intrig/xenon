recref_exe = executable('recref', 'recref_unit.cpp',
    include_directories : inc,
    link_with : xenon_lib)
test('recref', recref_exe)

msg_exe = executable('msg', 'message_unit.cpp', xddl_h,
    include_directories : inc,
    link_with : xenon_lib)
test('msg', msg_exe,
    workdir : meson.source_root() + '/unit')

subdir('itu')
subdir('xmlparse')
subdir('document')
subdir('script')
subdir('xddlunit')
