cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 20)

project (demo)
add_executable(demo demo.cpp libexdupe/libexdupe_binding.hpp)
target_link_libraries(demo PRIVATE libexdupe)

if (MSVC)
	# Make VS not use Debug and Release sub dirs
    set_source_files_properties(demo.cpp PROPERTIES COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
	add_compile_options("/W4")
	set_target_properties(demo PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
	)
else()
	set(CMAKE_EXE_LINKER_FLAGS "-static")
	add_compile_options ("-pedantic" "-Wextra" "-Wall" "-Wno-gnu-zero-variadic-macro-arguments")
endif(MSVC)

add_subdirectory(libexdupe)