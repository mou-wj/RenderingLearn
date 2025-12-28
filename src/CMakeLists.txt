add_subdirectory(Framework)
add_subdirectory(Application)

# Collect possible main sources (main.cpp or files under main/)
file(GLOB APP_MAIN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
add_executable(Langcher ${APP_MAIN_FILES})
target_link_libraries(Langcher PRIVATE Engine)



