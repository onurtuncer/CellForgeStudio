find_program(DOTNET_EXE NAMES dotnet)
if(NOT DOTNET_EXE)
  message(WARNING "dotnet CLI not found — WpfControls will not be built.")
  return()
endif()

add_custom_target(wpfcontrols_build
  COMMAND ${DOTNET_EXE} build ${CMAKE_CURRENT_SOURCE_DIR} -c $<CONFIG> -p:Version=${CellForgeStudio_VERSION}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Building WpfControls (.NET 10 / WPF)"
  VERBATIM
)
