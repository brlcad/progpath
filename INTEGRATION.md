# Integrating progpath

`progpath` is intentionally small: build it directly in-tree, install it for
`find_package`, or consume it with `pkg-config`.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## CMake package

After `cmake --install`, consumers can use:

```cmake
find_package(progpath REQUIRED)
target_link_libraries(myapp PRIVATE progpath::progpath)
```

Use `progpath::progpath-static` if you specifically want the static target.

## pkg-config

After install, make sure `pkg-config` can find `progpath.pc` and then use:

```sh
pkg-config --cflags --libs progpath
```

## Vendoring

If you vendor the source directly, add the project with `add_subdirectory(...)`
and link either `progpath` or `progpath-static`.

## Installed docs

`make install` also installs this file and `README.md` into the platform's
standard documentation directory for the package.
