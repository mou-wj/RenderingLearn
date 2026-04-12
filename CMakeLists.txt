if(NOT WIN32)
	#current surpported platform is windows
	return()
endif()


cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(TargetName RenderingLearn)
project(${TargetName})
include(${CMAKE_SOURCE_DIR}/cmake/Find3rdsGenerated.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Functions.cmake)
set(BINARY_DIR "${CMAKE_BINARY_DIR}")
message("out: ${BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BINARY_DIR}")
message("INSTALLDIR: ${INSTALL_DIR}/${CONFIG}")


add_subdirectory(src)

#renderdoc tool
set(Renderdoc_INSTALL_DIR "D:/RenderDoc")
if(NOT EXISTS ${Renderdoc_INSTALL_DIR})
	message("render not found in ${Renderdoc_INSTALL_DIR}")
else()
endif()
