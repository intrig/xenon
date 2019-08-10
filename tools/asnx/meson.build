#bison = find_program('bison', required : true)
#flex = dependency('flex'), required : true)
# bison_target(['MyParser', 'asn1.ypp', cmake_current_binary_dir, '/parser.cc'])
# flex_target(['MyScanner', 'asn1.l', cmake_current_binary_dir, '/lexer.cc'])
# add_flex_bison_dependency(['MyScanner', 'MyParser'])
# include_directories(cmake_current_source_dir)
#asnx_exe = executable('asnx', 'asnast.cpp', bison_myparser_outputs, flex_myscanner_outputs)
# target_link_libraries(['asnx', 'xenon'])

flex = find_program('flex', required: true)
bison = find_program('bison', required: true)

lgen = generator(flex,
output : '@PLAINNAME@.yy.cpp',
arguments : ['-o', '@OUTPUT@', '@INPUT@'])

lfiles = lgen.process('asn1.l')

pgen = generator(bison,
output : ['@BASENAME@.tab.cpp', '@BASENAME@.tab.hh'],
arguments : ['@INPUT@', '--defines=@OUTPUT1@', '--output=@OUTPUT0@'])

pfiles = pgen.process('asn1.ypp')

asnx_exe = executable('asnx', 'asnast.cpp', lfiles, pfiles,
    include_directories : [inc, xenon_inc],
    link_with : xenon_lib)

#asnx_gen = generator(asnx_exe,
#output : '@PLAINNAME
#arguments : [ '-t', '@INPUT@',
#test('parsertest', e)
