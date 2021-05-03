cmake_minimum_required (VERSION 3.14)

project(H-alfaSSed)
set(HALFASSED_MAJOR_VERSION 0)
set(HALFASSED_MINOR_VERSION 2)
set(HALFASSED_PATCH_VERSION 1)
set(HALFASSED_VERSION
	${HALFASSED_MAJOR_VERSION}.${HALFASSED_MINOR_VERSION}.${HALFASSED_PATCH_VERSION}
)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()



if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (NOT (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9))
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.9 or greater.")
    endif ()
else ()
    message(FATAL_ERROR "Your C++ compiler does not support C++11")
endif ()


# Add sub-directories
add_subdirectory(main)
