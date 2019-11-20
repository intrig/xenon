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
    cpp_args : '-DUSING_MESON',
    link_with : xenon_lib)
