project(hexl_example LANGUAGES C CXX)
cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 11)

include_directories(include)
include_directories(/usr/include)
include_directories(/usr/local/include)

include(FindPkgConfig)
if(NOT PKG_CONFIG_FOUND)
  message(FATAL_ERROR "pkg-config not found!" )
endif()

#set(CMAKE_CXX_FLAGS "-O3 -fopenmp")
#set(CMAKE_C_FLAGS "-O3 -fopenmp")

pkg_check_modules(HEXL REQUIRED IMPORTED_TARGET hexl)
find_package(CURL REQUIRED)

get_target_property(HEXL_LINK_LIBRARIES PkgConfig::HEXL INTERFACE_LINK_LIBRARIES)
if(${CMAKE_VERSION} VERSION_LESS "3.15.0" AND ${HEXL_LINK_LIBRARIES} MATCHES "debug")
  set_target_properties(PkgConfig::HEXL PROPERTIES INTERFACE_LINK_OPTIONS "-fsanitize=address")
  set_target_properties(PkgConfig::HEXL PROPERTIES INTERFACE_COMPILE_OPTIONS "-fsanitize=address")
endif()

set_target_properties(PkgConfig::HEXL PROPERTIES INTERFACE_COMPILE_OPTIONS "-g")

add_executable(eval ios_eval.cpp)
target_link_libraries(eval pthread crypto PkgConfig::HEXL)
target_link_libraries(eval CURL::libcurl)

#add_executable(init ios_init.cpp)
#target_link_libraries(init pthread crypto PkgConfig::HEXL)

#add_executable(enc build/ios_enc_mobile.cpp)
#add_executable(enc_server build/ios_enc_server.cpp)
#add_executable(dec build/ios_dec_mobile.cpp)
