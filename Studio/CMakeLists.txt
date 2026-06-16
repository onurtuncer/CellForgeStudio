cmake_minimum_required(VERSION 3.16)
project(Studio VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
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

# -- Integrate C# "widgets" project into the CMake build
# Find dotnet CLI
find_program(DOTNET_EXE NAMES dotnet)
if(NOT DOTNET_EXE)
  message(WARNING "dotnet CLI not found. Widgets project will not be built as part of the CMake build.")
else()
  # Custom target to build the widgets C# project
  add_custom_target(widgets_build
	COMMAND ${DOTNET_EXE} build ${CMAKE_SOURCE_DIR}/widgets -c $<CONFIG> --no-restore
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/widgets
	COMMENT "Building C# widgets project"
	VERBATIM
  )

  # Ensure the C++ Studio target builds after the C# project
  add_dependencies(Studio widgets_build)

  # Copy the built widgets executable next to the Studio executable after build
  add_custom_command(TARGET Studio POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	  ${CMAKE_SOURCE_DIR}/widgets/bin/$<CONFIG>/net7.0/widgets.exe
	  $<TARGET_FILE_DIR:Studio>/widgets.exe
	COMMENT "Copying widgets.exe next to Studio executable"
  )
endif()
