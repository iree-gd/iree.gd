# References: 
# https://github.com/iml130/iree-template-cpp/blob/main/CMakeLists.txt
# https://github.com/godotengine/godot-cpp/blob/master/test/CMakeLists.txt

cmake_minimum_required(VERSION 3.23)

project(
    iree-gd
    VERSION 0.0.1 
    LANGUAGES CXX C
)

find_program(CCACHE_FOUND ccache)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(Python_EXECUTABLE "${Python3_EXECUTABLE}")
find_package(Python COMPONENTS Interpreter REQUIRED)

# Update default value for options
option(IREE_BUILD_COMPILER "Builds the IREE compiler." OFF)
option(IREE_BUILD_TESTS "Builds IREE unit tests." OFF)
option(IREE_BUILD_BUNDLED_LLVM "Builds the bundled llvm-project (vs using installed)." OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64") # Enforce universal binary only on Mac OS.
endif()

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

# Disable GPU backend by default as not all platform supports GPU backend.
set(IREE_HAL_DRIVER_VULKAN OFF)
set(IREE_HAL_DRIVER_METAL OFF)
set(IREE_HAL_DRIVER_CUDA OFF)

if(APPLE)
	set(IREE_HAL_DRIVER_METAL ON)
elseif(WIN32 OR LINUX OR BSD OR ANDROID)
	set(IREE_HAL_DRIVER_VULKAN ON)
else()
	set(IREE_HAL_DRIVER_LOCAL_SYNC ON)
	set(IREE_HAL_DRIVER_LOCAL_TASK ON)
	set(IREE_HAL_EXECUTABLE_LOADER_VMVX_MODULE ON)
endif()

if("${CMAKE_BUILD_TYPE}" MATCHES Debug OR "${CMAKE_BUILD_TYPE}" MATCHES RelWithDebInfo)
	set(IREEGD_BUILD_TYPE "debug")
	set(GODOTCPP_TARGET "template_debug")
else()
	set(IREEGD_BUILD_TYPE "release")
	set(GODOTCPP_TARGET "template_release")
endif()

add_subdirectory(${3RD_PARTY_DIR}/godot-cpp)
add_subdirectory(${3RD_PARTY_DIR}/iree)

file(GLOB_RECURSE SRCS ${SRC_DIR}/*.c**)
file(GLOB_RECURSE HEADERS ${SRC_DIR}/*.h**)

string(TOLOWER ${CMAKE_SYSTEM_NAME} IREEGD_SYSTEM_NAME)
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} IREEGD_ARCH)

if("${IREEGD_ARCH}" MATCHES aarch64)
	set(IREEGD_ARCH arm64)
elseif("${IREEGD_ARCH}" MATCHES arm)
    set(IREEGD_ARCH arm64)
elseif("${IREEGD_ARCH}" MATCHES amd64)
	set(IREEGD_ARCH x86_64)
endif()

if("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
	if("${CMAKE_OSX_ARCHITECTURES}" MATCHES "arm64;x86_64")
		set(IREEGD_ARCH universal)
	endif()
endif()

set(IREEGD_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/sample/addons/iree-gd)
set(IREEGD ${PROJECT_NAME}.${IREEGD_SYSTEM_NAME}.${IREEGD_BUILD_TYPE}.${IREEGD_ARCH})

if("${IREEGD_SYSTEM_NAME}" MATCHES "ios")
	add_library(${IREEGD} STATIC)
else()
	add_library(${IREEGD} SHARED)
endif()

target_sources(
	${IREEGD}
	PRIVATE 
	${SRCS} 
	${HEADERS}
)

target_link_libraries(
	${IREEGD}
	godot-cpp
	iree::base
	iree::hal
	iree::hal::local
	iree::modules::hal
	iree::vm
	iree::vm::bytecode::module
)

set_target_properties(
	${IREEGD}
	PROPERTIES
	CXX_VISIBILITY_PRESET ${GODOTCPP_SYMBOL_VISIBILITY}

	POSITION_INDEPENDENT_CODE ON
	BUILD_RPATH_USE_ORIGIN ON

	# Try to ensure only static libraries are selected to be linked to.
	LINK_SEARCH_START_STATIC ON
	LINK_SEARCH_END_STATIC ON

	PREFIX "lib"

	ARCHIVE_OUTPUT_DIRECTORY ${IREEGD_OUTPUT_DIR}
	LIBRARY_OUTPUT_DIRECTORY ${IREEGD_OUTPUT_DIR}
	RUNTIME_OUTPUT_DIRECTORY ${IREEGD_OUTPUT_DIR}
)

if("${IREEGD_SYSTEM_NAME}" MATCHES "darwin")
	set_target_properties(
		${IREEGD}
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${IREEGD_OUTPUT_DIR}/libiree-gd.darwin.${IREEGD_BUILD_TYPE}.framework"
		LIBRARY_OUTPUT_DIRECTORY "${IREEGD_OUTPUT_DIR}/libiree-gd.darwin.${IREEGD_BUILD_TYPE}.framework"
		RUNTIME_OUTPUT_DIRECTORY "${IREEGD_OUTPUT_DIR}/libiree-gd.darwin.${IREEGD_BUILD_TYPE}.framework"
	)
endif()

target_compile_options(
	${IREEGD}
	PRIVATE
	"$<IF:$<BOOL:${WIN32}>,/GR,-frtti>" # Enable RTTI.
	"$<$<BOOL:${WIN32}>:/wd4576>" # Disable C4576.
)

if(APPLE)
	target_link_libraries(
		${IREEGD}
		iree::hal::drivers::metal::registration
	)
elseif(WIN32 OR LINUX OR BSD OR ANDROID)
	target_link_libraries(
		${IREEGD}
		iree::hal::drivers::vulkan::registration
	)
else()
	target_link_libraries(
		${IREEGD}
		iree::hal::drivers::local_sync::sync_driver
		iree::hal::drivers::local_task::task_driver
		iree::hal::local::loaders::embedded_elf_loader
		iree::hal::local::loaders::vmvx_module_loader
	)
endif()
