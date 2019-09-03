.PHONY: docs

D=docs

$(D)/xddl.adoc: all $(D)/ex.adoc
	$(RM) $@
	build/xspx/xspx --adoc src/xddl.xspx | build/tools/procadoc | build/tools/procadoc > $@

$(D)/xddl.html: $(D)/xddl.adoc
	$(RM) $@
	asciidoctor $(D)/xddl.adoc

docs: $(D)/xddl.html
