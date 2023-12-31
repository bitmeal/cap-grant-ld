cmake_minimum_required(VERSION 3.8)
project(cap_grant_ld LANGUAGES C)

# add default compiler flags
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CXX_FLAGS -Wall -Wextra -Wpedantic)
  add_compile_options(${CXX_FLAGS})
  # # add to C++ targets only, not C
  # add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${CXX_FLAGS}>")
endif()


# find_package(procps REQUIRED)
# find_package(cap-ng REQUIRED)

# build static library including main()
add_library(lib${PROJECT_NAME} cap_grant_ld.c)
target_link_libraries(lib${PROJECT_NAME} PUBLIC procps cap-ng)
target_compile_features(lib${PROJECT_NAME} PUBLIC c_std_99)
set_target_properties(lib${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)

# build executable from empty file and static lib
# file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/null.c "")
add_executable(${PROJECT_NAME} /dev/null)
target_link_libraries(${PROJECT_NAME} lib${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)

# check for ROS2 / colcon build
if((DEFINED ENV{COLCON} OR $ENV{_} MATCHES "colcon") AND $ENV{AMENT_PREFIX_PATH} MATCHES "ros")
  # colcon does not show STATUS messages; but serves as info if ROS build was NOT intended
  message(STATUS "building for ROS2 using ament_cmake")

  find_package(ament_cmake REQUIRED)
  set(IS_COLCON_BUILD TRUE)
endif()

#########################
# build for ROS2
if(IS_COLCON_BUILD)
  # ## TESTING
  # if(BUILD_TESTING)
  #     # TODO
  # endif()

  # INSTALL
  install(
    TARGETS ${PROJECT_NAME}
    DESTINATION lib/${PROJECT_NAME}
  )

  install(
    TARGETS lib${PROJECT_NAME}
    EXPORT export_lib${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
  )

  # export for use by ament packages
  ament_export_targets(export_lib${PROJECT_NAME})
  ament_export_libraries(lib${PROJECT_NAME})
  ament_export_dependencies()

  ament_package()

#########################
# build standalone CMake
else()

  include(GNUInstallDirs)
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

#########################
endif()
