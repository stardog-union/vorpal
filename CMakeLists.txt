cmake_minimum_required(VERSION 3.1)

set(default_build_type "Release")
if(CMAKE_BUILD_TYPE)
  message(STATUS "Build type is '${CMAKE_BUILD_TYPE}'.")
else()
  message(STATUS "Setting build type to '${default_build_type}'.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Build type (default ${default_build_type})" FORCE)
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

set(BOOST_VERSION 1.67.0)
set(VOWPAL_WABBIT_VERSION 8.3.1)
set(ZLIB_VERSION 1.2.11)

set(PROJECT_NAME vorpal)
project(${PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_TOOLCHAIN_FILE)
  find_package(JNI)

  if (JNI_FOUND)
      message(STATUS "Found JNI")
  endif()
endif()

include_directories(${JNI_INCLUDE_DIRS})

include(ExternalProject)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# ___  ___ ___  ___ 
#|   \| __| _ \/ __|
#| |) | _||  _/\__ \
#|___/|___|_|  |___/
#

if (NOT DEPS_LOCATION)
    set(DEPS_LOCATION "${CMAKE_BINARY_DIR}/deps")
endif()
include_directories(${DEPS_LOCATION}/include)
link_directories(${DEPS_LOCATION}/lib)


#######################################
# Fetch and Build zlib
#######################################

ExternalProject_Add(
  zlib
  DOWNLOAD_COMMAND bash -c "rmdir ${CMAKE_CURRENT_BINARY_DIR}/external/src/zlib && wget -nc -P ${CMAKE_CURRENT_BINARY_DIR}/../download/zlib http://prdownloads.sourceforge.net/libpng/zlib1211.zip?download && unzip ${CMAKE_CURRENT_BINARY_DIR}/../download/zlib/zlib1211.zip\?download -d ${CMAKE_CURRENT_BINARY_DIR}/external/src && mv ${CMAKE_CURRENT_BINARY_DIR}/external/src/zlib-${ZLIB_VERSION} ${CMAKE_CURRENT_BINARY_DIR}/external/src/zlib"
  URL http://prdownloads.sourceforge.net/libpng/zlib1211.zip?download
  PREFIX external
  INSTALL_DIR external/out/zlib
  CONFIGURE_COMMAND ./configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/external/out/zlib
  BUILD_COMMAND make -j2
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  LOG_DOWNLOAD ON
)

ExternalProject_Get_Property(zlib install_dir)
set(ZLIB_INSTALL_DIR ${install_dir})
add_library(libz STATIC IMPORTED)
set(LIBZ_A ${ZLIB_INSTALL_DIR}/lib/libz.a)
set_property(TARGET libz PROPERTY IMPORTED LOCATION ${LIBZ_A})


#######################################
# Fetch and Build Boost
#######################################

ExternalProject_Add(
  boost
  DOWNLOAD_COMMAND bash -c "rmdir ${CMAKE_CURRENT_BINARY_DIR}/external/src/boost && wget -nc -P ${CMAKE_CURRENT_BINARY_DIR}/../download/boost https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/boost_1_67_0.zip && unzip ${CMAKE_CURRENT_BINARY_DIR}/../download/boost/boost_1_67_0.zip -d ${CMAKE_CURRENT_BINARY_DIR}/external/src && mv ${CMAKE_CURRENT_BINARY_DIR}/external/src/boost_1_67_0 ${CMAKE_CURRENT_BINARY_DIR}/external/src/boost"
  PREFIX external
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/../download/boost
  INSTALL_DIR external/out/boost
  CONFIGURE_COMMAND ${BOOST_ENV_FLAGS_STR} ./bootstrap.sh ${BOOST_BOOTSTRAP_FLAGS_STR} --prefix=${CMAKE_CURRENT_BINARY_DIR}/external/out/boost --without-icu --with-libraries=filesystem,program_options,system
  BUILD_COMMAND ${BOOST_ENV_FLAGS_STR} ./b2 ${BOOST_BUILD_FLAGS_STR} -q -o boost_build.out -d3 -j4 --ignore-site-config threading=multi variant=release link=static --prefix=${CMAKE_CURRENT_BINARY_DIR}/external/out/boost --disable-icu --with-filesystem --with-program_options --with-system
  INSTALL_COMMAND ${BOOST_ENV_FLAGS_STR} ./b2 ${BOOST_BUILD_FLAGS_STR}  -q -d3 -j4 --ignore-site-config install
  BUILD_IN_SOURCE 1
  LOG_DOWNLOAD ON
)

ExternalProject_Get_Property(boost install_dir)
set(BOOST_INSTALL_DIR ${install_dir})

set(BOOST_INCLUDE_DIR ${install_dir}/include)
include_directories(${BOOST_INCLUDE_DIR})

add_library(libboost_filesystem STATIC IMPORTED)
set(LIBBOOST_FILESYSTEM_A ${install_dir}/lib/libboost_filesystem.a)
set_property(TARGET libboost_filesystem PROPERTY IMPORTED_LOCATION ${LIBBOOST_FILESYSTEM_A})

add_library(libboost_program_options STATIC IMPORTED)
set(LIBBOOST_PROGRAM_OPTIONS_A ${install_dir}/lib/libboost_program_options.a)
set_property(TARGET libboost_program_options PROPERTY IMPORTED_LOCATION ${LIBBOOST_PROGRAM_OPTIONS_A})

add_library(libboost_system STATIC IMPORTED)
set(LIBBOOST_SYSTEM_A ${install_dir}/lib/libboost_system.a)
set_property(TARGET libboost_system PROPERTY IMPORTED_LOCATION ${LIBBOOST_SYSTEM_A})


#######################################
# Fetch and Build Vowpal Wabbit
#######################################

set(VOWPAL_WABBIT_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/out/vowpal_wabbit)
ExternalProject_Add(
  vowpal_wabbit
  DEPENDS boost zlib
  DOWNLOAD_COMMAND bash -c "rmdir ${CMAKE_CURRENT_BINARY_DIR}/external/src/vowpal_wabbit && wget -nc -P ${CMAKE_CURRENT_BINARY_DIR}/../download/vowpal_wabbit https://github.com/JohnLangford/vowpal_wabbit/archive/${VOWPAL_WABBIT_VERSION}.zip && unzip ${CMAKE_CURRENT_BINARY_DIR}/../download/vowpal_wabbit/${VOWPAL_WABBIT_VERSION}.zip -d ${CMAKE_CURRENT_BINARY_DIR}/external/src && mv ${CMAKE_CURRENT_BINARY_DIR}/external/src/vowpal_wabbit-${VOWPAL_WABBIT_VERSION} ${CMAKE_CURRENT_BINARY_DIR}/external/src/vowpal_wabbit"
  URL https://github.com/JohnLangford/vowpal_wabbit/archive/${VOWPAL_WABBIT_VERSION}.zip
  PREFIX external
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/../download/vowpal_wabbit
  INSTALL_DIR external/out/vowpal_wabbit
  CONFIGURE_COMMAND ./configure -enable-shared --prefix=${VOWPAL_WABBIT_INSTALL_DIR} --enable-dependency-tracking --with-boost=${BOOST_INSTALL_DIR} --with-zlib=${ZLIB_INSTALL_DIR} || true
  BUILD_COMMAND make -j2
  INSTALL_COMMAND make install && sh -c "cp vowpalwabbit/*.h ${VOWPAL_WABBIT_INSTALL_DIR}/include/vowpalwabbit" && sh -c "cp -r rapidjson/include/rapidjson ${VOWPAL_WABBIT_INSTALL_DIR}/include"
  BUILD_IN_SOURCE 1
  LOG_DOWNLOAD ON
)

ExternalProject_Get_Property(vowpal_wabbit install_dir)
set(VOWPAL_WABBIT_INCLUDE_DIR ${install_dir}/include)
include_directories(${VOWPAL_WABBIT_INCLUDE_DIR})


#######################################
# RocksDB Required OS flags
########################################
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    add_definitions(-DOS_MACOSX)
endif ()


#######################################
# Build vorpal
#######################################

set(VORPAL_COMMON_FLAGS "-Wall -Werror=reorder -Wno-unused-function -pedantic -Wno-unknown-pragmas -Wno-vla -pthread -U_FORTIFY_SOURCE -fno-omit-frame-pointer -fno-common")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} ${VORPAL_COMMON_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} ${VORPAL_COMMON_FLAGS}")

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

add_library(vorpal SHARED ${SOURCES} ${HEADERS})
target_link_libraries(vorpal)

add_dependencies(vorpal vowpal_wabbit)

#target_link_libraries(vorpal libboost_program_options libz -static-libstdc++)
