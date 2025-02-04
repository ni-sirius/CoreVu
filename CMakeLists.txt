cmake_minimum_required(VERSION 3.0.0)
project(CoreVu VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(GLFW_PATH "C:/workspace/CoreVu/3rdparty/glfw")
set(GLM_PATH "C:/workspace/CoreVu/3rdparty/glm-master")
set(TINYOBJLOADER_PATH "C:/workspace/CoreVu/3rdparty/tinyobjloader")
set(TRACY_PATH "C:/workspace/CoreVu/3rdparty/tracy-0.10")
set(STB_PATH "C:/workspace/CoreVu/3rdparty/stb")
add_compile_definitions(TRACY_ENABLE)

add_subdirectory(corevu)
add_subdirectory(coremem)

set(CMAKE_CXX_STANDARD 20)

# add cross-platforms source files and header files
list(APPEND APP_SOURCE
    corevu_test/main.cpp
    corevu_test/app.cpp
    corevu_test/renderer.cpp
    ${TRACY_PATH}/public/TracyClient.cpp
     )
list(APPEND APP_HEADER
    corevu_test/app.hpp
    corevu_test/gravity_system_test.hpp
    corevu_test/mem_sys_test.hpp
    corevu_test/renderer.hpp
     )

if (MSVC)
     # warning level 4
     add_compile_options(/W4 /Od)
 else()
     # additional warnings
     add_compile_options(-Wall -Wextra -Wpedantic)
 endif()

 set(CMAKE_CXX_STANDARD 20)

add_executable(CoreVuTest
    ${APP_SOURCE}
    ${APP_HEADER}
)

target_include_directories(CoreVuTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLFW_PATH}/include
    $ENV{VULKAN_SDK}/include
    ${GLM_PATH}
    ${TRACY_PATH}/public/tracy
    ${TINYOBJLOADER_PATH}
    ${STB_PATH}
)

target_link_libraries(CoreVuTest PRIVATE
    CoreVu
    ws2_32
    dbghelp
    CoreMem
)
