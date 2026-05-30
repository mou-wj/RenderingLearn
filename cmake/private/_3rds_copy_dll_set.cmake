# 调用方式：
#   cmake -Ddll_list_var=XXX_DLLS -Ddst_dir=... -P _3rds_copy_dll_set.cmake
# 直接拷贝 DLL 集合变量到目标目录
foreach(dll_path ${dll_list_var})
  if(EXISTS "${dll_path}")
    file(COPY "${dll_path}" DESTINATION "${dst_dir}")
    message(STATUS "Copied DLL: ${dll_path} -> ${dst_dir}")
  else()
    message(WARNING "DLL not found: ${dll_path}")
  endif()
endforeach()
