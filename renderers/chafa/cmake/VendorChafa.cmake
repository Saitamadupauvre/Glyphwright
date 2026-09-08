include(FetchContent)
include(ExternalProject)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GW_GLIB REQUIRED IMPORTED_TARGET glib-2.0 gobject-2.0)

find_program(GW_AUTOCONF autoconf)
find_program(GW_AUTOMAKE automake)
find_program(GW_LIBTOOLIZE NAMES libtoolize glibtoolize)
if(NOT GW_AUTOCONF OR NOT GW_AUTOMAKE OR NOT GW_LIBTOOLIZE)
    message(FATAL_ERROR
        "GW_BUILD_RENDERER_CHAFA requires autoconf, automake, and libtool to build "
        "chafa from source (it has no CMake or meson build, only autotools). "
        "Install them (e.g. `apt install autoconf automake libtool pkg-config libglib2.0-dev`) "
        "and reconfigure."
    )
endif()

FetchContent_Declare(
    chafa_src
    GIT_REPOSITORY https://github.com/hpjansson/chafa.git
    GIT_TAG        1.18.2
)
FetchContent_Populate(chafa_src)

set(GW_CHAFA_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/chafa-install")

ExternalProject_Add(chafa_build
    SOURCE_DIR         "${chafa_src_SOURCE_DIR}"
    CONFIGURE_COMMAND  NOCONFIGURE=1 <SOURCE_DIR>/autogen.sh
    COMMAND            <SOURCE_DIR>/configure
                        --prefix=${GW_CHAFA_PREFIX}
                        --disable-shared
                        --enable-static
                        --with-pic
                        --without-tools
                        --enable-man=no
                        --with-avif=no
                        --with-heif=no
                        --with-jpeg=no
                        --with-svg=no
                        --with-tiff=no
                        --with-webp=no
                        --with-jxl=no
    BUILD_COMMAND      make -j
    INSTALL_COMMAND    make install
    BUILD_IN_SOURCE    TRUE
    BUILD_BYPRODUCTS   "${GW_CHAFA_PREFIX}/lib/libchafa.a"
)

add_library(chafa_vendored STATIC IMPORTED GLOBAL)
add_dependencies(chafa_vendored chafa_build)
file(MAKE_DIRECTORY "${GW_CHAFA_PREFIX}/include/chafa")
file(MAKE_DIRECTORY "${GW_CHAFA_PREFIX}/lib/chafa/include")
set_target_properties(chafa_vendored PROPERTIES
    IMPORTED_LOCATION "${GW_CHAFA_PREFIX}/lib/libchafa.a"
    INTERFACE_INCLUDE_DIRECTORIES "${GW_CHAFA_PREFIX}/include/chafa;${GW_CHAFA_PREFIX}/lib/chafa/include"
)
target_link_libraries(chafa_vendored INTERFACE PkgConfig::GW_GLIB)
