# MIT License
#
# Copyright (c) 2024 Kritzel Kratzel.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in 
# the Software without restriction, including without limitation the rights to 
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all 
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# ------------------------------------------------------------------------------
# lua4882 DLL
add_library(lua4882 SHARED)
# setup lua include directory
target_include_directories(lua4882 PRIVATE ${LIBLUA_INCLUDEDIR})
# setup platform-specific sources, compile and linker options
if(WIN32 AND NOT MinGW)
  target_include_directories(lua4882 PRIVATE ${NI4882_INCDIR})
  target_compile_options(lua4882 PRIVATE /D_WINDLL /D_WIN32 /D_CRT_SECURE_NO_WARNINGS)
  if(${CMAKE_SYSTEM_VERSION} EQUAL 6.1.7601)
    # Windows 7 SP1
    target_link_options(lua4882 PRIVATE /LIBPATH:${LIBLUA_LIBDIR} liblua.lib
      /LIBPATH:${NI4882_LIBDIR} ni4882.obj)
  else()
    # Windows 10
    target_link_options(lua4882 PRIVATE /LIBPATH:${LIBLUA_LIBDIR} liblua.lib
      /LIBPATH:${NI4882_LIBDIR} ni4882scrt.obj)
  endif()
else()
  message(FATAL_ERROR "Not yet fully implemented.")
endif()
# plattform-independend sources
target_sources(lua4882 PRIVATE lua4882.c)
# Install
install(TARGETS lua4882 RUNTIME DESTINATION ${INSTALL_TOP_CDIR})
