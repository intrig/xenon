.PHONY: all clean realclean check test upgrade-ict install uninstall
.PHONY: check-install tags update-ict

all: include/xenon/ict/ict.h build debug
	ninja -C build

build:
	mkdir $@ && cd $@ && cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..

debug:
	mkdir $@ && cd $@ && cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ..

xcode:
	mkdir $@ && cd $@ && cmake -GXcode -DCMAKE_BUILD_TYPE=Debug ..

clang:
	mkdir $@ && cd $@ && cmake -GNinja -DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=/usr/local/clang_9.0.0/bin/clang \
	-DCMAKE_CXX_COMPILER=/usr/local/clang_9.0.0/bin/clang++ ..

clean:
	@test -d build && ninja -C build clean || true

realclean:
	rm -rf build

check: all
	CTEST_OUTPUT_ON_FAILURE=1 ninja -C build test

install: build
	ninja -C build install

uninstall:

tags:
	@echo Making tags...
	@$(RM) tags; find . -name '*.cpp' -o -name '*.c' \
	-o -name '*.h' > flist && \
	ctags --file-tags=yes -L flist --totals && rm flist
	@echo tags complete.

include/xenon/ict/ict.h:
	git submodule update --init --recursive

update-ict:
	git submodule update --init --recursive

upgrade-ict:
	git submodule foreach git pull origin master

check-install:
	cmake -E remove_directory instacheck/build
	cmake -B instacheck/build -S instacheck -GNinja -DCMAKE_BUILD_TYPE=Release
	cmake --build instacheck/build
	cd instacheck/build && ctest

xddl.adoc: all ex.adoc
	$(RM) xddl.adoc
	build/xspx/xspx --adoc src/xddl.xspx | build/tools/procadoc | \
    build/tools/procadoc > xddl.adoc
