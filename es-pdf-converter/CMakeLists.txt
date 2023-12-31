#  SPDX-License-Identifier: MIT
#
#  EmulationStation Desktop Edition
#  CMakeLists.txt (es-pdf-converter)
#
#  CMake configuration for es-pdf-convert
#

project(es-pdf-convert)

if(WIN32)
    set(POPPLER_CPP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/poppler/Library/include/poppler/cpp)
    if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        set(POPPLER_CPP_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/poppler-cpp.lib)
        # Disable DLL interface warnings.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251")
    else()
        set(POPPLER_CPP_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libpoppler-cpp-0.dll)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -municode")
    endif()
elseif(APPLE)
    set(POPPLER_CPP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/poppler/cpp ${CMAKE_CURRENT_SOURCE_DIR}/../external/poppler/build/cpp)
    set(POPPLER_CPP_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/../libpoppler-cpp.0.dylib)
else()
    find_package(Poppler REQUIRED COMPONENTS cpp)
endif()

include_directories(${POPPLER_CPP_INCLUDE_DIR})
add_executable(es-pdf-convert WIN32 ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(es-pdf-convert ${POPPLER_CPP_LIBRARY})

if(WIN32)
    set_target_properties(es-pdf-convert PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/es-pdf-converter" INSTALL_RPATH_USE_LINK_PATH TRUE)
else()
    set_target_properties(es-pdf-convert PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()
