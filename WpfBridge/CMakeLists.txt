find_program(MSBUILD_EXE
  NAMES msbuild.exe
  HINTS
    "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Community/MSBuild/Current/Bin"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/17/Community/MSBuild/Current/Bin"
)

if(NOT MSBUILD_EXE)
  message(WARNING "MSBuild not found — WpfBridge will not be built.")
  return()
endif()

add_custom_target(wpfbridge_build
  COMMAND ${MSBUILD_EXE}
            ${CMAKE_CURRENT_SOURCE_DIR}/WpfBridge.vcxproj
            /p:Configuration=$<CONFIG>
            /p:Platform=x64
            /p:OutDir=${CMAKE_BINARY_DIR}/WpfBridge/bin/$<CONFIG>/
            /m
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Building WpfBridge (C++/CLI / .NET 10)"
  VERBATIM
)
