cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(TargetName RenderingLearn)
project(${TargetName})
set(Dir3rd ./3rdParty)

set(IncludeDir3rd "${Dir3rd}/include")
set(LibDir3rd "${Dir3rd}/lib")
set(BinDir3rd "${Dir3rd}/bin")
set(Libs3rd 
	glew32.lib
	glfw3.lib
	assimp-vc143-mtd.lib

)
#find_package(Glslang REQUIRED)
find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
    list(APPEND Libs3rd ${Vulkan_LIBRARIES})
	list(APPEND IncludeDir3rd "${Vulkan_INCLUDE_DIRS}")
	message("${Vulkan_INCLUDE_DIRS} xxxx")
#	message("${IncludeDir3rd} xxxx")
#	message("${Libs3rd} xxxx")
	message("${Vulkan_LIBRARIES} xxxx")
	get_filename_component(VulkanLibDir ${Vulkan_LIBRARIES} DIRECTORY)
	message("${VulkanLibDir} xxx")
	list(APPEND LibDir3rd "${VulkanLibDir}")

	#file(GLOB VulkanSpirvCrossLibs "${VulkanLibDir}/spirv-cross-*d.lib")

	#if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	#	list(FILTER VulkanSpirvCrossLibs INCLUDE REGEX .*spirv-cross-.*d.dll$)
	#else() 
	#	list(FILTER VulkanSpirvCrossLibs EXCLUDE REGEX .*spirv-cross-.*d.dll$)
	#endif()
	#message(FATAL_ERROR "${VulkanSpirvCrossLibs}")
	#list(APPEND Libs3rd ${VulkanSpirvCrossLibs})

else()
	message("dsadsad")
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
	#list(APPEND LibDir3rd ${Glslang_INSTALL_DIR}/lib)
	file(GLOB GlslangLibs ${Glslang_INSTALL_DIR}/${BuildType}/lib/*.lib)
	list(APPEND Libs3rd ${GlslangLibs})
	#message(FATAL_ERROR "glslng include ${Glslang_INSTALL_DIR}/${BuildType}/include/glslang")
	#message(FATAL_ERROR "glslng libs ${GlslangLibs}")


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
	#list(APPEND LibDir3rd ${Glslang_INSTALL_DIR}/lib)
	file(GLOB SprivCrossLibs ${SpirvCross_INSTALL_DIR}/${BuildType}/lib/*.lib)
	list(APPEND Libs3rd ${SprivCrossLibs})
	#message(FATAL_ERROR "sprivcross libs ${SprivCrossLibs}")


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
	#list(APPEND ${LibDir3rd} ${SpirvTools_INSTALL_DIR}/${BuildType}/lib )
	#file(GLOB SprivToolsLibs ${SpirvTools_INSTALL_DIR}/${BuildType}/lib/*.lib)
	set(SprivToolsLibDir ${SpirvTools_INSTALL_DIR}/${BuildType}/lib)
	list(APPEND Libs3rd ${SprivToolsLibs} ${SprivToolsLibDir}/SPIRV-Tools.lib ${SprivToolsLibDir}/SPIRV-Tools-opt.lib)
	#message(FATAL_ERROR "sprivcross libs ${SprivCrossLibs}")


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

target_include_directories(${TargetName} PUBLIC D:/RenderDoc)
#target_link_libraries(${TargetName} PUBLIC  D:/RenderDoc/renderdoc.dll)

# 设置运行时库路径，确保运行时可以找到RenderDoc的DLL
if (WIN32)
    set_target_properties(${TargetName} PROPERTIES
                          VS_DEBUGGER_COMMAND_ARGUMENTS "\"${CMAKE_CURRENT_SOURCE_DIR}\""  # 设置RenderDoc的DLL路径
                          VS_DEBUGGER_ENVIRONMENT "PATH=D:/RenderDoc\\$<CONFIG>")
endif()

#install
#message("${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/")
#install(FILES "${BinDir3rd}/glew32.dll" DESTINATION "${COMPILE_PDB_OUTPUT_DIRECTORY}")


#脚本命令
#find_program(Python_EXECUTABLE python NO_CMAKE_FIND_ROOT_PATH)
#收集需要用脚本处理的头文件
#file(GLOB headFiles "test/*.h")
#设置需要运行py脚本文件
#set(PythonScript "xxx.py")
#execute_process(
#	COMMAND ${Python_EXECUTABLE} ${PythonScript} "-i" "${headFiles}" "-o" "xxx.h"	
#)