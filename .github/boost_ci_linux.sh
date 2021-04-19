#!/bin/bash

set -ex

ci_print_packages_info() {
  if [ -n "$CI_DEBUG" ]; then
      echo $1
      whoami
      dpkg -l
      echo "Checking /usr/local"
      ls -al /usr/local
      echo "Checking clang"
      which clang++ || true
      clang++ --version || true
      ls -al /etc/alternatives
      ls -al /usr/bin/c*
  fi
}

ci_remove_packages() {
  while [ $# -gt 0 ]; do
    case "$1" in
      --packages=*)
        local CI_PACKAGES_TO_REMOVE="${1#*=}"
        ;;
      *)
        echo "Error: invalid argument $1"
        exit 1
    esac
    shift
  done


  ci_print_packages_info "Before package removal or install"
  if [ -n "$CI_PACKAGES_TO_REMOVE" ]; then
      echo "About to run sudo apt-get purge -y $CI_PACKAGES_TO_REMOVE"
      sudo apt-get purge -y $CI_PACKAGES_TO_REMOVE
  fi
}

ci_install_packages() {
  while [ $# -gt 0 ]; do
    case "$1" in
      --packages=*)
        local CI_PACKAGES="${1#*=}"
        ;;
      --llvm-add-ppa=*)
        local CI_LLVM_ADD_PPA="${1#*=}"
        ;;
      --llvm-version=*)
        local CI_LLVM_VER="${1#*=}"
        ;;
      --ppa-sources=*)
        local CI_PPA_SOURCES="${1#*=}"
        ;;
      *)
        echo "Error: invalid argument $1"
        exit 1
    esac
    shift
  done

  for i in {1..3}; do
    sudo -E apt-add-repository -y "ppa:ubuntu-toolchain-r/test" && break || sleep 2
  done

  if test -n "${CI_LLVM_ADD_PPA}" ; then
      wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
      local CI_LLVM_OS=$(lsb_release -cs)
      if test -n "${CI_LLVM_VER}" ; then
          sudo -E apt-add-repository "deb http://apt.llvm.org/${CI_LLVM_OS}/ llvm-toolchain-${CI_LLVM_OS}-${CI_LLVM_VER} main"
      else
          # Snapshot (i.e. trunk) build of clang
          sudo -E apt-add-repository "deb http://apt.llvm.org/${CI_LLVM_OS}/ llvm-toolchain-${CI_LLVM_OS} main"
      fi
  fi
  echo ">>>>> APT: UPDATE.."
  sudo -E apt-get -o Acquire::Retries=3 update
  if test -n "${CI_PPA_SOURCES}" ; then
      echo ">>>>> APT: INSTALL SOURCES.."
      for SOURCE in $CI_PPA_SOURCES; do
          sudo -E apt-add-repository ppa:$SOURCE
      done
  fi
  echo ">>>>> APT: INSTALL ${CI_PACKAGES}.."
  sudo -E DEBIAN_FRONTEND=noninteractive apt-get -o Acquire::Retries=3 -y --no-install-suggests --no-install-recommends install ${CI_PACKAGES}
  ci_print_packages_info "After package removal and install"
}

ci_prepare_boost_libraries() {
  while [ $# -gt 0 ]; do
    case "$1" in
      --git-branch=*)
        local CI_GIT_BRANCH="${1#*=}"
        ;;
      --repo-name=*)
        local CI_REPO_NAME="${1#*=}"
        ;;
      --project-dir=*)
        local CI_PROJECT_DIR="${1#*=}"
        ;;
      --in-libs-dir=*)
        local CI_BOOST_IN_LIBS_DIR="${1#*=}"
        ;;
      --boost-root-dir=*)
        local CI_BOOST_ROOT_DIR="${1#*=}"
        ;;
      --llvm-version=*)
        local CI_LLVM_VER="${1#*=}"
        ;;
      *)
        echo "Error: invalid argument $1"
        exit 1
    esac
    shift
  done

  if [ -z "$CI_GIT_BRANCH" ]; then
      echo "--git-branch is not set"
      exit 1
  fi
  if [ -z "$CI_REPO_NAME" ]; then
      echo "--repo-name is not set"
      exit 1
  fi

  local CI_PROJECT_DIR=${CI_PROJECT_DIR:-$(pwd)}
  local CI_BOOST_IN_LIBS_DIR=${CI_BOOST_IN_LIBS_DIR:-$(basename $CI_REPO_NAME)}
  local CI_BOOST_ROOT_DIR=${CI_BOOST_ROOT_DIR:-$HOME/boost-local}

  local BOOST_BRANCH=develop && [ "$CI_GIT_BRANCH" == "master" ] && BOOST_BRANCH=master || true
  git clone -b $BOOST_BRANCH --depth 10 https://github.com/boostorg/boost.git $CI_BOOST_ROOT_DIR
  cd $CI_BOOST_ROOT_DIR
  git submodule update --init --depth 10 --jobs 2 tools/build tools/boostdep tools/inspect libs/filesystem
  python tools/boostdep/depinst/depinst.py --git_args "--depth 10 --jobs 2" $CI_BOOST_ROOT_DIR/libs/filesystem
  echo "Testing $CI_BOOST_ROOT_DIR/libs/$CI_BOOST_IN_LIBS_DIR moved from $CI_PROJECT_DIR, branch $BOOST_BRANCH"
  rm -rf $CI_BOOST_ROOT_DIR/libs/$CI_BOOST_IN_LIBS_DIR || true
  cp -rp $CI_PROJECT_DIR $CI_BOOST_ROOT_DIR/libs/$CI_BOOST_IN_LIBS_DIR
  python tools/boostdep/depinst/depinst.py --git_args "--depth 10 --jobs 2" $CI_BOOST_IN_LIBS_DIR
  git status
  ./bootstrap.sh
  ./b2 headers
  ./b2 -j4 variant=debug tools/inspect/build
  echo "using gcc ;" >> ~/user-config.jam
  echo "using clang ;" >> ~/user-config.jam
  if [ -n "$CI_LLVM_VER" ]; then
      echo "using clang : ${CI_LLVM_VER} : clang++-${CI_LLVM_VER} ;" >> ~/user-config.jam;
  fi
  echo "using clang : libc++ : clang++-libc++ ;" >> ~/user-config.jam
}

