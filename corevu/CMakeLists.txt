# add cross-platforms source files and header files
list(APPEND CORE_SOURCE
    src/corevu_device.cpp
    src/corevu_window.cpp
    src/corevu_pipeline.cpp
    src/corevu_swap_chain.cpp
    src/corevu_model.cpp
    src/corevu_gameobject.cpp
    src/systems/render_system.cpp
     )
list(APPEND CORE_HEADER
     include/corevu_device.hpp
     include/corevu_window.hpp
     include/corevu_pipeline.hpp
     include/corevu_swap_chain.hpp
     include/corevu_model.hpp
     include/corevu_gameobject.hpp
     include/systems/render_system.hpp
     )

if (MSVC)
     # warning level 4
     add_compile_options(/W4)
 else()
     # additional warnings
     add_compile_options(-Wall -Wextra -Wpedantic)
 endif()

add_library(CoreVu
    ${CORE_SOURCE}
    ${CORE_HEADER}
)

set(GLFW_PATH "C:/workspace/CoreVu/3rdparty/glfw")
set(GLM_PATH "C:/workspace/CoreVu/3rdparty/glm-master")
set(CMAKE_CXX_STANDARD 20)

target_include_directories(CoreVu PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GLFW_PATH}/include
    $ENV{VULKAN_SDK}/include
    ${GLM_PATH}
)

find_package(Vulkan REQUIRED)

target_link_libraries(CoreVu PRIVATE
    ${GLFW_PATH}/lib-mingw-w64/libglfw3.a
    #${GLFW_PATH}/lib-vc2022/glfw3_mt.lib
    Vulkan::Vulkan
)

