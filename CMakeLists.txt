cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 20)

project (demo)
add_executable(demo demo.cpp libexdupe/binding.hpp)
target_link_libraries(demo PRIVATE libexdupe)
set(CMAKE_EXE_LINKER_FLAGS "-static")
if (MSVC)
    set_source_files_properties(demo.cpp PROPERTIES COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
	set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/pthread/cmake")
	add_compile_options("/W4")
else()
	add_compile_options ("-pedantic" "-Wextra" "-Wall" "-Wno-gnu-zero-variadic-macro-arguments")
endif(MSVC)



add_subdirectory(libexdupe)
