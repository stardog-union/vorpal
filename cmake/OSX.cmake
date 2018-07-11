SET(CMAKE_SYSTEM_NAME Darwin)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER /opt/osxcross/target/bin/o64-clang)
SET(CMAKE_RANLIB /opt/osxcross/target/bin/x86_64-apple-darwin15-ranlib)
SET(CMAKE_CXX_COMPILER /opt/osxcross/target/bin/o64-clang++-libc++)
SET(CMAKE_INSTALL_NAME_TOOL /opt/osxcross/target/bin/x86_64-apple-darwin15-install_name_tool)
#SET(CMAKE_C_LINK_EXECUTABLE /opt/osxcross/target/bin/x86_64-apple-darwin15-ld)
#SET(CMAKE_CXX_LINK_EXECUTABLE /opt/osxcross/target/bin/x86_64-apple-darwin15-ld)

# here is the target environment located
#SET(CMAKE_FIND_ROOT_PATH /home/vagrant/osxcross/target/macports/pkgs/opt/local/)
SET(CMAKE_OSX_SYSROOT /opt/osxcross/target/SDK/MacOSX10.11.sdk/)
SET(CMAKE_AR /opt/osxcross/target/bin/x86_64-apple-darwin15-ar CACHE FILEPATH "Archiver")
SET(CMAKE_LINKER /opt/osxcross/target/bin/x86_64-apple-darwin15-ld CACHE FILEPATH "Linker")
SET(CMAKE_EXE_LINKER_FLAGS "-v" CACHE STRING "Flags")

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(JNI_INCLUDE_DIRS 
    /opt/jdk-8u162-macosx/include
    /opt/jdk-8u162-macosx/include/darwin
)

set(BOOST_ENV_FLAGS_STR
    CC=${CMAKE_C_COMPILER}
    CXX=${CMAKE_CXX_COMPILER}
)

set(BOOST_CONFIG_CONTENT "
using clang-darwin : latest : \"${CMAKE_CXX_COMPILER}\" :
<rc>${CMAKE_RC_COMPILER}
<archiver>${CMAKE_AR}
<ranlib>${CMAKE_RANLIB}
${SEARCH_PATHS}
;
")
message("${BOOST_CONFIG_CONTENT}")

# Must write a configuration file. Can't be configured from the command line.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/user-config.jam "${BOOST_CONFIG_CONTENT}")

set(BOOST_BOOTSTRAP_FLAGS_STR
  --with-toolset=darwin
)

set(BOOST_BUILD_FLAGS_STR
    target-os=darwin
    threadapi=pthread
    toolset=clang-darwin-latest
    --user-config=${CMAKE_CURRENT_BINARY_DIR}/user-config.jam
)

set(SEARCH_PATHS)
foreach(PATHS ${CMAKE_PREFIX_PATH})
    set(SEARCH_PATHS "${SEARCH_PATHS}
<include>${CMAKE_PREFIX_PATH}/include
<library-path>${CMAKE_PREFIX_PATH}/lib
")
endforeach()
