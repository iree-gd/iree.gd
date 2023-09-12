# Taking these as references:
# https://github.com/iml130/iree-template-cpp/blob/main/CMakeLists.txt
# https://github.com/godotengine/godot-cpp/blob/master/test/CMakeLists.txt

cmake_minimum_required(VERSION 3.23)

#-------------------------------------------------------------------------------
# Project configuration
#-------------------------------------------------------------------------------
project(
    iree-gd
    VERSION 0.0.1 
    LANGUAGES CXX C
)

#-------------------------------------------------------------------------------
# System packages and programs
#-------------------------------------------------------------------------------
find_program(CCACHE_FOUND ccache)
find_program(PYTHON_FOUND python REQUIRED)

#-------------------------------------------------------------------------------
# Options
#-------------------------------------------------------------------------------
option(ENABLE_ASAN "Sanitize address with ASan." OFF) # ASan might not work: https://github.com/google/sanitizers/issues/85

# Update default value for options
option(IREE_BUILD_COMPILER "Builds the IREE compiler." OFF)
option(IREE_BUILD_TESTS "Builds IREE unit tests." OFF)
option(IREE_BUILD_SAMPLES "Builds IREE sample projects." ON)
option(IREE_BUILD_BUNDLED_LLVM "Builds the bundled llvm-project (vs using installed)" OFF)
option(IREE_HAL_DRIVER_LOCAL_SYNC "Enables the 'local-sync' runtime HAL driver" ON)
option(IREE_HAL_DRIVER_LOCAL_TASK "Enables the 'local-task' runtime HAL driver" ON)
option(IREE_HAL_EXECUTABLE_LOADER_VMVX_MODULE "Enables the VMVX module loader for local HAL drivers" ON)
option(IREE_ENABLE_THREADING "Enables threading for IREE." ON)

#-------------------------------------------------------------------------------
# Project-wide properties
#-------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE STRING "Path to sources files (and header files).")
set(SAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sample)
set(3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty CACHE STRING "Path to all the thirdparty libraries.")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_BINARY_DIR}/lib/cmake/mlir") # Extend module path to allow submodules to find AddMLIR

if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

if(ENABLE_ASAN)
    set(IREE_ENABLE_ASAN ON)
    add_compile_options("$<IF:$<STREQUAL:${CMAKE_CXX_COMPILER_ID},MSVC>,/fsanitize=address,-fsanitize=address>")
    add_link_options("$<IF:$<STREQUAL:${CMAKE_CXX_COMPILER_ID},MSVC>,/fsanitize=address,-fsanitize=address>")
endif(ENABLE_ASAN)

#-------------------------------------------------------------------------------
# `godot-cpp`
#-------------------------------------------------------------------------------
set(GODOT_COMPILE_FLAGS)
set(GODOT_LINKER_FLAGS)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# using Visual Studio C++
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /EHsc /WX") # /GF /MP
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /DTYPED_METHOD_BIND")

	if(CMAKE_BUILD_TYPE MATCHES Debug)
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /MDd") # /Od /RTC1 /Zi
	else()
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /MD /O2") # /Oy /GL /Gy
		STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	endif(CMAKE_BUILD_TYPE MATCHES Debug)

	# Disable conversion warning, truncation, unreferenced var, signed mismatch
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /wd4244 /wd4305 /wd4101 /wd4018 /wd4267")

	add_definitions(-DNOMINMAX)

	# For warning level 4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endif()

