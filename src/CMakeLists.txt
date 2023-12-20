cmake_minimum_required(VERSION 2.8)
#上面是最小的cmake版本
project(src)

set(CMAKE_VERBOSE_MAKEFILE ON) #显示详细的编译信息
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

#定义源文件路径
set(LIB_SRC
    log.cc
    util.cc
)

#生成一个共享库文件
add_library(src SHARED ${LIB_SRC})

#生成一个测试文件
add_executable(test ../tests/test.cc)
#测试文件依赖我们的lib
add_dependencies(test src)
#测试文件链接我们的lib
target_link_libraries(test src)

#设置输出路径
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)