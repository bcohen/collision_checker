cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

#Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
set(ROS_BUILD_TYPE Release)

rosbuild_init()

rosbuild_add_boost_directories()

find_package(PkgConfig REQUIRED)
pkg_check_modules(sbpl_geometry_utils REQUIRED sbpl_geometry_utils)
include_directories(${sbpl_geometry_utils_INCLUDE_DIRS})
link_directories(${sbpl_geometry_utils_LIBRARY_DIRS})

find_package(Eigen REQUIRED)
include_directories(${EIGEN_INCLUDE_DIRS})
add_definitions(${EIGEN_DEFINITIONS})

#rosbuild_add_compile_flags(-g)

#set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#rosbuild_add_executable(test_model src/test_collision_model.cpp)
#target_link_libraries(test_model sbpl_collision_checking)

rosbuild_add_executable(test_space src/test_collision_space.cpp)
target_link_libraries(test_space sbpl_collision_checking)

rosbuild_add_executable(test_collision_checking src/test_collision_checking.cpp)
target_link_libraries(test_collision_checking sbpl_collision_checking)

