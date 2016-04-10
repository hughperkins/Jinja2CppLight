cmake_minimum_required(VERSION 2.8)

if (NOT CMAKE_BUILD_TYPE)
    message("Setting build type to 'RelWithDebInfo'")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

if(UNIX)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

execute_process(COMMAND python -V ERROR_VARIABLE python_test)
if(${python_test} MATCHES "Python 2\\.7\\..*")
    #message("Python 2.7 found")
    OPTION(PYTHON_AVAILABLE "Facilitates development, not needed for normal build." ON)
else()
    OPTION(PYTHON_AVAILABLE "Facilitates development, not needed for normal build." OFF)
endif()

include_directories(.)
include_directories(src)

add_library(Jinja2CppLight SHARED src/Jinja2CppLight.cpp src/stringhelper.cpp)

if(PYTHON_AVAILABLE)
    add_custom_target(
        cog
        python ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cogapp/cog.py -q -I ${CMAKE_CURRENT_SOURCE_DIR}/cog-batteries -r ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/test/*.h
    )
    add_dependencies(Jinja2CppLight cog)
endif(PYTHON_AVAILABLE)

if(UNIX)
add_library(gtest SHARED thirdparty/gtest/gtest-all.cc)
target_link_libraries(gtest pthread)
else()
add_library(gtest thirdparty/gtest/gtest-all.cc)
endif()
target_include_directories(gtest PRIVATE thirdparty/gtest)

add_executable(unittests thirdparty/gtest/gtest_main.cc test/testJinja2CppLight.cpp test/teststringhelper.cpp)
target_link_libraries(unittests gtest)
target_link_libraries(unittests Jinja2CppLight)
target_include_directories(unittests PRIVATE thirdparty/gtest)

