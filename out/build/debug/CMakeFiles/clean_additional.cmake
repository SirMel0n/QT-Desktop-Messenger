# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\DesktopMessenger_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\DesktopMessenger_autogen.dir\\ParseCache.txt"
  "DesktopMessenger_autogen"
  )
endif()
