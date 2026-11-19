#!/bin/sh
set -eu
# Add paths for common build tools across CFarm hosts
PATH=/opt/freeware/bin:/usr/local/bin:/usr/local64/bin:/opt/csw/bin:$PATH
export PATH

# Require a single, nonempty CI directory name before creating it.
case "${CI_WORKDIR-}" in
  progpath-ci-?*) ;;
  *) echo 'Invalid CI_WORKDIR: expected progpath-ci-<suffix>' >&2; exit 1 ;;
esac
work_suffix=${CI_WORKDIR#progpath-ci-}
case "$work_suffix" in
  *[!A-Za-z0-9.-]*) echo 'Invalid CI_WORKDIR: unsafe directory name' >&2; exit 1 ;;
esac
case "${HOME-}" in
  /*) ;;
  *) echo 'Invalid HOME: expected a nonempty absolute path' >&2; exit 1 ;;
esac
cd -P "$HOME"
work_parent=$(pwd -P)
case "$work_parent" in
  ''|/) echo 'Refusing to create a CI directory under filesystem root' >&2; exit 1 ;;
esac
readonly work_parent work_suffix

cleanup() {
  # Never delete from a changed parent or pass an absolute path to rm.
  if [ "$(pwd -P)" != "${work_parent:?}" ]; then
    echo 'Refusing cleanup: workspace parent changed' >&2
    return 1
  fi
  rm -rf "./progpath-ci-${work_suffix:?}"
}

# Arm cleanup only after this run creates a fresh directory.
# An existing directory or symlink makes mkdir fail without deletion.
mkdir "./progpath-ci-$work_suffix"
trap cleanup 0
# Keep the parent shell anchored here for cleanup, even on failure.
(
  cd "./progpath-ci-$work_suffix"
  git init
  git remote add origin "https://github.com/$REPO.git"
  # Fetch the exact tested commit, including synthetic PR merges.
  git fetch --depth 1 origin "$SHA"
  git checkout --detach FETCH_HEAD

  # Matrix options are whitespace-separated arguments; never evaluate them as shell code.
  set -f
  # shellcheck disable=SC2086
  set -- ${CFARM_CMAKE_ARGS-}
  cmake -B build -DCMAKE_BUILD_TYPE=Debug "-DPROGPATH_UTF8=${PROGPATH_UTF8_MODE:?}" "$@"
  cmake --build build
  cd build && ctest --output-on-failure --verbose
)
