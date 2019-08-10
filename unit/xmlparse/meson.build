xmlparse_exe = executable('xmlparse', 'xmlparseunit.cpp',
    include_directories : inc,
    link_with : xenon_lib)
test('xmlparse', xmlparse_exe,
    workdir : meson.source_root() + '/unit/xmlparse')
