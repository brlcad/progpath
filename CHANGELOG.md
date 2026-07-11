# Changelog

All notable changes to `progpath` should be documented in this file.

## Unreleased

- Prioritize authoritative operating-system executable-path APIs ahead of
  invocation-name and `PATH`-search fallbacks.
- Convert the public distribution artifact to a configured single-header
  `progpath.h` with `PROGPATH_IMPLEMENTATION`.
- Keep compiled shared/static/package flows backed by the single-header
  implementation.
- Add direct single-header tests and installed-header consumer coverage.
- Install package documentation alongside headers and libraries.
- Add an installed `progpath(3)` man page for API discovery.
