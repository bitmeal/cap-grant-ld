cmake_minimum_required(VERSION 3.8)
project(cap-grant-LD LANGUAGES C)

# find_package(procps REQUIRED)
# find_package(cap-ng REQUIRED)

add_executable(cap_grant_ld cap_grant_ld.c)
target_link_libraries(cap_grant_ld procps cap-ng)

include(GNUInstallDirs)
install(TARGETS cap_grant_ld RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})