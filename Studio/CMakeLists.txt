cmake_minimum_required(VERSION 3.22)
project(Studio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# ── Studio executable ─────────────────────────────────────────────────────────

add_executable(Studio WIN32 src/main.cpp)

target_compile_definitions(Studio PRIVATE UNICODE _UNICODE)
target_include_directories(Studio PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Depend on managed projects and copy their DLLs alongside the executable.
# Both targets are defined in sibling subdirectories processed before this one.
if(TARGET wpfcontrols_build)
  add_dependencies(Studio wpfcontrols_build)
  add_custom_command(TARGET Studio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_SOURCE_DIR}/WpfControls/bin/$<CONFIG>/net10.0-windows/WpfControls.dll
      $<TARGET_FILE_DIR:Studio>/WpfControls.dll
    COMMENT "Copying WpfControls.dll"
    VERBATIM
  )
endif()

# Copy help content next to the executable
add_custom_command(TARGET Studio POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/help
    $<TARGET_FILE_DIR:Studio>/help
  COMMENT "Copying help content"
  VERBATIM
)

if(TARGET wpfbridge_build)
  add_dependencies(Studio wpfbridge_build)
  # WpfBridge.dll + its .NET CLR runtime files.
  # Ijwhost.dll is the IJW bridge that bootstraps the .NET runtime when
  # LoadLibraryW loads WpfBridge.dll; without it LoadLibrary returns NULL.
  # runtimeconfig.json tells the host which .NET / WindowsDesktop version to load.
  # deps.json provides the framework dependency graph.
  foreach(_f WpfBridge.dll Ijwhost.dll WpfBridge.runtimeconfig.json WpfBridge.deps.json)
    add_custom_command(TARGET Studio POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/WpfBridge/bin/$<CONFIG>/${_f}
        $<TARGET_FILE_DIR:Studio>/${_f}
      VERBATIM
    )
  endforeach()
endif()

if(TARGET helpviewer_build)
  add_dependencies(Studio helpviewer_build)
  # Copy all HelpViewer build outputs (exe + WpfControls.dll + WebView2 DLLs)
  # into a HelpViewer/ subdirectory next to Studio.exe.
  # Studio.exe launches HelpViewer/HelpViewer.exe as a separate process.
  add_custom_command(TARGET Studio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_BINARY_DIR}/HelpViewer/bin/$<CONFIG>/
      $<TARGET_FILE_DIR:Studio>/HelpViewer/
    COMMENT "Copying HelpViewer"
    VERBATIM
  )
  # HelpControl resolves help/ relative to WpfControls.dll, which lives inside
  # HelpViewer/.  Copy the help source tree there.
  add_custom_command(TARGET Studio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/help/
      $<TARGET_FILE_DIR:Studio>/HelpViewer/help/
    COMMENT "Copying help content into HelpViewer/"
    VERBATIM
  )
endif()

if(TARGET aboutviewer_build)
  add_dependencies(Studio aboutviewer_build)
  add_custom_command(TARGET Studio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_BINARY_DIR}/AboutViewer/bin/$<CONFIG>/
      $<TARGET_FILE_DIR:Studio>/AboutViewer/
    COMMENT "Copying AboutViewer"
    VERBATIM
  )
endif()

# ── about_test executable ─────────────────────────────────────────────────────

add_executable(about_test WIN32 src/test_about.cpp)

target_compile_definitions(about_test PRIVATE UNICODE _UNICODE)

if(TARGET wpfcontrols_build)
  add_dependencies(about_test wpfcontrols_build)
  add_custom_command(TARGET about_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_SOURCE_DIR}/WpfControls/bin/$<CONFIG>/net10.0-windows/WpfControls.dll
      $<TARGET_FILE_DIR:about_test>/WpfControls.dll
    VERBATIM
  )
endif()

if(TARGET wpfbridge_build)
  add_dependencies(about_test wpfbridge_build)
  foreach(_f WpfBridge.dll Ijwhost.dll WpfBridge.runtimeconfig.json WpfBridge.deps.json)
    add_custom_command(TARGET about_test POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/WpfBridge/bin/$<CONFIG>/${_f}
        $<TARGET_FILE_DIR:about_test>/${_f}
      VERBATIM
    )
  endforeach()
endif()
