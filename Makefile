.PHONY: all clean realclean check test update get-deps

all: include/ict/ict.h build
	ninja -C build

build:
	meson --buildtype release $@

clean:
	@test -d build && ninja -C build clean || true

realclean:
	rm -rf build

check: build
	ninja -C build test

test: check

tags:
	@echo Making tags...
	@$(RM) tags; find . -name node_modules -type d -prune -o -name '*.c' -o -name '*.cc' \
    -o -name '*.cpp' \
	-o -name '*.h' -o -name '*.py' > flist && \
	ctags --file-tags=yes --language-force=C++ -L flist && rm flist
	@echo tags complete.

include/ict/ict.h:
	git submodule update --init --recursive

update:
	git submodule foreach git pull origin master

get-deps:
	sudo apt-get update
	sudo apt-get install -y libboost-all-dev meson
