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
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(Python_EXECUTABLE "${Python3_EXECUTABLE}")
find_package(Python COMPONENTS Interpreter REQUIRED)

#-------------------------------------------------------------------------------
# Options
#-------------------------------------------------------------------------------

# Update default value for options
option(IREE_BUILD_COMPILER "Builds the IREE compiler." OFF)
option(IREE_BUILD_BUNDLED_LLVM "Builds the bundled llvm-project (vs using installed)" OFF)

#-------------------------------------------------------------------------------
# Project-wide properties
#-------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
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

if(IREE_ENABLE_ASAN)
    add_compile_options("$<IF:$<STREQUAL:${CMAKE_CXX_COMPILER_ID},MSVC>,/wd5211,-Wno-deprecated-declarations>") # This is because ASAN disable `sprintf` usage on MacOS but libyaml use it.
    add_compile_options("$<IF:$<STREQUAL:${CMAKE_CXX_COMPILER_ID},MSVC>,/fsanitize=address,-fsanitize=address>")
    add_link_options("$<IF:$<STREQUAL:${CMAKE_CXX_COMPILER_ID},MSVC>,/fsanitize=address,-fsanitize=address>")
endif(IREE_ENABLE_ASAN)

#-------------------------------------------------------------------------------
# `iree`
#-------------------------------------------------------------------------------

# Here, we will force set the options so the build won't break.

# Disable GPU backend by default as not all platform supports GPU backend.
set(IREE_HAL_DRIVER_VULKAN OFF)
set(IREE_HAL_DRIVER_METAL OFF)
set(IREE_HAL_DRIVER_CUDA OFF)

if(APPLE)
	set(IREE_HAL_DRIVER_METAL ON)
elseif(WIN32 OR LINUX OR BSD)
	set(IREE_HAL_DRIVER_VULKAN ON)
else()
	set(IREE_HAL_DRIVER_LOCAL_SYNC ON)
	set(IREE_HAL_DRIVER_LOCAL_TASK ON)
	set(IREE_HAL_EXECUTABLE_LOADER_VMVX_MODULE ON)
endif()

#-------------------------------------------------------------------------------
# `godot-cpp`
#-------------------------------------------------------------------------------
set(GODOT_COMPILE_FLAGS)
set(GODOT_LINKER_FLAGS)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# using Visual Studio C++
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /EHsc /WX") # /GF /MP
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /DTYPED_METHOD_BIND")

	if(CMAKE_BUILD_TYPE MATCHES Debug)
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /MDd") # /Od /RTC1 /Zi
	else()
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /MD /O2") # /Oy /GL /Gy
		STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	endif(CMAKE_BUILD_TYPE MATCHES Debug)

	# Disable conversion warning, truncation, unreferenced var, signed mismatch, C syntax in C++ warning
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /wd4244 /wd4305 /wd4101 /wd4018 /wd4267 /wd4576")

	add_definitions(-DNOMINMAX)

	# For warning level 4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endif()

else()

	# It might not be a good idea to link statically with system library (https://developer.apple.com/library/archive/qa/qa1118/_index.html).
	if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
		set(GODOT_LINKER_FLAGS "-static-libgcc -static-libstdc++ -Wl,-R,'$$ORIGIN'")
	endif()

	set(GODOT_COMPILE_FLAGS "-fPIC -g -Wwrite-strings")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wchar-subscripts -Wcomment -Wdisabled-optimization")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wformat -Wformat=2 -Wformat-security -Wformat-y2k")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wimport -Winit-self -Winline -Winvalid-pch")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wmissing-braces -Wmissing-format-attribute")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wmissing-include-dirs -Wmissing-noreturn -Wpointer-arith")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wredundant-decls -Wreturn-type -Wsequence-point")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wswitch -Wswitch-enum -Wtrigraphs")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wuninitialized -Wunknown-pragmas -Wunreachable-code -Wunused-label")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wunused-value -Wvariadic-macros -Wvolatile-register-var -Wno-error=attributes")

	if(IREE_ENABLE_WERROR_FLAG)
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Werror")
	endif()

	# -Wshadow -Wextra -Wall -Weffc++ -Wfloat-equal -Wstack-protector -Wunused-parameter -Wsign-compare -Wunused-variable -Wcast-align
	# -Wunused-function -Wstrict-aliasing -Wstrict-aliasing=2 -Wmissing-field-initializers -Wpacked

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

string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} ARCH)
if("${ARCH}" MATCHES aarch64)
	set(ARCH arm64)
elseif("${ARCH}" MATCHES amd64)
	set(ARCH x86_64)
endif()

#-------------------------------------------------------------------------------
# Third-party dependencies
#-------------------------------------------------------------------------------
add_subdirectory(${3RD_PARTY_DIR}/iree)
add_subdirectory(${3RD_PARTY_DIR}/godot-cpp)

#-------------------------------------------------------------------------------
# `iree.gd` library
#-------------------------------------------------------------------------------

set(LIB_NAME ${PROJECT_NAME}.${SYSTEM_NAME}.${BUILD_TYPE}.${ARCH})
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
	godot-cpp
	iree::base
	iree::hal
	iree::hal::local
	iree::modules::hal
	iree::vm
	iree::vm::bytecode::module
)
if(APPLE)
	target_link_libraries(
		${LIB_NAME}
		iree::hal::drivers::metal::registration
	)
elseif(WIN32 OR LINUX OR BSD)
	target_link_libraries(
		${LIB_NAME}
		iree::hal::drivers::vulkan::registration
	)
else()
	target_link_libraries(
		${LIB_NAME}
		iree::hal::drivers::local_sync::sync_driver
		iree::hal::drivers::local_task::task_driver
		iree::hal::local::loaders::embedded_elf_loader
		iree::hal::local::loaders::vmvx_module_loader
	)
endif()

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