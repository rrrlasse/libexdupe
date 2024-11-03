cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 20)

project (demo)
add_executable(demo demo.cpp libexdupe/binding.hpp)
target_link_libraries(demo PRIVATE libexdupe)

if (MSVC)
    set_source_files_properties(demo.cpp PROPERTIES COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
	add_compile_options("/W4")
else()
	set(CMAKE_EXE_LINKER_FLAGS "-static")
	add_compile_options ("-pedantic" "-Wextra" "-Wall" "-Wno-gnu-zero-variadic-macro-arguments")
endif(MSVC)



add_subdirectory(libexdupe)
