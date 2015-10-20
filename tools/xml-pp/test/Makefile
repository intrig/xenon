test:
	rm -f temp.xddl
	xml-pp test.xddl > temp.xddl
	diff --strip-trailing-cr formatted.xddl temp.xddl
	cp test.xddl temp.xddl
	xml-pp -r temp.xddl
	diff --strip-trailing-cr formatted.xddl temp.xddl

sync:
	cp ../test.xddl .
	xml-pp test.xddl > formatted.xddl
