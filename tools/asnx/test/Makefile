test:
	rm -f temp.xddl $(OUT)
	asnx -ic test1.asn > temp.xddl && diff --strip-trailing-cr test1.xddl temp.xddl

sync:
	cp ../test.asn test1.asn
	asnx -c test1.asn > test1.xddl
