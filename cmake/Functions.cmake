#debug log error
function(log_error msg)
  message(FATAL_ERROR "${msg}")
endfunction()

# ---- runtime dll copy helper ----
function(_3rds_copy_runtime_deps consumer dst_dir)
  if(NOT TARGET ${consumer})
    message(FATAL_ERROR "Target '${consumer}' does not exist")
  endif()

  add_custom_command(TARGET ${consumer}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime DLLs for target: ${consumer}"
    COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_RUNTIME_DLLS:${consumer}>"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_dir}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:${consumer}>
      "${dst_dir}"
    COMMAND_EXPAND_LISTS
  )
endfunction()


# ---- unified dll postbuild copy helper ----
# 用法: _3rds_postbuild_copy_dlls(<consumer> <dll_list_var|auto> [<dst_dir>])
# <dll_list_var|auto>: 变量名（如 RENDERDOC_DLLS_RELEASE）或 auto（用$<TARGET_RUNTIME_DLLS:consumer>）
# <dst_dir> 可选，默认 $<TARGET_FILE_DIR:consumer>
function(_3rds_postbuild_copy_dlls consumer dlls dst_dir)
    add_custom_command(TARGET ${consumer}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime DLLs for target: ${consumer}"
      COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime DLLs: ${dlls}"
      COMMAND ${CMAKE_COMMAND} -Ddll_list_var=${dlls} -Ddst_dir="${dst_dir}" -P "${CMAKE_SOURCE_DIR}/cmake/private/_3rds_copy_dll_set.cmake"
    )
endfunction()

