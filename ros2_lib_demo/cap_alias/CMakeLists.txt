cmake_minimum_required(VERSION 3.8)
project(cap_alias LANGUAGES C)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(cap_grant_ld REQUIRED)

# build executable
add_executable(${PROJECT_NAME} /dev/null)
target_link_libraries(${PROJECT_NAME} cap_grant_ld::libcap_grant_ld)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)

install(
  TARGETS ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME}
)

# configure post install script
set(PROJECT_CAPS "cap_net_raw,cap_net_admin,cap_sys_nice=ep")
configure_file(postinst.bash.in ${CMAKE_CURRENT_BINARY_DIR}/postinst.bash @ONLY NEWLINE_STYLE UNIX)

install(
  PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/postinst.bash
  DESTINATION lib/${PROJECT_NAME}
  RENAME postinst
)

ament_package()
