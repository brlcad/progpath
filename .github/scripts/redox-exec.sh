#!/bin/sh
set -eu

build_dir=${1:?expected the build directory followed by a test executable}
shift
case "$build_dir" in
  /*) ;;
  *) echo 'The Redox build directory must be absolute' >&2; exit 1 ;;
esac
case "$PWD/" in
  "$build_dir/"*) ;;
  *) echo 'The Redox test must run inside its build directory' >&2; exit 1 ;;
esac
case "${1-}" in
  "$build_dir/"*) ;;
  *) echo 'Expected a test executable inside the Redox build directory' >&2; exit 1 ;;
esac

# redoxer uses newlines to delimit arguments, so keep the guest command on one line.
exec redoxer exec --output - --folder "$build_dir/:$build_dir" bash -c \
  'set -eu; cd "$1"; export PROGPATH_DEBUG="$2"; shift 2; exec "$@"' \
  bash "$PWD" "${PROGPATH_DEBUG:?}" "$@"
