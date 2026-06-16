cmake_minimum_required(VERSION 3.22)
project(CellForgeStudio)

# Order matters: WpfControls and WpfBridge are defined before Studio
# so Studio can reference their targets with add_dependencies().
add_subdirectory(WpfControls)
add_subdirectory(WpfBridge)
add_subdirectory(Studio)
