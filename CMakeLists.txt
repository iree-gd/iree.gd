# Taking these as references:
# https://github.com/iml130/iree-template-cpp/blob/main/CMakeLists.txt
# https://github.com/godotengine/godot-cpp/blob/master/test/CMakeLists.txt

cmake_minimum_required(VERSION 3.23)

# Fetch iree tools
message(STATUS "Downloading IREE tools.")
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
	iree-dist
	URL https://github.com/openxla/iree/releases/download/candidate-20230415.490/iree-dist-20230415.490-linux-x86_64.tar.xz
	DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.tmp
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools
	DOWNLOAD_EXTRACT_TIMESTAMP NEW
)
FetchContent_MakeAvailable(iree-dist)

#-------------------------------------------------------------------------------
# Project configuration
#-------------------------------------------------------------------------------

project(
    iree-gd
    VERSION 0.0.1 
    LANGUAGES CXX C
)

# Ccache for sanity
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache) # Less useful to do it for linking, see edit2
endif(CCACHE_FOUND)

# Update default value for options
option(IREE_BUILD_COMPILER "Builds the IREE compiler." OFF)
option(IREE_BUILD_TESTS "Builds IREE unit tests." OFF)
option(IREE_BUILD_SAMPLES "Builds IREE sample projects." ON)
option(IREE_BUILD_BUNDLED_LLVM "Builds the bundled llvm-project (vs using installed)" OFF)

option(IREE_HAL_DRIVER_LOCAL_SYNC "Enables the 'local-sync' runtime HAL driver" ON)
option(IREE_HAL_DRIVER_LOCAL_TASK "Enables the 'local-task' runtime HAL driver" ON)
option(IREE_HAL_EXECUTABLE_LOADER_VMVX_MODULE "Enables the VMVX module loader for local HAL drivers" ON)
option(IREE_ENABLE_THREADING "Enables threading for IREE." ON)

set(IREE_HOST_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools/bin)

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

set(BIN_NAME ${PROJECT_NAME}.${SYSTEM_NAME}.${BUILD_TYPE})
file(GLOB_RECURSE SRCS ${SRC_DIR}/*.c**)
file(GLOB_RECURSE HEADERS ${SRC_DIR}/*.h**)

add_library(${BIN_NAME} SHARED ${SRCS} ${HEADERS})
target_include_directories(
	${BIN_NAME} 
	PRIVATE 
	${3RD_PARTY_DIR}/godot-cpp ${3RD_PARTY_DIR}/godot-cpp/gdextension 
	${3RD_PARTY_DIR}/iree/runtime/src
)
target_link_libraries(
	${BIN_NAME}
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
	${BIN_NAME} 
	APPEND_STRING PROPERTY COMPILE_FLAGS 
	${GODOT_COMPILE_FLAGS}
)
set_property(
	TARGET 
	${BIN_NAME} 
	APPEND_STRING PROPERTY LINK_FLAGS 
	${GODOT_LINKER_FLAGS}
)
set_target_properties(
	${BIN_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy the libraries into `sample/` folder
# as assuming it contain a godot project to test things out.
set(SAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sample)
if(EXISTS ${SAMPLE_DIR})
	if(TARGET ${BIN_NAME})
		add_custom_command(
			TARGET ${BIN_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy 
					$<TARGET_FILE:${BIN_NAME}> 
					${SAMPLE_DIR}/extension/iree-gd/$<TARGET_FILE_NAME:${BIN_NAME}>
		)
	endif()
endif()