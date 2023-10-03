cmake_minimum_required(VERSION 3.0.0)
project(CoreVu VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_subdirectory(corevu)

# add cross-platforms source files and header files
list(APPEND APP_SOURCE
    corevu_test/main.cpp
     )
list(APPEND APP_HEADER
     )

add_executable(CoreVuTest
    ${APP_SOURCE}
    ${APP_HEADER}
)

target_include_directories(CoreVuTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/corevu_test
)

target_link_libraries(CoreVuTest PRIVATE
    CoreVu
)
