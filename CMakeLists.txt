cmake_minimum_required(VERSION 3.21)

#use build.bat / build.sh instead of cmake

if(EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
	set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed")
else()
    message(FATAL_ERROR 
        "vcpkg could not be found. If you already have vcpkg installed,
		then set the environment variable VCPKG_ROOT to the absolute path
		where vcpkg was installed. For some reason vcpkg does not set this 
		environment variable automatically when it gets installed..."
    )
endif()

project(Pong LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(raylib CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.c)

target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${raylib_LIBRARIES})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

add_custom_target(copy_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
	"${CMAKE_SOURCE_DIR}/sounds" "${CMAKE_CURRENT_BINARY_DIR}/sounds"
)
add_dependencies(${PROJECT_NAME} copy_resources)