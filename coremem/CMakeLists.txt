# add cross-platforms source files and header files
list(APPEND CORE_SOURCE
    src/IAllocator.cpp
    src/LinearAllocator.cpp
    src/StackAllocator.cpp
    src/PoolAllocator.cpp
     )
list(APPEND CORE_HEADER
     include/IAllocator.hpp
     include/LinearAllocator.hpp
     include/StackAllocator.hpp
     include/PoolAllocator.hpp
     )

if (MSVC)
     # warning level 4
     add_compile_options(/W4)
 else()
     # additional warnings
     add_compile_options(-Wall -Wextra -Wpedantic)
 endif()

add_library(CoreMem
    ${CORE_SOURCE}
    ${CORE_HEADER})

#set(GLFW_PATH "C:/workspace/CoreVu/3rdparty/glfw")
#set(GLM_PATH "C:/workspace/CoreVu/3rdparty/glm-master")
set(CMAKE_CXX_STANDARD 20)

target_include_directories(CoreMem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
#    ${GLFW_PATH}/include
#    $ENV{VULKAN_SDK}/include
    ${GLM_PATH}
)

#find_package(Vulkan REQUIRED)

#target_link_libraries(CoreMem PRIVATE
#    ${GLFW_PATH}/lib-mingw-w64/libglfw3.a
#    #${GLFW_PATH}/lib-vc2022/glfw3_mt.lib
#    Vulkan::Vulkan
#)

