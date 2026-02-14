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