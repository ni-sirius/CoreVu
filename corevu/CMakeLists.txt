# add cross-platforms source files and header files
list(APPEND CORE_SOURCE
    src/corevu_device.cpp
    src/corevu_window.cpp
    src/corevu_pipeline.cpp
     )
list(APPEND CORE_HEADER
     include/corevu_device.hpp
     include/corevu_window.hpp
     include/corevu_pipeline.hpp
     )

add_library(CoreVu
    ${CORE_SOURCE}
    ${CORE_HEADER}
)

set(GLFW_PATH "C:/workspace/CoreVu/3rdparty/glfw")
set(GLM_PATH "C:/workspace/CoreVu/3rdparty/glm-master")

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

