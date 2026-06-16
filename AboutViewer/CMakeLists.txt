find_program(DOTNET_EXE NAMES dotnet)
if(NOT DOTNET_EXE)
  message(WARNING "dotnet CLI not found — AboutViewer will not be built.")
  return()
endif()

# AboutViewer has no ProjectReferences so it needs no ordering constraint.
add_custom_target(aboutviewer_build
  COMMAND ${DOTNET_EXE} build ${CMAKE_CURRENT_SOURCE_DIR}
          -c $<CONFIG>
          -o ${CMAKE_BINARY_DIR}/AboutViewer/bin/$<CONFIG>
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Building AboutViewer (.NET 10 / WPF)"
  VERBATIM
)
