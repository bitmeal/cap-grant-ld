# cap grant LD
> *🚨 security risk incoming!*


Run executables with ***capabilities***, while using ***shared libraries*** from locations in ***`$LD_LIBRARY_PATH`***, as ***non-root*** user!

* [intro](#intro)
* [what it does](#what-it-does)
* [usage](#usage)
* [building](#building)
* [use with ROS2](#use-with-ros2)
    * [ROS2 usage](#ros2-usage)
    * [ROS2 building](#ros2-building)
    * [ROS2 library](#ros2-library)



## intro
Working on embedded devices, with special hardware or on realtime systems, at times requires additional capabilities for a process to fulfill its task. Either while developing, or when using frameworks with their default install location not in system directories (looking at you *ROS*!), dynamic loading of libraries in `$LD_LIBRARY_PATH` is a frequent requirement (or at least convenient).

Setting capabilities on executables is straightforward, with one pitfall: When capabilites are set on an executable, `$LD_LIBRARY_PATH` is removed from the processes environment for security reasons; keep in mind these are good reasons! The solutions for overcoming this, as hard baking library paths, installing libraries to system locations, or running applications as root, may be a higher risk or just less desirable than the calculated risk of injection of malicious shared libraries with known capabilities and privileges. Enter *cap grant LD*!

## what it does
By not setting capabilities on the target executable directly, we can inject `$LD_LIBRARY_PATH` - without it being removed - into the processes environment, while granting capabilities at runtime:

1. *cap grant LD* clones all permissions from its *PERMITTED* or *EFFECTIVE* set as `eip` to the ambient set of the callee process
2. injects `$LD_LIBRARY_PATH` from it's parents (!) initial (!) environment to the callee process

**Important:** Make sure the parent process of `cap_grant_ld` has `$LD_LIBRARY_PATH` in its initial environment, readable using *procps* (in *procfs*, `/proc/<ppid>/environ`)!


## usage
1. set capabilities to give to process on `cap_grant_ld` executable
2. run `cap_grant_ld <executable> [<arguments> [...]]`

Options
* `-v` don't be silent; tell what's happening
* `-p` clone permitted capabilities to ambient set *(default)*
* `-e` clone effective capabilities to ambient set
* `-l` disable `$LD_LIBRARY_PATH` injection

```bash
# EXAMPLE capsh

$ ./cap_grant_ld -v capsh --has-p=cap_net_raw
cloning PERMITTED caps:
injecting environment from [<ppid>]:
- LD_LIBRARY_PATH=/some/path
executing: capsh --has-p=cap_net_raw
cap[cap_net_raw] not permitted


# with caps
$ sudo setcap cap_net_raw,cap_net_admin,cap_sys_nice+ep ./cap_grant_ld
$ ./cap_grant_ld -v capsh --has-p=cap_net_raw
cloning PERMITTED caps:
- [12] net_admin
- [13] net_raw
- [23] sys_nice
injecting environment from [<ppid>]:
- LD_LIBRARY_PATH=/some/path
executing: capsh --has-p=cap_net_raw
# empty stdout indicates success of capsh --has-p
```

## building
requirement:
* `libcap-ng-dev`
* `libprocps-dev`

```bash
# "raw"
gcc cap_grant_ld.c -lcap-ng -lprocps -o cap_grant_ld

# make
make install

# cmake
cmake -B build .
cmake --build build --target install
```

## use with ROS2
### ROS2 usage
set permissions on the executable, find its path using:
```bash
$ readlink -f `ros2 pkg prefix cap_grant_ld`/lib/cap_grant_ld/cap_grant_ld
```

use as:
* `prefix='ros2 run cap_grant_ld cap_grant_ld'` with ROS2 **launchfiles**
* `ros2 run cap_grant_ld cap_grant_ld`
* *standalone* from install path

### ROS2 building
The `CMakeLists.txt` detects if it is used within a ROS2 colcon/ament workspace and builds for ROS2 automatically. The workspace has to be configured correctl, i.e. its `setup.bash` has to be sourced prior!

### ROS2 library
The package exports a static library and CMake target. These allow you to easily provide a copy of *cap_grant_ld*  with custom name, in your package, to set the required capabilities for your application onyl. Neat, is it?

Find an example including a post-install script to set permissions in `ros2_lib_demo/cap_alias`.

```xml
<!-- package.xml /-->

<build_depend>cap-grant-ld</build_depend>
```
```cmake
# CMakeLists.txt

add_executable(${PROJECT_NAME}_grant /dev/null)
target_link_libraries(${PROJECT_NAME}_grant cap_grant_ld::libcap_grant_ld)
set_target_properties(${PROJECT_NAME}_grant PROPERTIES LINKER_LANGUAGE C)
```
The library `cap_grant_ld::libcap_grant_ld` includes all functionality (including `main()`) and link rules to `cap-ng` and `procps`. `add_executable()` needs at least one source file for an executable target. Using `/dev/null` as source does not allow CMake to deduce the language type, thus we have to set it manually.
