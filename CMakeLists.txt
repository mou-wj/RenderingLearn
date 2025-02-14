cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(TargetName RenderingLearn)
project(${TargetName})
set(Dir3rd ./3rdParty)

set(IncludeDir3rd "${Dir3rd}/include")
set(LibDir3rd "${Dir3rd}/lib")
set(BinDir3rd "${Dir3rd}/bin")
set(Libs3rd 
	glfw3.lib
)

find_package(Vulkan)
if(Vulkan_FOUND)
    list(APPEND Libs3rd ${Vulkan_LIBRARIES})
	list(APPEND IncludeDir3rd "${Vulkan_INCLUDE_DIRS}")
	get_filename_component(VulkanLibDir ${Vulkan_LIBRARIES} DIRECTORY)
	list(APPEND LibDir3rd "${VulkanLibDir}")

else()
	message(FATAL_ERROR "vulkan is not found")
endif()

#glslang 3rd
set(Glslang_INSTALL_DIR "D:/GlslangInstall")
if(NOT EXISTS ${Glslang_INSTALL_DIR})
	message(FATAL_ERROR "glslang not found in ${Glslang_INSTALL_DIR}")
else()
	
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(BuildType Debug)
	else() 
		set(BuildType Release)
	endif()
	list(APPEND IncludeDir3rd ${Glslang_INSTALL_DIR}/${BuildType}/include/glslang)
	file(GLOB GlslangLibs ${Glslang_INSTALL_DIR}/${BuildType}/lib/*.lib)
	list(APPEND Libs3rd ${GlslangLibs})



endif()

#spirvcross 3rd
set(SpirvCross_INSTALL_DIR "D:/SpirvCrossInstall")
if(NOT EXISTS ${SpirvCross_INSTALL_DIR})
	message(FATAL_ERROR "spriv_cross not found in ${SpirvCross_INSTALL_DIR}")
else()
	
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(BuildType x64-Debug)
	else() 
		set(BuildType x64-Release)
	endif()
	list(APPEND IncludeDir3rd ${SpirvCross_INSTALL_DIR}/${BuildType}/include/spirv_cross)
	file(GLOB SprivCrossLibs ${SpirvCross_INSTALL_DIR}/${BuildType}/lib/*.lib)
	list(APPEND Libs3rd ${SprivCrossLibs})



endif()

#spirvtools 3rd
set(SpirvTools_INSTALL_DIR "D:/SpirvToolsInstall")
if(NOT EXISTS ${SpirvTools_INSTALL_DIR})
	message(FATAL_ERROR "spriv_tools not found in ${SpirvTools_INSTALL_DIR}")
else()
	
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(BuildType x64-Debug)
	else() 
		set(BuildType x64-Release)
	endif()
	list(APPEND IncludeDir3rd ${SpirvTools_INSTALL_DIR}/${BuildType}/include/spirv_tools)
	list(APPEND LibDir3rd ${Glslang_INSTALL_DIR}/lib)

	set(SprivToolsLibDir ${SpirvTools_INSTALL_DIR}/${BuildType}/lib)
	list(APPEND Libs3rd ${SprivToolsLibs} ${SprivToolsLibDir}/SPIRV-Tools.lib ${SprivToolsLibDir}/SPIRV-Tools-opt.lib)


endif()


file(GLOB_RECURSE SrcFiles ./src/*.cpp ./src/*.h)
#message(FATAL_ERROR "${SrcFiles}")

add_executable(${TargetName} ${SrcFiles})
target_include_directories(${TargetName} PUBLIC ${IncludeDir3rd})
target_compile_definitions(${TargetName} PRIVATE VULKAN_INCLUDE_DIRS="${Vulkan_INCLUDE_DIRS}" PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

target_link_directories(${TargetName} PUBLIC ${LibDir3rd})
target_compile_definitions(${TargetName} PUBLIC -DPROJECT_NAME=${TargetName})
message("3rd lib path : ${LibDir3rd}")
message("3rd libs : ${Libs3rd}")

target_link_libraries(${TargetName} PUBLIC ${Libs3rd})


# 添加RenderDoc库到链接中
#spirvtools 3rd
set(Renderdoc_INSTALL_DIR "D:/RenderDoc")
if(NOT EXISTS ${Renderdoc_INSTALL_DIR})
	message(FATAL_ERROR "render not found in ${Renderdoc_INSTALL_DIR}")
else()

	target_include_directories(${TargetName} PUBLIC ${Renderdoc_INSTALL_DIR})
	#target_link_libraries(${TargetName} PUBLIC  D:/RenderDoc/renderdoc.dll)
	
	# 设置运行时库路径，确保运行时可以找到RenderDoc的DLL
	if (WIN32)
	    set_target_properties(${TargetName} PROPERTIES
	                          VS_DEBUGGER_COMMAND_ARGUMENTS "\"${CMAKE_CURRENT_SOURCE_DIR}\""  # 设置RenderDoc的DLL路径
	                          VS_DEBUGGER_ENVIRONMENT "PATH=${Renderdoc_INSTALL_DIR}\\$<CONFIG>")
	endif()



endif()

