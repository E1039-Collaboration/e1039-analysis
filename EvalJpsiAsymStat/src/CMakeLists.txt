# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(EvalJpsiAsymStat CXX)

file(GLOB          sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB     dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})

# ROOT dict generation
add_custom_command (
  OUTPUT EvalJpsiAsymStat_Dict.cc
  COMMAND rootcint
  ARGS -f EvalJpsiAsymStat_Dict.cc -noIncludePaths -inlineInputHeader -c -p
    -I${PROJECT_SOURCE_DIR}  -I${ROOT_PREFIX}/include
    ${dist_headers}  ${PROJECT_SOURCE_DIR}/LinkDef.h
  DEPENDS ${dist_headers} ${PROJECT_SOURCE_DIR}/LinkDef.h
)

# ROOT
find_program(ROOTCONF "root-config")
if(ROOTCONF)
  message("-- Detecting ROOT:    found at ${ROOTCONF}")
else()
  message(FATAL_ERROR "-- Detecting ROOT:    not found")
endif()
execute_process(COMMAND root-config --prefix OUTPUT_VARIABLE ROOT_PREFIX  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs   OUTPUT_VARIABLE ROOT_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${ROOT_CFLAGS}")

add_library(EvalJpsiAsymStat SHARED ${sources} EvalJpsiAsymStat_Dict.cc)
target_link_libraries(EvalJpsiAsymStat  fun4all interface_main geom_svc ktracker g4detectors UtilAna)
install(TARGETS EvalJpsiAsymStat DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
add_dependencies(install_pcm EvalJpsiAsymStat)
