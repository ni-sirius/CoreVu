# add cross-platforms paths source files and header files

set(GLFW_PATH "C:/workspace/CoreVu/3rdparty/glfw")
set(GLM_PATH "C:/workspace/CoreVu/3rdparty/glm-master")
set(TRACY_PATH "C:/workspace/CoreVu/3rdparty/tracy-0.10")
set(TINYOBJLOADER_PATH "C:/workspace/CoreVu/3rdparty/tinyobjloader")
#add_compile_definitions(TRACY_ENABLE)

list(APPEND CORE_SOURCE
    src/corevu_device.cpp
    src/corevu_window.cpp
    src/corevu_pipeline.cpp
    src/corevu_swap_chain.cpp
    src/corevu_model.cpp
    src/corevu_gameobject.cpp
    src/corevu_camera.cpp
    src/systems/render_system.cpp
    src/ext/keyboard_movement_controller.cpp
     )
list(APPEND CORE_HEADER
     include/corevu_device.hpp
     include/corevu_window.hpp
     include/corevu_pipeline.hpp
     include/corevu_swap_chain.hpp
     include/corevu_model.hpp
     include/corevu_gameobject.hpp
     include/corevu_camera.hpp
     include/systems/render_system.hpp
     include/systems/color_system.hpp
     include/systems/gravity_system.hpp
     include/systems/vector_field_system.hpp
     include/ext/keyboard_movement_controller.hpp
     )

if (MSVC)
     # warning level 4
     #add_compile_options(/W4)
 else()
     # additional warnings
     #add_compile_options(-Wall -Wextra -Wpedantic)
 endif()

 set(CMAKE_CXX_STANDARD 20)
 #set(CMAKE_CXX_STANDARD_REQUIRED ON)
 #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")

add_library(CoreVu
    ${CORE_SOURCE}
    ${CORE_HEADER}
)

target_include_directories(CoreVu PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GLFW_PATH}/include
    $ENV{VULKAN_SDK}/include
    ${GLM_PATH}
    ${TRACY_PATH}/public/tracy
    ${TINYOBJLOADER_PATH}
)

find_package(Vulkan REQUIRED)

if (MSVC)
  target_link_libraries(CoreVu PRIVATE
    ${GLFW_PATH}/lib-vc2022/glfw3_mt.lib
)
else()
  target_link_libraries(CoreVu PRIVATE
    ${GLFW_PATH}/lib-mingw-w64/libglfw3.a
)
endif()
target_link_libraries(CoreVu PRIVATE
    Vulkan::Vulkan
)