else()

	set(GODOT_LINKER_FLAGS "-static-libgcc -static-libstdc++ -Wl,-R,'$$ORIGIN'")

	set(GODOT_COMPILE_FLAGS "-fPIC -g -Wwrite-strings")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wchar-subscripts -Wcomment -Wdisabled-optimization")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wformat -Wformat=2 -Wformat-security -Wformat-y2k")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wimport -Winit-self -Winline -Winvalid-pch -Werror")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wmissing-braces -Wmissing-format-attribute")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wpointer-arith")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wredundant-decls -Wreturn-type -Wsequence-point")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wswitch -Wswitch-enum -Wtrigraphs")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wuninitialized -Wunknown-pragmas -Wunreachable-code -Wunused-label")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wunused-value -Wvariadic-macros -Wvolatile-register-var -Wno-error=attributes")

	# -Wshadow -Wextra -Wall -Weffc++ -Wfloat-equal -Wstack-protector -Wunused-parameter -Wsign-compare -Wunused-variable -Wcast-align
	# -Wunused-function -Wstrict-aliasing -Wstrict-aliasing=2 -Wmissing-field-initializers

	if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wno-ignored-attributes")
	endif()

	if(CMAKE_BUILD_TYPE MATCHES Debug)
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -fno-omit-frame-pointer -O0")
	else()
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -O3")
	endif(CMAKE_BUILD_TYPE MATCHES Debug)
endif()

# Synchronized with godot-cpp's CMakeLists.txt

set(BITS 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(BITS 64)
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

if("${CMAKE_BUILD_TYPE}" MATCHES Debug OR "${CMAKE_BUILD_TYPE}" MATCHES RelWithDebInfo)
	set(GODOT_CPP_BUILD_TYPE Debug)
else()
	set(GODOT_CPP_BUILD_TYPE Release)
endif()

string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME)
string(TOLOWER ${GODOT_CPP_BUILD_TYPE} BUILD_TYPE)

if(ANDROID)
	# Added the android abi after system name
	set(SYSTEM_NAME ${SYSTEM_NAME}.${ANDROID_ABI})
endif()

#-------------------------------------------------------------------------------
# Third-party dependencies
#-------------------------------------------------------------------------------
add_subdirectory(${3RD_PARTY_DIR}/iree)
add_subdirectory(${3RD_PARTY_DIR}/godot-cpp)

#-------------------------------------------------------------------------------
# `iree.gd` library
#-------------------------------------------------------------------------------
set(LIB_NAME ${PROJECT_NAME}.${SYSTEM_NAME}.${BUILD_TYPE})
file(GLOB_RECURSE SRCS ${SRC_DIR}/*.c**)
file(GLOB_RECURSE HEADERS ${SRC_DIR}/*.h**)

add_library(${LIB_NAME} SHARED ${SRCS} ${HEADERS})
target_include_directories(
	${LIB_NAME} 
	PRIVATE 
	${3RD_PARTY_DIR}/godot-cpp ${3RD_PARTY_DIR}/godot-cpp/gdextension 
	${3RD_PARTY_DIR}/iree/runtime/src
)
target_link_libraries(
	${LIB_NAME}
	iree::base
	iree::hal
	iree::hal::drivers::local_sync::sync_driver
	iree::hal::local
	iree::hal::local::loaders::embedded_elf_loader
	iree::hal::local::loaders::vmvx_module_loader
	iree::modules::hal
	iree::vm
	iree::vm::bytecode::module
	iree::hal::drivers::local_task::task_driver
	iree::task::api
	iree::hal::drivers::vulkan::registration
	godot-cpp
)
target_compile_options(
    ${LIB_NAME}
    PRIVATE
    "$<IF:$<BOOL:${WIN32}>,/GR,-frtti>" # Enable RTTI.
)
set_property(
	TARGET 
	${LIB_NAME} 
	APPEND_STRING PROPERTY COMPILE_FLAGS 
	${GODOT_COMPILE_FLAGS}
)
set_property(
	TARGET 
	${LIB_NAME} 
	APPEND_STRING PROPERTY LINK_FLAGS 
	${GODOT_LINKER_FLAGS}
)
set_target_properties(
	${LIB_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy the libraries into `sample/` folder
add_custom_command(
    TARGET ${LIB_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
            $<TARGET_FILE:${LIB_NAME}> 
            ${SAMPLE_DIR}/extensions/iree-gd/$<TARGET_FILE_NAME:${LIB_NAME}>
)
