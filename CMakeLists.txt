if(NOT WIN32)
	#current surpported platform is windows
	return()
endif()


cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(TargetName RenderingLearn)
project(${TargetName})

set(IncludeDir3rd)
set(Libs3rd)


find_package(Vulkan)
if(Vulkan_FOUND)
    list(APPEND Libs3rd ${Vulkan_LIBRARIES})
	list(APPEND IncludeDir3rd "${Vulkan_INCLUDE_DIRS}")
	
else()
	message(FATAL_ERROR "vulkan is not found")
endif()

	
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	set(BuildType Debug)
else() 
	set(BuildType Release)
endif()


#glfw 3rd
set(GLFW_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/install/glfw")
if(NOT EXISTS ${GLFW_INSTALL_DIR})
	message(FATAL_ERROR "glfw not found in ${GLFW_INSTALL_DIR}")
else()
	
	list(APPEND IncludeDir3rd ${GLFW_INSTALL_DIR}/${BuildType}/include)
	file(GLOB GLFWLibs ${GLFW_INSTALL_DIR}/${BuildType}/lib/*.lib)
	list(APPEND Libs3rd ${GLFWLibs})


endif()


#glslang 3rd
set(Glslang_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/install/glslang")
if(NOT EXISTS ${Glslang_INSTALL_DIR})
	message(FATAL_ERROR "glslang not found in ${Glslang_INSTALL_DIR}")
else()

	list(APPEND IncludeDir3rd ${Glslang_INSTALL_DIR}/${BuildType}/include)
	list(APPEND IncludeDir3rd ${Glslang_INSTALL_DIR}/${BuildType}/include/glslang/Public)
	file(GLOB GlslangLibs ${Glslang_INSTALL_DIR}/${BuildType}/lib/*.lib)
	list(APPEND Libs3rd ${GlslangLibs})



endif()

#spirvcross 3rd
set(SpirvCross_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/install/SPIRV-Cross")
if(NOT EXISTS ${SpirvCross_INSTALL_DIR})
	message(FATAL_ERROR "spriv_cross not found in ${SpirvCross_INSTALL_DIR}")
else()

	list(APPEND IncludeDir3rd ${SpirvCross_INSTALL_DIR}/${BuildType}/include/spirv_cross)
	file(GLOB SprivCrossLibs ${SpirvCross_INSTALL_DIR}/${BuildType}/lib/*.lib)
	list(APPEND Libs3rd ${SprivCrossLibs})



endif()

#spirvtools 3rd
set(SpirvTools_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/install/SPIRV-Tools")
if(NOT EXISTS ${SpirvTools_INSTALL_DIR})
	message(FATAL_ERROR "spriv_tools not found in ${SpirvTools_INSTALL_DIR}")
else()
	
	list(APPEND IncludeDir3rd ${SpirvTools_INSTALL_DIR}/${BuildType}/include/spirv_tools)
	set(SprivToolsLibDir ${SpirvTools_INSTALL_DIR}/${BuildType}/lib)
	list(APPEND Libs3rd ${SprivToolsLibs} ${SprivToolsLibDir}/SPIRV-Tools.lib ${SprivToolsLibDir}/SPIRV-Tools-opt.lib)
endif()


#tiny gltf
set(TinyGLTF_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/install/tinygltf")
if(NOT EXISTS ${TinyGLTF_INSTALL_DIR})
	message(FATAL_ERROR "tiny gltf not found in ${TinyGLTF_INSTALL_DIR}")
else()
	
	list(APPEND IncludeDir3rd ${TinyGLTF_INSTALL_DIR}/${BuildType}/include)
	file(GLOB TinyGLTFLibs ${TinyGLTF_INSTALL_DIR}/${BuildType}/lib/*.lib)
	list(APPEND Libs3rd ${TinyGLTFLibs})
endif()

#GL
find_package(OpenGL REQUIRED)

list(APPEND Libs3rd ${OPENGL_LIBRARIES})


file(GLOB_RECURSE SrcFiles ./src/*.cpp ./src/*.h)
#message(FATAL_ERROR "${SrcFiles}")

add_executable(${TargetName} ${SrcFiles})
target_include_directories(${TargetName} PUBLIC ${IncludeDir3rd})
target_compile_definitions(${TargetName} PUBLIC -DPROJECT_NAME=${TargetName} PRIVATE VULKAN_INCLUDE_DIRS="${Vulkan_INCLUDE_DIRS}" PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

message("3rd libs : ${Libs3rd}")

target_link_libraries(${TargetName} PUBLIC ${Libs3rd})


#renderdoc tool
set(Renderdoc_INSTALL_DIR "D:/RenderDoc")
if(NOT EXISTS ${Renderdoc_INSTALL_DIR})
	message("render not found in ${Renderdoc_INSTALL_DIR}")
else()
	target_compile_definitions(${TargetName} PUBLIC -DENABLE_RENDERDOC_CAPTURE -DRenderdoc_INSTALL_DIR="${Renderdoc_INSTALL_DIR}")
	target_include_directories(${TargetName} PUBLIC ${Renderdoc_INSTALL_DIR})
	# setup renderdoc runtime path
    set_target_properties(${TargetName} PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${Renderdoc_INSTALL_DIR}"
    )


endif()
