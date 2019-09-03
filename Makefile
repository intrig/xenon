.PHONY: all clean realclean check test update get-deps install uninstall check-install
.PHONY: tags

all: include/xenon/ict/ict.h build
	ninja -C build

build:
	meson --buildtype release $@

clean:
	@test -d build && ninja -C build clean || true
	rm -f xddl/3GPP/TS-25.331.xddl
	rm -f xddl/3GPP/TS-36.331.xddl

realclean:
	rm -rf build

check: build
	ninja -C build test

install: build
	ninja -C build install

uninstall:
	ninja -C build uninstall

check-install: build
	XDDLPATH=/usr/local/share/xddl xv unit/xddlunit/icd_gold.xv
	c++ -pipe -D_FILE_OFFSET_BITS=64 -Wall -Winvalid-pch -Wnon-virtual-dtor -std=c++17 -O3 -o 'build/decode.cpp.o' -c examples/decode.cpp
	c++ -o build/decode 'build/decode.cpp.o' -lxenon
	XDDLPATH=/usr/local/share/xddl build/decode

tags:
	@echo Making tags...
	@$(RM) tags; find . -name node_modules -type d -prune -o -name '*.c' -o -name '*.cc' \
    -o -name '*.cpp' \
	-o -name '*.h' -o -name '*.py' > flist && \
	ctags --file-tags=yes --language-force=C++ -L flist && rm flist
	@echo tags complete.

include/xenon/ict/ict.h:
	git submodule update --init --recursive

update:
	git submodule foreach git pull origin master

get-deps:
	sudo apt-get update
	sudo apt-get install -y meson

xddl.adoc: all ex.adoc
	$(RM) xddl.adoc
	build/xspx/xspx --adoc src/xddl.xspx | build/tools/procadoc | build/tools/procadoc > xddl.adoc

include docs/Makefile
