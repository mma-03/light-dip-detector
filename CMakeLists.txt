cmake_minimum_required(VERSION 3.18)
project(light_sampler LANGUAGES C)

set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Wextra -Wpedantic -Werror  -g)

file(GLOB SOURCES CONFIGURE_DEPENDS app/*.c hal/*.c)
add_executable(light_sampler ${SOURCES})
target_include_directories(light_sampler PRIVATE app hal)

add_custom_command(TARGET light_sampler POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:light_sampler>
$ENV{HOME}/ensc351/public/myApps/)
