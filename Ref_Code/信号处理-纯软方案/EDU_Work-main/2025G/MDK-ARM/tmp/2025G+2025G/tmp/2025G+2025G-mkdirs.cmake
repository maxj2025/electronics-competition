# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/47306/Downloads/EDU_Work-main/EDU_Work-main/2025G/MDK-ARM/tmp/2025G+2025G")
  file(MAKE_DIRECTORY "C:/Users/47306/Downloads/EDU_Work-main/EDU_Work-main/2025G/MDK-ARM/tmp/2025G+2025G")
endif()
file(MAKE_DIRECTORY
  "C:/Users/47306/Downloads/EDU_Work-main/EDU_Work-main/2025G/MDK-ARM/tmp/1"
  "C:/Users/47306/Downloads/EDU_Work-main/EDU_Work-main/2025G/MDK-ARM/tmp/2025G+2025G"
  "C:/Users/47306/Downloads/EDU_Work-main/EDU_Work-main/2025G/MDK-ARM/tmp/2025G+2025G/tmp"
  "C:/Users/47306/Downloads/EDU_Work-main/EDU_Work-main/2025G/MDK-ARM/tmp/2025G+2025G/src/2025G+2025G-stamp"
  "C:/Users/47306/Downloads/EDU_Work-main/EDU_Work-main/2025G/MDK-ARM/tmp/2025G+2025G/src"
  "C:/Users/47306/Downloads/EDU_Work-main/EDU_Work-main/2025G/MDK-ARM/tmp/2025G+2025G/src/2025G+2025G-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/47306/Downloads/EDU_Work-main/EDU_Work-main/2025G/MDK-ARM/tmp/2025G+2025G/src/2025G+2025G-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/47306/Downloads/EDU_Work-main/EDU_Work-main/2025G/MDK-ARM/tmp/2025G+2025G/src/2025G+2025G-stamp${cfgdir}") # cfgdir has leading slash
endif()
