INCLUDE (FindPackageHandleStandardArgs)

find_package(PkgConfig REQUIRED QUIET)
PKG_SEARCH_MODULE(LASTFMLIB REQUIRED liblastfm liblastfmlib QUIET)

find_package_handle_standard_args(LASTFMLIB DEFAULT_MSG LASTFMLIB_FOUND)

MARK_AS_ADVANCED( LASTFMLIB_LIBRARIES LASTFMLIB_INCLUDE_DIRS )
