cmake_minimum_required(VERSION 3.1)
project(vorpal VERSION 1.1.7 LANGUAGES C CXX)

set(BOOST_VERSION 1.67.0)
set(VOWPAL_WABBIT_VERSION 8.3.1)
set(ZLIB_VERSION 1.2.11)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

# BUILD_SHARED_LIBS is a standard CMake variable, but we declare it here to make
# it prominent in the GUI.
option(BUILD_SHARED_LIBS "Build shared libraries(DLLs)." ON)

option(VORPAL_BUILD_TESTS "Build Vorpal's own tests." ON)

if (NOT CMAKE_TOOLCHAIN_FILE)
  find_package(JNI)

  if (JNI_FOUND)
      message(STATUS "Found JNI")
  endif()
endif()

include_directories(${JNI_INCLUDE_DIRS})
include(ExternalProject)

#######################################
# Fetch and Build zlib
#######################################

ExternalProject_Add(
  zlib
  DOWNLOAD_COMMAND bash -c "rmdir ${CMAKE_CURRENT_BINARY_DIR}/external/src/zlib && wget -nc -P ${CMAKE_CURRENT_BINARY_DIR}/../download/zlib http://prdownloads.sourceforge.net/libpng/zlib1211.zip?download && unzip -o ${CMAKE_CURRENT_BINARY_DIR}/../download/zlib/zlib1211.zip\?download -d ${CMAKE_CURRENT_BINARY_DIR}/external/src && mv ${CMAKE_CURRENT_BINARY_DIR}/external/src/zlib-${ZLIB_VERSION} ${CMAKE_CURRENT_BINARY_DIR}/external/src/zlib"
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
  DOWNLOAD_COMMAND bash -c "rmdir ${CMAKE_CURRENT_BINARY_DIR}/external/src/boost && wget -nc -P ${CMAKE_CURRENT_BINARY_DIR}/../download/boost https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/boost_1_67_0.zip && unzip -o ${CMAKE_CURRENT_BINARY_DIR}/../download/boost/boost_1_67_0.zip -d ${CMAKE_CURRENT_BINARY_DIR}/external/src && mv ${CMAKE_CURRENT_BINARY_DIR}/external/src/boost_1_67_0 ${CMAKE_CURRENT_BINARY_DIR}/external/src/boost"
  URL https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.zip
  PREFIX external
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/../download/boost
  INSTALL_DIR external/out/boost
  CONFIGURE_COMMAND ./bootstrap.sh --prefix=${CMAKE_CURRENT_BINARY_DIR}/external/out/boost --without-icu --with-libraries=filesystem,program_options,system
  BUILD_COMMAND ./b2 -j4
  INSTALL_COMMAND ./b2 install
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
  DOWNLOAD_COMMAND bash -c "rmdir ${CMAKE_CURRENT_BINARY_DIR}/external/src/vowpal_wabbit && wget -nc -P ${CMAKE_CURRENT_BINARY_DIR}/../download/vowpal_wabbit https://github.com/JohnLangford/vowpal_wabbit/archive/${VOWPAL_WABBIT_VERSION}.zip && unzip -o ${CMAKE_CURRENT_BINARY_DIR}/../download/vowpal_wabbit/${VOWPAL_WABBIT_VERSION}.zip -d ${CMAKE_CURRENT_BINARY_DIR}/external/src && mv ${CMAKE_CURRENT_BINARY_DIR}/external/src/vowpal_wabbit-${VOWPAL_WABBIT_VERSION} ${CMAKE_CURRENT_BINARY_DIR}/external/src/vowpal_wabbit"
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




include(TestBigEndian)
test_big_endian(VORPAL_IS_BIG_ENDIAN)

include(CheckIncludeFile)
check_include_file("byteswap.h" HAVE_BYTESWAP_H)
check_include_file("stddef.h" HAVE_STDDEF_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("sys/endian.h" HAVE_SYS_ENDIAN_H)
check_include_file("sys/mman.h" HAVE_SYS_MMAN_H)
check_include_file("sys/resource.h" HAVE_SYS_RESOURCE_H)
check_include_file("sys/time.h" HAVE_SYS_TIME_H)
check_include_file("sys/uio.h" HAVE_SYS_UIO_H)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_include_file("windows.h" HAVE_WINDOWS_H)

include(CheckLibraryExists)
check_library_exists(z zlibVersion "" HAVE_LIBZ)
check_library_exists(lzo2 lzo1x_1_15_compress "" HAVE_LIBLZO2)

include(CheckCXXSourceCompiles)
check_cxx_source_compiles(
    "int main(void) { return __builtin_expect(0, 1); }" HAVE_BUILTIN_EXPECT)

check_cxx_source_compiles(
    "int main(void) { return __builtin_ctzll(0); }" HAVE_BUILTIN_CTZ)

include(CheckSymbolExists)
check_symbol_exists("mmap" "sys/mman.h" HAVE_FUNC_MMAP)
check_symbol_exists("sysconf" "unistd.h" HAVE_FUNC_SYSCONF)

find_package(GTest QUIET)
if(GTEST_FOUND)
  set(HAVE_GTEST 1)
endif(GTEST_FOUND)

find_package(Gflags QUIET)
if(GFLAGS_FOUND)
  set(HAVE_GFLAGS 1)
endif(GFLAGS_FOUND)

configure_file(
  "${PROJECT_SOURCE_DIR}/cmake/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

# We don't want to define HAVE_ macros in public headers. Instead, we use
# CMake's variable substitution with 0/1 variables, which will be seen by the
# preprocessor as constants.
set(HAVE_STDINT_H_01  ${HAVE_STDINT_H})
set(HAVE_STDDEF_H_01  ${HAVE_STDDEF_H})
set(HAVE_SYS_UIO_H_01 ${HAVE_SYS_UIO_H})
if(NOT HAVE_STDINT_H_01)
  set(HAVE_STDINT_H_01 0)
endif(NOT HAVE_STDINT_H_01)
if(NOT HAVE_STDDEF_H_01)
  set(HAVE_STDDEF_H_01 0)
endif(NOT HAVE_STDDEF_H_01)
if(NOT HAVE_SYS_UIO_H_01)
  set(HAVE_SYS_UIO_H_01 0)
endif(NOT HAVE_SYS_UIO_H_01)

configure_file(
  "${PROJECT_SOURCE_DIR}/vorpal-stubs-public.h.in"
  "${PROJECT_BINARY_DIR}/vorpal-stubs-public.h")

add_library(vorpal "")
target_sources(vorpal
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/jni_base_learner.cc>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWActionProbsLearner.cc>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWActionScoresLearner.cc>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWLearners.cc>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWMulticlassLearner.cc>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWMultilabelsLearner.cc>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWProbLearner.cc>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWScalarLearner.cc>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWScalarsLearner.cc>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_VW.cc>
    "${PROJECT_BINARY_DIR}/config.h"

  # Only CMake 3.3+ supports PUBLIC sources in targets exported by "install".
  $<$<VERSION_GREATER:CMAKE_VERSION,3.2>:PUBLIC>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/jni_base_learner.h>
    $<INSTALL_INTERFACE:include/jni_base_learner.h>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWActionProbsLearner.h>
    $<INSTALL_INTERFACE:include/vowpalWabbit_learner_VWActionProbsLearner.h>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWActionScoresLearner.h>
    $<INSTALL_INTERFACE:include/vowpalWabbit_learner_VWActionScoresLearner.h>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWLearners.h>
    $<INSTALL_INTERFACE:include/vowpalWabbit_learner_VWLearners.h>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWMulticlassLearner.h>
    $<INSTALL_INTERFACE:include/vowpalWabbit_learner_VWMulticlassLearner.h>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWMultilabelsLearner.h>
    $<INSTALL_INTERFACE:include/vowpalWabbit_learner_VWMultilabelsLearner.h>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWProbLearner.h>
    $<INSTALL_INTERFACE:include/vowpalWabbit_learner_VWProbLearner.h>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWScalarLearner.h>
    $<INSTALL_INTERFACE:include/vowpalWabbit_learner_VWScalarLearner.h>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_learner_VWScalarsLearner.h>
    $<INSTALL_INTERFACE:include/vowpalWabbit_learner_VWScalarsLearner.h>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/vowpalWabbit_VW.h>
    $<INSTALL_INTERFACE:include/vowpalWabbit_VW.h>
)

target_include_directories(vorpal
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(vorpal
  PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

target_compile_definitions(vorpal PRIVATE -DHAVE_CONFIG_H)
if(BUILD_SHARED_LIBS)
  set_target_properties(vorpal PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif(BUILD_SHARED_LIBS)

add_dependencies(vorpal vowpal_wabbit)

include(GNUInstallDirs)
install(TARGETS vorpal
  EXPORT VorpalTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
  FILES
    "${PROJECT_SOURCE_DIR}/vorpal.h"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/VorpalConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)
install(
  EXPORT VorpalTargets
  NAMESPACE Vorpal::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Vorpal"
)
install(
  FILES
    "${PROJECT_SOURCE_DIR}/cmake/VorpalConfig.cmake"
    "${PROJECT_BINARY_DIR}/VorpalConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Vorpal"
)
