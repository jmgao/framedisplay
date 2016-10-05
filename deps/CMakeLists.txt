set(SKIP_INSTALL_ALL TRUE)
add_subdirectory(zlib)

SET(PNG_SHARED OFF CACHE BOOL "Skip building libpng shared lib" FORCE)
SET(PNG_TESTS OFF CACHE BOOL "Skip building libpng tests" FORCE)
add_subdirectory(libpng)
