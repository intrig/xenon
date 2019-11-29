xddl_h = custom_target(
    'xddl_h',
    output : 'xddl.h',
    input : '../../src/xddl.xspx',
    command : [xspx_exe, '-H', '@OUTPUT@', '@INPUT@'],
    install : true,
    install_dir : get_option('includedir') + '/xenon')

copy = find_program('cp')

config_h = custom_target(
    'config_h',
    input : meson.source_root() + '/config.meson.in',
    output : 'config.h',
    command : [ copy, '@INPUT@', '@OUTPUT@' ],
    install : true,
    install_dir : get_option('includedir') + '/xenon')

install_headers(
    'DateTime.h',
    'att_pair.h',
    'cpp_code.h',
    'functions.h',
    'message.h',
    'node.h',
    'recref.h',
    'spec_server.h',
    'xddl_code.h',
    'xddl_head.h',
    'xddl_tail.h',
    'xenon.h',
    'ximsi.h',
    'xml_parser.h',
    'xml_parser_base.h',
    install_dir : get_option('includedir') + '/xenon')

install_headers(
    'ict/bitstring.h',
    'ict/command.h',
    'ict/exception.h',
    'ict/expr.h',
    'ict/ict.h',
    'ict/multivector.h',
    'ict/netvar.h',
    'ict/osstream.h',
    'ict/string64.h',
    install_dir : get_option('includedir') + '/xenon/ict')
