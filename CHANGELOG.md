# Changelog

All notable changes to `progpath` should be documented in this file.

## Unreleased

- Add an opt-in `PROGPATH_UTF8` build contract with strict Windows
  UTF-16 conversion, validation on byte-oriented platforms, compile-time
  capability errors, and non-ASCII path tests across the platform matrix.
- Prioritize authoritative operating-system executable-path APIs ahead of
  invocation-name and `PATH`-search fallbacks.
- Convert the public distribution artifact to a configured single-header
  `progpath.h` with `PROGPATH_IMPLEMENTATION`.
- Keep compiled shared/static/package flows backed by the single-header
  implementation.
- Add direct single-header tests and installed-header consumer coverage.
- Install package documentation alongside headers and libraries.
- Add an installed `progpath(3)` man page for API discovery.