ci_preprocess_coverage() {
  while [ $# -gt 0 ]; do
    case "$1" in
      --repo-name=*)
        local CI_REPO_NAME="${1#*=}"
        ;;
      --project-dir=*)
        local CI_PROJECT_DIR="${1#*=}"
        ;;
      --in-libs-dir=*)
        local CI_BOOST_IN_LIBS_DIR="${1#*=}"
        ;;
      --boost-root-dir=*)
        local CI_BOOST_ROOT_DIR="${1#*=}"
        ;;
      --llvm-version=*)
        local CI_LLVM_VER="${1#*=}"
        ;;
      --ignore-coverage=*)
        local CI_IGNORE_COVERAGE="${1#*=}"
        ;;
      --gcov-tool=*)
        local CI_GCOVTOOL="${1#*=}"
        ;;
      *)
        echo "Error: invalid argument $1"
        exit 1
    esac
    shift
  done
  if [ -z "$CI_REPO_NAME" ]; then
      echo "--repo-name is not set"
      exit 1
  fi
  if [ -n "$CI_GCOVTOOL" ] && [ -n "$CI_LLVM_VER" ]; then
      echo "Only --gcov-tool or --llvm-version must be set at the same time"
      exit 1
  fi

  local CI_IGNORE_COVERAGE=${CI_IGNORE_COVERAGE:-''}
  local CI_PROJECT_DIR=${CI_PROJECT_DIR:-$(pwd)}
  local CI_BOOST_IN_LIBS_DIR=${CI_BOOST_IN_LIBS_DIR:-$(basename $CI_REPO_NAME)}
  local CI_BOOST_ROOT_DIR=${CI_BOOST_ROOT_DIR:-$HOME/boost-local}

  mkdir -p $CI_PROJECT_DIR/coverals

  cd $CI_BOOST_ROOT_DIR/libs/$CI_BOOST_IN_LIBS_DIR/test/
  find ../../../bin.v2/ -name "*.gcda" -exec cp "{}" $CI_PROJECT_DIR/coverals/ \;
  find ../../../bin.v2/ -name "*.gcno" -exec cp "{}" $CI_PROJECT_DIR/coverals/ \;
  find ../../../bin.v2/ -name "*.da" -exec cp "{}" $CI_PROJECT_DIR/coverals/ \;
  find ../../../bin.v2/ -name "*.no" -exec cp "{}" $CI_PROJECT_DIR/coverals/ \;
  wget https://github.com/linux-test-project/lcov/archive/v1.14.zip
  unzip v1.14.zip

  if [ -n "$CI_LLVM_VER" ]; then
      echo -e "#!/bin/bash\nexec llvm-cov-${CI_LLVM_VER} gcov \"\$@\"" > lcov-1.14/bin/gcov_for_clang.sh
      chmod 755 lcov-1.14/bin/gcov_for_clang.sh
      local LCOV="`pwd`/lcov-1.14/bin/lcov --gcov-tool `pwd`/lcov-1.14/bin/gcov_for_clang.sh"
  else
      local LCOV="`pwd`/lcov-1.14/bin/lcov --gcov-tool $CI_GCOVTOOL"
  fi

  echo "$LCOV --directory $CI_PROJECT_DIR/coverals --base-directory `pwd` --capture --output-file $CI_PROJECT_DIR/coverals/coverage.info"
  $LCOV --directory $CI_PROJECT_DIR/coverals --base-directory `pwd` --capture --output-file $CI_PROJECT_DIR/coverals/coverage.info
  cd $CI_BOOST_ROOT_DIR
  $LCOV --remove $CI_PROJECT_DIR/coverals/coverage.info "/usr*" "*/$CI_BOOST_IN_LIBS_DIR/test/*" $CI_IGNORE_COVERAGE "*/$CI_BOOST_IN_LIBS_DIR/tests/*" "*/$CI_BOOST_IN_LIBS_DIR/examples/*" "*/$CI_BOOST_IN_LIBS_DIR/example/*" -o $CI_PROJECT_DIR/coverals/coverage.info
  local OTHER_LIBS=`grep "submodule .*" .gitmodules | sed 's/\[submodule\ "\(.*\)"\]/"\*\/boost\/\1\.hpp" "\*\/boost\/\1\/\*"/g'| sed "/\"\*\/boost\/$CI_BOOST_IN_LIBS_DIR\/\*\"/d" | sed ':a;N;$!ba;s/\n/ /g'`
  echo $OTHER_LIBS
  eval "$LCOV --remove $CI_PROJECT_DIR/coverals/coverage.info $OTHER_LIBS -o $CI_PROJECT_DIR/coverals/coverage.info"
}

