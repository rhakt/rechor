cmake_minimum_required(VERSION 2.8)

project(rechor CXX)

ENABLE_LANGUAGE(C)

set(TARGET ${PROJECT_NAME})

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif()

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)

if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2 /EHsc")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /MDd /Zi")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /MD")
  
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -Wno-long-long -pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
  
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -stdlib=libc++")
  else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

endif()

find_package(PkgConfig REQUIRED)
pkg_search_module(LZ4 REQUIRED liblz4)

include_directories(${LZ4_INCLUDE_DIRS} $ENV{FBXSDK_DIR}/include $ENV{FLATBUFFERS_DIR}/include)
link_directories(${LZ4_LIBRARY_DIRS} $ENV{FBXSDK_DIR}/lib)

if (MSVC)
  link_directories($ENV{FBXSDK_DIR}/lib/vs2015/x86/)
endif()

file(GLOB_RECURSE CXX_SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(${TARGET} ${CXX_SOURCE_FILES})

target_link_libraries(${TARGET} ${LZ4_LIBRARIES} libfbxsdk-md)
