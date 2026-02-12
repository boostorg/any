# Copyright Claus Klein, 2025-2026
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

# Standard stuff

.SUFFIXES:

MAKEFLAGS+= --no-builtin-rules  # Disable the built-in implicit rules.
# MAKEFLAGS+= --warn-undefined-variables        # Warn when an undefined variable is referenced.
# MAKEFLAGS+= --include-dir=$(CURDIR)/conan     # Search DIRECTORY for included makefiles (*.mk).

export hostSystemName=$(shell uname)

ifeq (${hostSystemName},Darwin)
  export LLVM_PREFIX:=$(shell brew --prefix llvm)
  export LLVM_DIR:=$(shell realpath ${LLVM_PREFIX})
  export PATH:=${LLVM_DIR}/bin:${PATH}

  export CMAKE_CXX_STDLIB_MODULES_JSON=${LLVM_DIR}/lib/c++/libc++.modules.json
  export CXXFLAGS:=-stdlib=libc++
  export LDFLAGS=-L$(LLVM_DIR)/lib/c++ # XXX -lc++abi
  export CXX=clang++
  # FIXME: export GCOV="llvm-cov gcov"

  ### TODO: to test g++-15:
  export GCC_PREFIX:=$(shell brew --prefix gcc)
  export GCC_DIR:=$(shell realpath ${GCC_PREFIX})

  # export CMAKE_CXX_STDLIB_MODULES_JSON=${GCC_DIR}/lib/gcc/current/libstdc++.modules.json
  # export CXXFLAGS:=-stdlib=libstdc++
  # export CXX:=g++-15
  # export GCOV="gcov"
else ifeq (${hostSystemName},Linux)
  export LLVM_DIR=/usr/lib/llvm-20
  export PATH:=${LLVM_DIR}/bin:${PATH}
  export CXX=clang++-20
endif

.PHONY: all install coverage gclean distclean format demo

all: build/compile_commands.json
	ln -sf $< .
	ninja -C build

build/compile_commands.json: CMakeLists.txt GNUmakefile
	cmake -S . -B build -G Ninja \
	 -D CMAKE_EXPERIMENTAL_CXX_IMPORT_STD="d0edc3af-4c50-42ea-a356-e2862fe7a444" \
	 -D CMAKE_CXX_STDLIB_MODULES_JSON=${CMAKE_CXX_STDLIB_MODULES_JSON} \
	 -D CMAKE_CXX_STANDARD=20 -D CMAKE_CXX_EXTENSIONS=YES -D CMAKE_CXX_STANDARD_REQUIRED=YES \
	 -D CMAKE_CXX_MODULE_STD=NO \
	 -D BOOST_USE_MODULES=YES \
	 -D BUILD_TESTING=NO \
	 -D CMAKE_BUILD_TYPE=Release \
	 -D CMAKE_INSTALL_MESSAGE=LAZY \
	 -D CMAKE_SKIP_INSTALL_RULES=NO \
	 --log-level=VERBOSE --fresh \
	 # -D CMAKE_CXX_FLAGS='-fno-inline --coverage' \
	 # --trace-expand --trace-source=use-fetch-content.cmake \
	 # --debug-find-pkg=GTest

install: build/cmake_install.cmake
	cmake --install build

distclean: # XXX clean
	rm -rf build stagedir compile_commands.json
	find . -name '*~' -delete

gclean: clean
	find build -name '*.gc..' -delete

build/coverage: test
	mkdir -p $@

coverage: build/coverage
	gcovr --merge-mode-functions separate

format: distclean
	pre-commit run --all

demo: distclean
	-rm -rf /opt/local/*/*
	cmake --preset appleclang-release --fresh --log-level=VERBOSE
	cmake --workflow appleclang-release
	cmake --preset gcc-release --fresh --log-level=VERBOSE
	cmake --workflow gcc-release
	cmake --preset llvm-release --fresh --log-level=VERBOSE
	cmake --workflow llvm-release
	cmake --preset llvm-debug --fresh --log-level=VERBOSE
	cmake --workflow llvm-debug
	cmake --install build/llvm-debug --prefix=/opt/local
	cmake --install build/llvm-release --prefix=/opt/local
	tree /opt/local


# Anything we don't know how to build will use this rule.
% ::
	ninja -C build $(@)
