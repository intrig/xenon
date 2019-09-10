project('instacheck', 'cpp',
    default_options : ['cpp_std=c++17', 'default_library=shared'])

decode_exe = executable('decode', 'decode.cpp',
    link_args : '-lxenon')

xddldir = get_option('prefix') + '/' + get_option('datadir') + '/xddl'

test('decode', decode_exe, workdir : xddldir)
