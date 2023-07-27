cmake_minimum_required(VERSION 3.8)
project(cap_grant_ld LANGUAGES C)

# add default compiler flags
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CXX_FLAGS -Wall -Wextra -Wpedantic)
  add_compile_options(${CXX_FLAGS})
  # # add to C++ targets only, not C
  # add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${CXX_FLAGS}>")
endif()


find_package(ament_cmake QUIET)
# find_package(procps REQUIRED)
# find_package(cap-ng REQUIRED)

add_executable(${PROJECT_NAME} cap_grant_ld.c)
target_link_libraries(${PROJECT_NAME} procps cap-ng)
target_compile_features(${PROJECT_NAME} PUBLIC c_std_99)


# check for ROS2 / colcon build
set(IS_COLCON_BUILD FALSE)
if(DEFINED ENV{COLCON_PREFIX_PATH})
  string(REPLACE ":" ";" COLCON_PREFIX_PATH_LIST $ENV{COLCON_PREFIX_PATH})
  foreach(COLCON_PREFIX IN LISTS COLCON_PREFIX_PATH_LIST)
    # build expected build directory from one of <COLCON_PREFIX_PATH> as full path in native representation
    file(TO_CMAKE_PATH ${COLCON_PREFIX} COLCON_PREFIX)
    file(REAL_PATH "${COLCON_PREFIX}/../build/${PROJECT_NAME}/" COLCON_BUILD_DIR)
    file(TO_NATIVE_PATH "${COLCON_BUILD_DIR}/" COLCON_BUILD_DIR_NATIVE)

    # get full path to current build directory in native representation
    file(REAL_PATH "${CMAKE_CURRENT_BINARY_DIR}" CURRENT_BINARY_DIR_REAL)
    file(TO_NATIVE_PATH "${CURRENT_BINARY_DIR_REAL}/" CURRENT_BINARY_DIR_NATIVE)

    # compare if expectation and reality match
    string(COMPARE EQUAL ${COLCON_BUILD_DIR_NATIVE} ${CURRENT_BINARY_DIR_NATIVE} BINARY_DIR_MATCH_COLCON)
    if(BINARY_DIR_MATCH_COLCON)
      set(IS_COLCON_BUILD TRUE)
    endif()
  endforeach()
endif()

#########################
# build for ROS2
# https://github.com/ament/ament_tools/pull/142
if(ament_cmake_FOUND AND IS_COLCON_BUILD)
# if(DEFINED ENV{AMENT_TOOLS})
  # colcon does not show STATUS messages; but serves as info if ROS build was NOT intended
  message(STATUS "building for ROS2 using ament_cmake")

  # INSTALL
  install(
    TARGETS ${PROJECT_NAME}
    DESTINATION lib/${PROJECT_NAME}
  )

  # ## TESTING
  # if(BUILD_TESTING)
  #     # TODO
  # endif()

  ament_package()

#########################
# build standalone CMake
else()

  include(GNUInstallDirs)
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

#########################
endif()
