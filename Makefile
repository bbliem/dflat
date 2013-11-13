# You probably need to adjust the *_dir variables before running "make"
cxxflags_release="-DWITH_THREADS=0"
cxxflags_debug=$(cxxflags_release)
cxxflags_gprof=$(cxxflags_debug)
cxxflags_static=$(cxxflags_release)
cxxflags_static32="-DWITH_THREADS=0 -m32"

gringo_dir=$(CURDIR)/../gringo-3.0.5-source
gringo_lib=$(gringo_dir)/build/release/lib/libgringo.a
gringo_lib_debug=$(gringo_dir)/build/debug/lib/libgringo.a
gringo_lib_gprof=$(gringo_dir)/build/gprof/lib/libgringo.a
gringo_lib_static=$(gringo_dir)/build/release/lib/libgringo.a
gringo_lib_static32=$(gringo_dir)/build/static32/lib/libgringo.a

clasp_dir=$(CURDIR)/../clasp-2.1.3
clasp_lib=$(clasp_dir)/build/release/libclasp/lib/libclasp.a
clasp_lib_debug=$(clasp_dir)/build/debug/libclasp/lib/libclasp.a
clasp_lib_gprof=$(clasp_dir)/build/gprof/libclasp/lib/libclasp.a
clasp_lib_static=$(clasp_dir)/build/release/libclasp/lib/libclasp.a
clasp_lib_static32=$(clasp_dir)/build/release_static_m32/libclasp/lib/libclasp.a

sharp_dir=$(CURDIR)/../sharp-1.1.1
sharp_lib=$(sharp_dir)/src/.libs/libsharp.a
sharp_lib_debug=$(sharp_dir)/src/.libs/libsharp.a
sharp_lib_gprof=$(sharp_dir)/src/.libs/libsharp.a
sharp_lib_static=$(sharp_dir)/src/.libs/libsharp.a
sharp_lib_static32=$(CURDIR)/../sharp-1.1.1-32bit/src/.libs/libsharp.a

ifeq ($(CXX),clang++)
	cmake_extra_options=\
		-DCMAKE_USER_MAKE_RULES_OVERRIDE=$(CURDIR)/clang-overrides \
		-D_CMAKE_TOOLCHAIN_PREFIX=llvm-
endif

all: release

release:
	mkdir -p build/release
	cd build/release && \
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

debug:
	mkdir -p build/debug
	cd build/debug && \
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

gprof:
	mkdir -p build/gprof
	cd build/gprof && \
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

static:
	mkdir -p build/static
	cd build/static && \
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

static32:
	mkdir -p build/static32
	cd build/static32 && \
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

clean:
	rm -rf build
