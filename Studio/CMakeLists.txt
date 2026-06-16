cmake_minimum_required(VERSION 3.22)
project(Studio VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionally control build types
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()


# Build as a Windows GUI application when possible (suitable for MFC)
add_executable(Studio WIN32
	src/main.cpp
)

if(MSVC)
  # Use MFC dynamically in Visual Studio
  set_target_properties(Studio PROPERTIES VS_MFC "Dynamic")
endif()

target_include_directories(Studio PRIVATE ${PROJECT_SOURCE_DIR}/include)

## -- Integrate WPF C# class library (WpfControls) into the CMake build
find_program(DOTNET_EXE NAMES dotnet)
if(NOT DOTNET_EXE)
  message(WARNING "dotnet CLI not found. WpfControls project will not be built as part of the CMake build.")
else()
  # Custom target to build the WpfControls C# project (class library)
  add_custom_target(wpfcontrols_build
	COMMAND ${DOTNET_EXE} build ${CMAKE_SOURCE_DIR}/WpfControls -c $<CONFIG> --no-restore
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/WpfControls
	COMMENT "Building WpfControls C# project"
	VERBATIM
  )

  # Ensure the C++ Studio target builds after the C# project
  add_dependencies(Studio wpfcontrols_build)

  # Copy the built WpfControls DLL next to the Studio executable after build
  add_custom_command(TARGET Studio POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	  ${CMAKE_SOURCE_DIR}/WpfControls/bin/$<CONFIG>/net7.0/WpfControls.dll
	  $<TARGET_FILE_DIR:Studio>/WpfControls.dll
	COMMENT "Copying WpfControls.dll next to Studio executable"
  )
endif()


## -- Integrate WpfBridge C++/CLI project via MSBuild when using MSVC
find_program(MSBUILD_EXE NAMES msbuild.exe)
if(NOT MSBUILD_EXE)
  message(WARNING "MSBuild not found. WpfBridge project will not be built as part of the CMake build.")
else()
  # Build WpfBridge.vcxproj and set OutDir to a known location under the CMake binary dir
  add_custom_target(wpfbridge_build
	COMMAND ${MSBUILD_EXE} ${CMAKE_SOURCE_DIR}/WpfBridge/WpfBridge.vcxproj /p:Configuration=$<CONFIG> /p:Platform=x64 /p:OutDir=${CMAKE_BINARY_DIR}/WpfBridge/bin/$<CONFIG>/ /m
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/WpfBridge
	COMMENT "Building WpfBridge C++/CLI project"
	VERBATIM
  )

  add_dependencies(Studio wpfbridge_build)

  # Copy resulting DLL next to Studio executable
  add_custom_command(TARGET Studio POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	  ${CMAKE_BINARY_DIR}/WpfBridge/bin/$<CONFIG>/WpfBridge.dll
	  $<TARGET_FILE_DIR:Studio>/WpfBridge.dll
	COMMENT "Copying WpfBridge.dll next to Studio executable"
  )
endif()


# Small native test that creates a window and calls ShowWpfAboutDialog
add_executable(about_test WIN32
  src/test_about.cpp
)

add_dependencies(about_test wpfcontrols_build wpfbridge_build)

add_custom_command(TARGET about_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_SOURCE_DIR}/WpfControls/bin/$<CONFIG>/net7.0/WpfControls.dll
	$<TARGET_FILE_DIR:about_test>/WpfControls.dll
  COMMENT "Copying WpfControls.dll next to about_test executable"
)

add_custom_command(TARGET about_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_BINARY_DIR}/WpfBridge/bin/$<CONFIG>/WpfBridge.dll
	$<TARGET_FILE_DIR:about_test>/WpfBridge.dll
  COMMENT "Copying WpfBridge.dll next to about_test executable"
)
