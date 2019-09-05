In order to build the xddl.adoc file, and also generate a local xddl.html file, run
the following command from this directory:

    ninja -C ../build  && ../build/xspx/xspx --adoc ../src/xddl.xspx | ../build/tools/procadoc -s ex.adoc > xddl.adoc && asciidoctor xddl.adoc
