option(GW_BUILD_RENDERER_CHAFA "Build the Chafa ASCII/Kitty terminal renderer (requires autoconf, automake, libtool, pkg-config, and libglib2.0-dev on the system)" OFF)

if(GW_BUILD_RENDERER_CHAFA)
    add_subdirectory(chafa)
endif()
