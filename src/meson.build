# include_directories('lua-5.1.4/src')
#cmake_archive_output_directory = [cmake_source_dir, '/o']
# add_custom_command(['OUTPUT', cmake_source_dir, '/include/xenon/xddl.h', cmake_source_dir, '/src/xddl.cpp', 'COMMAND', 'xspx', '-H', 't', '-S', 'xddl.cpp', cmake_current_source_dir, '/xddl.xspx', 'COMMAND', cmake_command, '-E', 'copy_if_different', 't', cmake_source_dir, '/include/xenon/xddl.h', 'COMMAND', cmake_command, '-E', 'copy_if_different', 'xddl.cpp', cmake_source_dir, '/src/xddl.cpp', 'DEPENDS', 'xspx', 'MAIN_DEPENDENCY', cmake_current_source_dir, '/xddl.xspx', 'VERBATIM'])

lua_inc = include_directories('lua-5.1.4/src')

xddl_cpp = custom_target(
    'xddl_cpp',
    output : 'xddl.cpp',
    input : 'xddl.xspx',
    command : [xspx_exe, '-H', 'foo.h', '-S', '@OUTPUT@', '@INPUT@'])

xenon_lib = library('xenon', 'lua-5.1.4/src/lua_all.cpp', 'xml_parser_base.cpp', 
    'message.cpp', 'xenon.cpp', xddl_h, xddl_cpp,
    include_directories : [inc, lua_inc])
