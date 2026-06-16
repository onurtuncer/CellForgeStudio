#[=======================================================================[.rst:
FindCellForge
-------------

Module-mode finder for a CellForge SDK installed via the Windows MSI
(WiX/CPack) installer, e.g. into ``C:/Program Files/CellForge``.

CellForge also ships a CMake config-file package
(``lib/cmake/CellForge/CellForge-config.cmake``) generated at install time;
prefer ``find_package(CellForge CONFIG)`` when that file is reachable via
``CMAKE_PREFIX_PATH``. This module exists for consumers that only know the
MSI install root (or nothing at all) and need to *locate* that root and
verify individual components, e.g. when generating diagnostics or when the
config package is unavailable.

Each requested component is searched for independently — its public header,
its import library / static library, and (on Windows) its runtime DLL — so a
partial installation can be detected component by component.

Components
^^^^^^^^^^

  core
  workcell
  viewer
  platform_qt
  persistence
  project
  robot

Result Variables
^^^^^^^^^^^^^^^^^

``CellForge_FOUND``
  True if CellForge was found and all requested components were found.
``CellForge_ROOT``
  Root of the discovered CellForge installation
  (e.g. ``C:/Program Files/CellForge``).
``CellForge_INCLUDE_DIR``
  Top-level include directory (``<root>/include``); contains ``CellForge/``.
``CellForge_VERSION``
  Version string read from the installed ``CellForge/Version.h``, if found.

For each requested (or default) component ``<comp>``:

``CellForge_<comp>_FOUND``
  True if the component's header, library, and (on Windows) DLL were found.
``CellForge_<comp>_INCLUDE_DIR``
  Include directory containing the component's public header(s).
``CellForge_<comp>_LIBRARY``
  Path to the import library (``.lib``) or static/shared library.
``CellForge_<comp>_DLL``
  Path to the runtime DLL (Windows only).

Imported Targets
^^^^^^^^^^^^^^^^^

For each found component, this module defines ``CellForge::<comp>`` as an
``IMPORTED`` target (``SHARED`` on platforms with a DLL, otherwise
``UNKNOWN``/static), matching the targets exported by the umbrella config
package.

Hints
^^^^^

``CellForge_ROOT`` (cache/env variable)
  Explicit install root to search first, e.g.
  ``-DCellForge_ROOT="C:/Program Files/CellForge"``.

#]=======================================================================]

include(FindPackageHandleStandardArgs)

# ─── Map component -> on-disk library base name (cellforge_<comp>) ────────────
set(_CellForge_known_components
    core
    workcell
    viewer
    platform_qt
    persistence
    project
    robot)

if(NOT CellForge_FIND_COMPONENTS)
  set(CellForge_FIND_COMPONENTS ${_CellForge_known_components})
endif()

# ─── Per-component representative header, used to anchor the include search ───
set(_CellForge_header_core CellForge/Application.h)
set(_CellForge_header_workcell CellForge/Tags.h)
set(_CellForge_header_viewer CellForge/IViewerWidget.h)
set(_CellForge_header_platform_qt CellForge/qt/QtApplicationPlatform.h)
set(_CellForge_header_persistence CellForge/OcafPersistenceBackend.h)
set(_CellForge_header_project CellForge/Project.h)
set(_CellForge_header_robot CellForge/RobotModel.h)

# ─── Locate the installation root ──────────────────────────────────────────────
# WiX (CPACK_PACKAGE_INSTALL_DIRECTORY "CellForge") installs under "<Program Files>/CellForge" by default.
find_path(
  CellForge_ROOT
  NAMES include/CellForge/Application.h
  PATHS "$ENV{CellForge_ROOT}"
        "$ENV{ProgramFiles}/CellForge"
        "$ENV{ProgramFiles\(x86\)}/CellForge"
        "C:/Program Files/CellForge"
        "C:/Program Files (x86)/CellForge"
  PATH_SUFFIXES "" "."
  DOC "CellForge MSI installation root")

if(CellForge_ROOT)
  set(CellForge_INCLUDE_DIR "${CellForge_ROOT}/include")
endif()

