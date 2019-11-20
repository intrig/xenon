lua_inc = include_directories('lua-5.1.4/src')

xddl_cpp = custom_target(
    'xddl_cpp',
    output : 'xddl.cpp',
    input : 'xddl.xspx',
    command : [xspx_exe, '-S', '@OUTPUT@', '@INPUT@'])

xenon_lib = library('xenon', 'lua-5.1.4/src/lua_all.cpp', 'xml_parser_base.cpp', 
    'message.cpp', 'xenon.cpp', xddl_h, xddl_cpp,
    include_directories : [inc, lua_inc],
    install : true)
