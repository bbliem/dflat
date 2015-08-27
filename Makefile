# You probably need to adjust the *_dir variables before running "make"
cxxflags_release="-DWITH_THREADS=1"
cxxflags_debug=$(cxxflags_release)
cxxflags_gprof=$(cxxflags_debug)
cxxflags_profiler=$(cxxflags_release)
cxxflags_release32="-DWITH_THREADS=1 -m32 -DNO_UNICODE"
cxxflags_static=$(cxxflags_release)
cxxflags_static32=$(cxxflags_release32)

gringo_dir=$(CURDIR)/../gringo-4.5.0-source
gringo_lib=$(gringo_dir)/build/release/libgringo.a
gringo_lib_debug=$(gringo_dir)/build/debug/libgringo.a
gringo_lib_gprof=$(gringo_dir)/build/release/libgringo.a
gringo_lib_profiler=$(gringo_dir)/build/release/libgringo.a
gringo_lib_release32=$(gringo_dir)/build/release32/libgringo.a
gringo_lib_static=$(gringo_dir)/build/static/libgringo.a
gringo_lib_static32=$(gringo_dir)/build/static32/libgringo.a

clasp_dir=$(CURDIR)/../clasp-3.1.3
# FIXME MT paths
clasp_lib=$(clasp_dir)/build/release_mt/libclasp/lib/libclasp.a
clasp_lib_debug=$(clasp_dir)/build/debug_mt/libclasp/lib/libclasp.a
clasp_lib_gprof=$(clasp_dir)/build/release/libclasp/lib/libclasp.a
clasp_lib_profiler=$(clasp_dir)/build/release_mt/libclasp/lib/libclasp.a
clasp_lib_release32=$(clasp_dir)/build/release_m32/libclasp/lib/libclasp.a
clasp_lib_static=$(clasp_dir)/build/static/libclasp/lib/libclasp.a
clasp_lib_static32=$(clasp_dir)/build/static32/libclasp/lib/libclasp.a

sharp_dir=$(CURDIR)/../sharp-1.1.1
sharp_lib=$(sharp_dir)/src/.libs/libsharp.a
sharp_lib_debug=$(sharp_dir)/src/.libs/libsharp.a
sharp_lib_gprof=$(sharp_dir)/src/.libs/libsharp.a
sharp_lib_profiler=$(sharp_dir)/src/.libs/libsharp.a
sharp_lib_release32=$(CURDIR)/../sharp-1.1.1-32bit/src/.libs/libsharp.a
sharp_lib_static=$(sharp_dir)/src/.libs/libsharp.a
sharp_lib_static32=$(CURDIR)/../sharp-1.1.1-32bit/src/.libs/libsharp.a

gtest_dir=/usr/src/gtest

# You may also want to use --output-sync if you use multiple processes, but
# this has the unfortunate side-effect of disabling color output.
GNUMAKEFLAGS=--no-print-directory
export CTEST_OUTPUT_ON_FAILURE=1

cmake_extra_options=-DCMAKE_MODULE_PATH=$(CURDIR)
ifeq ($(CXX),clang++)
	cmake_extra_options+=\
		-DCMAKE_USER_MAKE_RULES_OVERRIDE=$(CURDIR)/clang-overrides \
		-D_CMAKE_TOOLCHAIN_PREFIX=llvm-
endif

write_version_header = (grep -q "\"$(shell git describe | sed 's/^v\([0-9]\+\.[0-9]\+.[0-9]\+\)$$/\1/')\"" version.h 2> /dev/null || echo "\#define VERSION_NUMBER \"$(shell git describe | sed 's/^v\([0-9]\+\.[0-9]\+.[0-9]\+\)$$/\1/')\"" > version.h)

.PHONY: all
all: release

.PHONY: release
release:
	mkdir -p build/release
	cd build/release && \
	$(call write_version_header) && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DCMAKE_BUILD_TYPE=release \
		-DCMAKE_CXX_FLAGS:STRING=$(cxxflags_release) \
		-Dgringo_lib=$(gringo_lib) \
		-Dclasp_lib=$(clasp_lib) \
		-Dsharp_lib=$(sharp_lib) \
		-Dgringo_dir=$(gringo_dir) \
		-Dclasp_dir=$(clasp_dir) \
		-Dsharp_dir=$(sharp_dir) \
	&& $(MAKE)

.PHONY: debug
debug:
	mkdir -p build/debug
	cd build/debug && \
	$(call write_version_header) && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DCMAKE_BUILD_TYPE=debug \
		-DCMAKE_CXX_FLAGS:STRING=$(cxxflags_debug) \
		-Dgringo_lib=$(gringo_lib_debug) \
		-Dclasp_lib=$(clasp_lib_debug) \
		-Dsharp_lib=$(sharp_lib) \
		-Dgringo_dir=$(gringo_dir) \
		-Dclasp_dir=$(clasp_dir) \
		-Dsharp_dir=$(sharp_dir) \
	&& $(MAKE)

.PHONY: gprof
gprof:
	mkdir -p build/gprof
	cd build/gprof && \
	$(call write_version_header) && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DCMAKE_BUILD_TYPE=gprof \
		-DCMAKE_CXX_FLAGS:STRING=$(cxxflags_gprof) \
		-Dgringo_lib=$(gringo_lib_gprof) \
		-Dclasp_lib=$(clasp_lib_gprof) \
		-Dsharp_lib=$(sharp_lib) \
		-Dgringo_dir=$(gringo_dir) \
		-Dclasp_dir=$(clasp_dir) \
		-Dsharp_dir=$(sharp_dir) \
	&& $(MAKE)