# ─── Version (from the installed core/Version.h, if present) ──────────────────
if(CellForge_INCLUDE_DIR AND EXISTS "${CellForge_INCLUDE_DIR}/CellForge/Version.h")
  file(STRINGS "${CellForge_INCLUDE_DIR}/CellForge/Version.h" _CellForge_version_lines
       REGEX "#define +CF_VERSION_(MAJOR|MINOR|PATCH)")
  foreach(_line IN LISTS _CellForge_version_lines)
    if(_line MATCHES "CF_VERSION_MAJOR +([0-9]+)")
      set(_CellForge_ver_major "${CMAKE_MATCH_1}")
    elseif(_line MATCHES "CF_VERSION_MINOR +([0-9]+)")
      set(_CellForge_ver_minor "${CMAKE_MATCH_1}")
    elseif(_line MATCHES "CF_VERSION_PATCH +([0-9]+)")
      set(_CellForge_ver_patch "${CMAKE_MATCH_1}")
    endif()
  endforeach()
  if(DEFINED _CellForge_ver_major)
    set(CellForge_VERSION "${_CellForge_ver_major}.${_CellForge_ver_minor}.${_CellForge_ver_patch}")
  endif()
endif()

# ─── Per-component discovery ───────────────────────────────────────────────────
set(_CellForge_required_vars CellForge_INCLUDE_DIR)

foreach(_comp IN LISTS CellForge_FIND_COMPONENTS)
  if(NOT
     _comp
     IN_LIST
     _CellForge_known_components)
    message(WARNING "FindCellForge: unknown component '${_comp}' (known: ${_CellForge_known_components})")
    continue()
  endif()

  set(_lib_name "cellforge_${_comp}")
  set(_header "${_CellForge_header_${_comp}}")

  # Header (anchors the component's include directory; flat under <root>/include)
  find_path(
    CellForge_${_comp}_INCLUDE_DIR
    NAMES "${_header}"
    HINTS "${CellForge_INCLUDE_DIR}"
    DOC "Include directory for CellForge::${_comp}")

  # Import/static library
  find_library(
    CellForge_${_comp}_LIBRARY
    NAMES "${_lib_name}"
    HINTS "${CellForge_ROOT}/lib"
    DOC "Library for CellForge::${_comp}")

  # Runtime DLL (Windows shared-library build)
  if(WIN32)
    find_file(
      CellForge_${_comp}_DLL
      NAMES "${_lib_name}.dll"
      HINTS "${CellForge_ROOT}/bin"
      DOC "Runtime DLL for CellForge::${_comp}")
  endif()

  set(_comp_vars CellForge_${_comp}_INCLUDE_DIR CellForge_${_comp}_LIBRARY)
  if(WIN32)
    list(APPEND _comp_vars CellForge_${_comp}_DLL)
  endif()

  find_package_handle_standard_args(CellForge_${_comp} REQUIRED_VARS ${_comp_vars})
  mark_as_advanced(CellForge_${_comp}_INCLUDE_DIR CellForge_${_comp}_LIBRARY CellForge_${_comp}_DLL)

  if(CellForge_${_comp}_FOUND AND NOT TARGET CellForge::${_comp})
    if(WIN32)
      add_library(CellForge::${_comp} SHARED IMPORTED)
      set_target_properties(CellForge::${_comp} PROPERTIES IMPORTED_LOCATION "${CellForge_${_comp}_DLL}"
                                                           IMPORTED_IMPLIB "${CellForge_${_comp}_LIBRARY}")
    else()
      add_library(CellForge::${_comp} UNKNOWN IMPORTED)
      set_target_properties(CellForge::${_comp} PROPERTIES IMPORTED_LOCATION "${CellForge_${_comp}_LIBRARY}")
    endif()
    set_target_properties(CellForge::${_comp} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                         "${CellForge_${_comp}_INCLUDE_DIR}")
  endif()

  if(CellForge_FIND_REQUIRED_${_comp})
    list(
      APPEND
      _CellForge_required_vars
      CellForge_${_comp}_INCLUDE_DIR
      CellForge_${_comp}_LIBRARY)
    if(WIN32)
      list(APPEND _CellForge_required_vars CellForge_${_comp}_DLL)
    endif()
  endif()
endforeach()

find_package_handle_standard_args(
  CellForge
  REQUIRED_VARS _CellForge_required_vars CellForge_ROOT
  VERSION_VAR CellForge_VERSION
  HANDLE_COMPONENTS)

mark_as_advanced(CellForge_ROOT CellForge_INCLUDE_DIR)
