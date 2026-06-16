find_program(DOTNET_EXE NAMES dotnet)
if(NOT DOTNET_EXE)
  message(WARNING "dotnet CLI not found — HelpViewer will not be built.")
  return()
endif()

add_custom_target(helpviewer_build
  # add_dependencies(helpviewer_build wpfcontrols_build) below ensures WpfControls
  # is fully written before this runs, so dotnet sees it as up-to-date and skips
  # the rebuild — avoiding the file-lock that occurs during a parallel build.
  COMMAND ${DOTNET_EXE} build ${CMAKE_CURRENT_SOURCE_DIR}
          -c $<CONFIG>
          -o ${CMAKE_BINARY_DIR}/HelpViewer/bin/$<CONFIG>
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Building HelpViewer (.NET 10 / WPF)"
  VERBATIM
)

# Ensure WpfControls is fully written before HelpViewer links against it.
if(TARGET wpfcontrols_build)
  add_dependencies(helpviewer_build wpfcontrols_build)
endif()
