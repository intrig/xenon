xddl_h = custom_target(
    'xddl_h',
    output : 'xddl.h',
    input : '../../src/xddl.xspx',
    command : [xspx_exe, '-H', '@OUTPUT@', '-S', 'foo.cpp', '@INPUT@'])

