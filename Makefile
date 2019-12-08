.PHONY: all clean realclean check test update get-deps install uninstall
.PHONY: check-install tags

all: include/xenon/ict/ict.h build debug
	ninja -C build

build:
	mkdir $@ && cd $@ && cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..

debug:
	mkdir $@ && cd $@ && cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ..

xcode:
	mkdir $@ && cd $@ && cmake -GXcode -DCMAKE_BUILD_TYPE=Debug ..

clean:
	@test -d build && ninja -C build clean || true

realclean:
	rm -rf build

check: build
	CTEST_OUTPUT_ON_FAILURE=1 ninja -C build test

install: build
	ninja -C build install

uninstall:

check-install: build
ifeq ($(TRAVIS_OS_NAME),linux)
	sudo ldconfig
endif
	rm -rf instacheck/build
	cd instacheck && mkdir build && cd build && cmake -GNinja ..
	ninja -C instacheck/build
	ninja -C instacheck/build test

tags:
	@echo Making tags...
	@$(RM) tags; find . -name '*.cpp' \
	-o -name '*.h' > flist && \
	ctags --file-tags=yes -L flist --totals && rm flist
	@echo tags complete.

include/xenon/ict/ict.h:
	git submodule update --init --recursive

update:
	git submodule foreach git pull origin master

get-deps:
ifeq ($(TRAVIS_OS_NAME),linux)
	sudo apt-get update
	sudo apt-get install -y ninja-build
endif
ifeq ($(TRAVIS_OS_NAME),osx)
	sudo cp deps/osx/ninja /usr/local/bin
endif

xddl.adoc: all ex.adoc
	$(RM) xddl.adoc
	build/xspx/xspx --adoc src/xddl.xspx | build/tools/procadoc | \
    build/tools/procadoc > xddl.adoc
