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

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(TARGET_PATH x11)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(TARGET_PATH win64)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(TARGET_PATH macos)
else()
	message(FATAL_ERROR "Not implemented support for ${CMAKE_SYSTEM_NAME}")
endif()

# Ccache for sanity
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache) # Less useful to do it for linking, see edit2
endif(CCACHE_FOUND)

# Update default value for options
option(IREE_BUILD_COMPILER "Builds the IREE compiler." OFF)
option(IREE_BUILD_TESTS "Builds IREE unit tests." OFF)
option(IREE_BUILD_SAMPLES "Builds IREE sample projects." OFF)
option(IREE_BUILD_BUNDLED_LLVM "Builds the bundled llvm-project (vs using installed)" OFF)

# Change the default output directory
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/dist/${TARGET_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/misc/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/misc/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/misc/bin)

# Set the c++ standard to c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

if(CMAKE_BUILD_TYPE MATCHES Debug)
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

# NOTE: The RTTI setting must match what LLVM was compiled with and IREE
#       defaults to RTTI disabled in `iree_copts.cmake`.
#       RTTI is disabled for `dialect` target, but you may want to consider to
#       to pass `-fno-rtti` as a global compile option by adding the line
add_compile_options("-fno-rtti")

set(3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty CACHE STRING "Path to all the thirdparty libraries.")
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE STRING "Path to sources files (and header files).")

#-------------------------------------------------------------------------------
# Third-party dependencies
#-------------------------------------------------------------------------------

# Extend module path to allow submodules to find AddMLIR
list(APPEND CMAKE_MODULE_PATH "${PROJECT_BINARY_DIR}/lib/cmake/mlir")

add_subdirectory(${3RD_PARTY_DIR}/iree)

add_compile_definitions(NO_SAFE_CAST) # To disable `typeid` usage (-fno-rtti)
add_subdirectory(${3RD_PARTY_DIR}/godot-cpp)

#-------------------------------------------------------------------------------
# Top-level components
#-------------------------------------------------------------------------------

file(GLOB_RECURSE SRCS ${SRC_DIR}/*.c**)
file(GLOB_RECURSE HEADERS ${SRC_DIR}/*.h**)

add_library(${PROJECT_NAME} SHARED ${SRCS} ${HEADERS})
target_include_directories(
	${PROJECT_NAME} 
	PRIVATE 
	${3RD_PARTY_DIR}/godot-cpp ${3RD_PARTY_DIR}/godot-cpp/gdextension 
	${3RD_PARTY_DIR}/iree/runtime/src
)
target_link_libraries(
	${PROJECT_NAME}
	iree_base_base
	iree_hal_hal
	iree_hal_drivers_local_task_registration_registration
	iree_hal_local_loaders_vmvx_module_loader
	iree_modules_hal_hal
	iree_vm_vm
	iree_vm_bytecode_module
	godot-cpp
)
set_property(
	TARGET 
	${PROJECT_NAME} 
	APPEND_STRING PROPERTY COMPILE_FLAGS 
	${GODOT_COMPILE_FLAGS}
)
set_property(
	TARGET 
	${PROJECT_NAME} 
	APPEND_STRING PROPERTY LINK_FLAGS 
	${GODOT_LINKER_FLAGS}
)
set_target_properties(
	${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/iree-gd/lib
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/iree-gd/lib
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/iree-gd/bin
)