.PHONY: profiler
profiler:
	mkdir -p build/profiler
	cd build/profiler && \
	$(call write_version_header) && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DCMAKE_BUILD_TYPE=profiler \
		-DCMAKE_CXX_FLAGS:STRING=$(cxxflags_profiler) \
		-Dgringo_lib=$(gringo_lib_profiler) \
		-Dclasp_lib=$(clasp_lib_profiler) \
		-Dsharp_lib=$(sharp_lib) \
		-Dgringo_dir=$(gringo_dir) \
		-Dclasp_dir=$(clasp_dir) \
		-Dsharp_dir=$(sharp_dir) \
	&& $(MAKE)

.PHONY: release32
release32:
	mkdir -p build/release32
	cd build/release32 && \
	$(call write_version_header) && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DCMAKE_BUILD_TYPE=release \
		-DCMAKE_CXX_FLAGS:STRING=$(cxxflags_release32) \
		-Dgringo_lib=$(gringo_lib_release32) \
		-Dclasp_lib=$(clasp_lib_release32) \
		-Dsharp_lib=$(sharp_lib_release32) \
		-Dgringo_dir=$(gringo_dir) \
		-Dclasp_dir=$(clasp_dir) \
		-Dsharp_dir=$(sharp_dir) \
	&& $(MAKE)

.PHONY: static
static:
	mkdir -p build/static
	cd build/static && \
	$(call write_version_header) && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DUSE_STATIC_LIBS=ON \
		-DCMAKE_BUILD_TYPE=release \
		-DCMAKE_CXX_FLAGS:STRING=$(cxxflags_static) \
		-Dgringo_lib=$(gringo_lib_static) \
		-Dclasp_lib=$(clasp_lib_static) \
		-Dsharp_lib=$(sharp_lib_static) \
		-Dgringo_dir=$(gringo_dir) \
		-Dclasp_dir=$(clasp_dir) \
		-Dsharp_dir=$(sharp_dir) \
	&& $(MAKE)

.PHONY: static32
static32:
	mkdir -p build/static32
	cd build/static32 && \
	$(call write_version_header) && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DUSE_STATIC_LIBS=ON \
		-DCMAKE_BUILD_TYPE=release \
		-DCMAKE_CXX_FLAGS:STRING=$(cxxflags_static32) \
		-Dgringo_lib=$(gringo_lib_static32) \
		-Dclasp_lib=$(clasp_lib_static32) \
		-Dsharp_lib=$(sharp_lib_static32) \
		-Dgringo_dir=$(gringo_dir) \
		-Dclasp_dir=$(clasp_dir) \
		-Dsharp_dir=$(sharp_dir) \
	&& $(MAKE)

.PHONY: dist
dist: release
	$(eval RELEASE := dflat-$(shell git describe | sed 's/^v\([0-9]\+\.[0-9]\+.[0-9]\+\)$$/\1/')-x86_64)
	$(eval DIST_DIR := build/dist/$(RELEASE))
	mkdir -p $(DIST_DIR)/lib
	cp build/release/dflat $(DIST_DIR)/dflat.bin
	strip $(DIST_DIR)/dflat.bin
	cp run-dflat.sh $(DIST_DIR)/dflat
	cp -L $(shell ldd build/release/dflat | awk '/=>/ { printf("%s ",$$3) } /ld-linux/ { printf("%s ",$$1) }') $(DIST_DIR)/lib
	cp -R applications $(DIST_DIR)
	cd build/dist && tar czf $(RELEASE).tar.gz $(RELEASE)
	mv build/dist/$(RELEASE).tar.gz build
	rm -rf build/dist

.PHONY: dist32
dist32: release32
	$(eval RELEASE := dflat-$(shell git describe | sed 's/^v\([0-9]\+\.[0-9]\+.[0-9]\+\)$$/\1/')-i386)
	$(eval DIST_DIR := build/dist32/$(RELEASE))
	mkdir -p $(DIST_DIR)/lib
	cp build/release32/dflat $(DIST_DIR)/dflat.bin
	strip $(DIST_DIR)/dflat.bin
	cp run-dflat.sh $(DIST_DIR)/dflat
	cp -L $(shell ldd build/release32/dflat | awk '/=>/ { printf("%s ",$$3) } /ld-linux/ { printf("%s ",$$1) }') $(DIST_DIR)/lib
	cp -R applications $(DIST_DIR)
	cd build/dist32 && tar czf $(RELEASE).tar.gz $(RELEASE)
	mv build/dist32/$(RELEASE).tar.gz build
	rm -rf build/dist32

.PHONY: test
test:
	@mkdir -p build/debug
	@cd build/debug && \
	$(call write_version_header) && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DCMAKE_BUILD_TYPE=debug \
		-DCMAKE_CXX_FLAGS:STRING=$(cxxflags_debug) \
		-Dbuild_tests=BOOL:ON \
		-Dgtest_dir=$(gtest_dir) \
		-Dgringo_lib=$(gringo_lib_debug) \
		-Dclasp_lib=$(clasp_lib_debug) \
		-Dsharp_lib=$(sharp_lib) \
		-Dgringo_dir=$(gringo_dir) \
		-Dclasp_dir=$(clasp_dir) \
		-Dsharp_dir=$(sharp_dir) \
	&& $(MAKE) && dflat-tests/tests
# For what it's worth, it should also be possible to run "make test" in
# build/release if you want to use ctest.

.PHONY: clean
clean:
	rm -rf build
