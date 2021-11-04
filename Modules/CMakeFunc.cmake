

#添加头文件
function(add_filtered_h filterName fileString )
	file(GLOB TMP_FILES ${fileString})
	set(SOURCE_FILES1 ${SOURCE_FILES1} ${TMP_FILES} PARENT_SCOPE)
	source_group(${filterName} FILES ${TMP_FILES})
	
endfunction( add_filtered_h )

#添加源代码
function(add_filtered_src filterName fileString )
	file(GLOB TMP_FILES ${fileString})
	set(SOURCE_FILES1 ${SOURCE_FILES1} ${TMP_FILES} PARENT_SCOPE)
	source_group(${filterName} FILES ${TMP_FILES})
endfunction( add_filtered_src )




#添加过滤文件夹
function(add_filtered_std relativePath )
	string(REPLACE "/" "\\" filterPart ${relativePath})
	set(MODULE_INCLUDE_DIRS  ${MODULE_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}" PARENT_SCOPE)
	add_filtered_h("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.h")
	add_filtered_h("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.hpp")
	add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.cpp")
	add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.c")
	add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.cc")
	# add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.asm")
	# add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.natvis")
	
	if(APPLE_IOS)
		add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.mm")
	endif()
	set(SOURCE_FILES1 ${SOURCE_FILES1} PARENT_SCOPE)
	
	
	
endfunction( add_filtered_std )






#添加过滤文件夹
function(add_filtered_std_no_include relativePath )
	string(REPLACE "/" "\\" filterPart ${relativePath})
	# include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}")
	add_filtered_h("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.h")
	add_filtered_h("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.hpp")
	add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.cpp")
	add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.c")
	add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.cc")
	# add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.asm")
	# add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.natvis")
	if(APPLE_IOS)
		add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.mm")
	endif()
	set(SOURCE_FILES1 ${SOURCE_FILES1} PARENT_SCOPE)
endfunction( add_filtered_std_no_include )


#添加过滤文件夹
function(add_filtered_std_no_include_iter relativePath )
	string(REPLACE "/" "\\" filterPart ${relativePath})
	
	#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}")
	add_filtered_h("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.h")
	add_filtered_h("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.hpp")
	add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.cpp")
	add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.c")
	add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.cc")
	
	# add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.asm")
	# add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.natvis")
	if(APPLE_IOS)
		add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.mm")
	endif()
	file(GLOB TMP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*")
	
	foreach(child ${TMP_FILES})
		if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${child}")
			
			add_filtered_std_no_include_iter(${child})
			
		endif()
	endforeach()
	set(SOURCE_FILES1 ${SOURCE_FILES1} PARENT_SCOPE)
endfunction( add_filtered_std_no_include_iter )

#添加过滤文件夹
function(add_filtered_std_Iter)
	add_filtered_h("Source Files" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
	add_filtered_h("Source Files" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
	add_filtered_src("Source Files" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
	add_filtered_src("Source Files" "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
	add_filtered_src("Source Files" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
	# add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.asm")
	# add_filtered_src("Source Files\\${filterPart}" "${CMAKE_CURRENT_SOURCE_DIR}/${relativePath}/*.natvis")

	
	#message("${CMAKE_CURRENT_SOURCE_DIR}")
	file(GLOB TMP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*")
	foreach(child ${TMP_FILES})
		
		if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${child}")
			add_filtered_std_no_include_iter(${child})
		endif()
	endforeach()
	set(SOURCE_FILES1 ${SOURCE_FILES1} PARENT_SCOPE)
	set(MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} PARENT_SCOPE)
endfunction( add_filtered_std_Iter )


#绝对路径的文件夹
function(add_filtered_root abPath )
	set(filterPart "")
	include_directories("${abPath}")
	set(MODULE_INCLUDE_DIRS  ${MODULE_INCLUDE_DIRS} "${abPath}" PARENT_SCOPE)
	add_filtered_h("Source Files\\${filterPart}" "${abPath}/*.h")
	add_filtered_h("Source Files\\${filterPart}" "${abPath}/*.hpp")
	add_filtered_src("Source Files\\${filterPart}" "${abPath}/*.cpp")
	add_filtered_src("Source Files\\${filterPart}" "${abPath}/*.c")
	add_filtered_src("Source Files\\${filterPart}" "${abPath}/*.cc")
	if(APPLE_IOS)
		add_filtered_src("Source Files\\${filterPart}" "${abPath}/*.mm")
	endif()
	set(SOURCE_FILES1 ${SOURCE_FILES1} PARENT_SCOPE)
endfunction(add_filtered_root )

#添加一个文件
function(add_filtered_file fullFileName)
	STRING(FIND ${fullFileName} "/" str_pos REVERSE)
	STRING(SUBSTRING ${fullFileName} 0 ${str_pos} relativePath)
	string(REPLACE "/" "\\" filterPart ${relativePath})
	set(TMP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${fullFileName}")
	set(SOURCE_FILES1 ${SOURCE_FILES1} ${TMP_FILES} PARENT_SCOPE)
	source_group("Source Files\\${filterPart}" FILES ${TMP_FILES})
endfunction( add_filtered_file )


function(add_filter_only filterName fileString )
	file(GLOB TMP_FILES ${fileString})
	source_group(${filterName} FILES ${TMP_FILES})
endfunction( add_filter_only )


function(add_global_define defineStr)
	add_definitions(-D${defineStr})
	set(LIB_GLOBAL_DEFINES ${LIB_GLOBAL_DEFINES} )
endfunction(add_global_define)


function(add_find_library target_Name lib_name)
	unset(FOUND_LIB CACHE)
	find_library(FOUND_LIB ${lib_name})
	if(FOUND_LIB)
		#message(STATUS "add_find_library:"${FOUND_LIB}",libName:"${lib_name})
		target_link_libraries(${target_name} ${FOUND_LIB})
	else()
		message("nout found  lib:"${lib_name})
	endif()
endfunction( add_find_library )

function(add_direct_projects path Filter)
	file(GLOB TMP_FILES RELATIVE ${path} "${path}/*")
	Set(FilterPath ${Filter})
	foreach(item ${TMP_FILES})
		if(IS_DIRECTORY "${path}/${item}")
			SET(LIB_NAME ${item})
			string(TOUPPER ${item} UperName)
			ADD_SUBDIRECTORY(${item})
			
		endif()
	endforeach()
	SET(MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} PARENT_SCOPE)
endfunction()


#模块化函数



# function(begin_module_path modulePath)
# 	SET(CACHE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
# 	# SET(CUR_MODULE_NAME ${modulePath} PARENT_SCOPE)
# 	SET(TEMP_NAME ${modulePath})
# 	string(REPLACE "/" "\\" filterPart ${TEMP_NAME})
# 	SET(CUR_MODULE_NAME ${TEMP_NAME} PARENT_SCOPE)

# 	SET(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${modulePath} PARENT_SCOPE)
# endfunction(begin_module_path)

# function(add_module_filtered_std fullFileName)
# 	STRING(FIND ${fullFileName} "/" str_pos REVERSE)
# 	STRING(SUBSTRING ${fullFileName} 0 ${str_pos} relativePath)
# 	string(REPLACE "/" "\\" filterPart ${relativePath})
# 	set(TMP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${fullFileName}")
# 	source_group("${CUR_MODULE_NAME}\\${filterPart}" FILES ${TMP_FILES})
# 	set(SOURCE_FILES ${SOURCE_FILES} ${TMP_FILES} PARENT_SCOPE)
# endfunction(add_module_filtered_std)

# function(add_module_filtered_root abPath )
# 	set(filterPart "")
# 	include_directories("${abPath}")
# 	set(MODULE_INCLUDE_DIRS  ${MODULE_INCLUDE_DIRS} "${abPath}" PARENT_SCOPE)
# 	add_filtered_h("${CUR_MODULE_NAME}\\${filterPart}" "${abPath}/*.h")
# 	add_filtered_h("${CUR_MODULE_NAME}\\${filterPart}" "${abPath}/*.hpp")
# 	add_filtered_src("${CUR_MODULE_NAME}\\${filterPart}" "${abPath}/*.cpp")
# 	add_filtered_src("${CUR_MODULE_NAME}\\${filterPart}" "${abPath}/*.c")
# 	add_filtered_src("${CUR_MODULE_NAME}\\${filterPart}" "${abPath}/*.cc")
# 	if(APPLE_IOS)
# 		add_filtered_src("${CUR_MODULE_NAME}\\${filterPart}" "${abPath}/*.mm")
# 	endif()
# 	set(SOURCE_FILES ${SOURCE_FILES} PARENT_SCOPE)
# endfunction(add_module_filtered_root )

# function(end_module_path)
# 	SET(CMAKE_CURRENT_SOURCE_DIR ${CACHE_MODULE_PATH} PARENT_SCOPE)
# endfunction(end_module_path)

# function(export_module_include export_dirs)
# 	set(MODULE_INCLUDE_DIRS  ${MODULE_INCLUDE_DIRS} "${export_dirs}" PARENT_SCOPE)
# endfunction(export_module_include)

# function(export_moudle_files export_files)
# 	SET(MODULE_SOURCE_FILES  ${MODULE_SOURCE_FILES} ${export_files} PARENT_SCOPE)
# endfunction(export_moudle_files)
#模块化函数