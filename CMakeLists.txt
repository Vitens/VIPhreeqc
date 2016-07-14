cmake_minimum_required (VERSION 2.8)

project (IPhreeqc CXX C)

# check if this is the root project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(STANDALONE_BUILD 1)
else()
  set(STANDALONE_BUILD 0)
endif()


# overide docdir on windows
if(WIN32 AND NOT CMAKE_INSTALL_DOCDIR)
  set(CMAKE_INSTALL_DOCDIR "" CACHE PATH "documentation root (doc)")
  set(CMAKE_INSTALL_DOCDIR "doc")
endif()

# set standard directory locations
include(GNUInstallDirs)


# Fortran
option (IPHREEQC_ENABLE_MODULE "Enable Fortran module" ON)
if (STANDALONE_BUILD EQUAL 1)
  if (NOT IPHREEQC_FORTRAN_TESTING)
    if (NOT CMAKE_Fortran_COMPILER OR CMAKE_Fortran_COMPILER STREQUAL "CMAKE_Fortran_COMPILER-NOTFOUND")
      option (IPHREEQC_FORTRAN_TESTING "Build Fortran test" OFF)
    else()
      option (IPHREEQC_FORTRAN_TESTING "Build Fortran test" ON)
    endif()
  else()
    if (IPHREEQC_FORTRAN_TESTING)
      enable_language(Fortran)
    endif()  
  endif()
endif()


# Disable deprecation warnings for standard C and STL functions in VS2005
# and later
if(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION GREATER 1400)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION GREATER 1400) 

add_definitions(-DSWIG_SHARED_OBJ)
add_definitions(-DUSE_PHRQ_ALLOC)

SET(IPhreeqc_SOURCES
src/CSelectedOutput.cpp
src/CSelectedOutput.hxx
src/CVar.hxx
src/Debug.h
src/ErrorReporter.hxx
src/IPhreeqc.cpp
src/IPhreeqc.h
src/IPhreeqc.hpp
src/IPhreeqc_interface_F.cpp
src/IPhreeqcCallbacks.h
src/IPhreeqcLib.cpp
src/phreeqcpp/advection.cpp
src/phreeqcpp/basicsubs.cpp
src/phreeqcpp/ChartHandler.cpp
src/phreeqcpp/ChartHandler.h
src/phreeqcpp/ChartObject.cpp
src/phreeqcpp/ChartObject.h
src/phreeqcpp/cl1.cpp
src/phreeqcpp/common/Parser.cxx
src/phreeqcpp/common/Parser.h
src/phreeqcpp/common/PHRQ_base.cxx
src/phreeqcpp/common/PHRQ_base.h
src/phreeqcpp/common/PHRQ_io.cpp
src/phreeqcpp/common/PHRQ_io.h
src/phreeqcpp/common/phrqtype.h
src/phreeqcpp/common/Utils.cxx
src/phreeqcpp/common/Utils.h
src/phreeqcpp/CurveObject.cpp
src/phreeqcpp/CurveObject.h
src/phreeqcpp/cvdense.cpp
src/phreeqcpp/cvdense.h
src/phreeqcpp/cvode.cpp
src/phreeqcpp/cvode.h
src/phreeqcpp/cxxKinetics.cxx
src/phreeqcpp/cxxKinetics.h
src/phreeqcpp/cxxMix.cxx
src/phreeqcpp/cxxMix.h
src/phreeqcpp/dense.cpp
src/phreeqcpp/dense.h
src/phreeqcpp/Dictionary.cpp
src/phreeqcpp/Dictionary.h
src/phreeqcpp/dumper.cpp
src/phreeqcpp/dumper.h
src/phreeqcpp/Exchange.cxx
src/phreeqcpp/Exchange.h
src/phreeqcpp/ExchComp.cxx
src/phreeqcpp/ExchComp.h
src/phreeqcpp/GasComp.cxx
src/phreeqcpp/GasComp.h
src/phreeqcpp/gases.cpp
src/phreeqcpp/GasPhase.cxx
src/phreeqcpp/GasPhase.h
src/phreeqcpp/global_structures.h
src/phreeqcpp/input.cpp
src/phreeqcpp/integrate.cpp
src/phreeqcpp/inverse.cpp
src/phreeqcpp/ISolution.cxx
src/phreeqcpp/ISolution.h
src/phreeqcpp/ISolutionComp.cxx
src/phreeqcpp/ISolutionComp.h
src/phreeqcpp/isotopes.cpp
src/phreeqcpp/kinetics.cpp
src/phreeqcpp/KineticsComp.cxx
src/phreeqcpp/KineticsComp.h
src/phreeqcpp/mainsubs.cpp
src/phreeqcpp/model.cpp
src/phreeqcpp/NA.h
src/phreeqcpp/NameDouble.cxx
src/phreeqcpp/NameDouble.h
src/phreeqcpp/NumKeyword.cxx
src/phreeqcpp/NumKeyword.h
src/phreeqcpp/nvector.cpp
src/phreeqcpp/nvector.h
src/phreeqcpp/nvector_serial.cpp
src/phreeqcpp/nvector_serial.h
src/phreeqcpp/parse.cpp
src/phreeqcpp/PBasic.cpp
src/phreeqcpp/PBasic.h
src/phreeqcpp/phqalloc.cpp
src/phreeqcpp/phqalloc.h
src/phreeqcpp/Phreeqc.cpp
src/phreeqcpp/Phreeqc.h
src/phreeqcpp/PhreeqcKeywords/Keywords.cpp
src/phreeqcpp/PhreeqcKeywords/Keywords.h
src/phreeqcpp/PHRQ_io_output.cpp
src/phreeqcpp/pitzer.cpp
src/phreeqcpp/pitzer_structures.cpp
src/phreeqcpp/PPassemblage.cxx
src/phreeqcpp/PPassemblage.h
src/phreeqcpp/PPassemblageComp.cxx
src/phreeqcpp/PPassemblageComp.h
src/phreeqcpp/prep.cpp
src/phreeqcpp/Pressure.cxx
src/phreeqcpp/Pressure.h
src/phreeqcpp/print.cpp
src/phreeqcpp/Reaction.cxx
src/phreeqcpp/Reaction.h
src/phreeqcpp/read.cpp
src/phreeqcpp/ReadClass.cxx
src/phreeqcpp/readtr.cpp
src/phreeqcpp/runner.cpp
src/phreeqcpp/runner.h
src/phreeqcpp/SelectedOutput.cpp
src/phreeqcpp/SelectedOutput.h
src/phreeqcpp/Serializer.cxx
src/phreeqcpp/Serializer.h
src/phreeqcpp/sit.cpp
src/phreeqcpp/smalldense.cpp
src/phreeqcpp/smalldense.h
src/phreeqcpp/Solution.cxx
src/phreeqcpp/Solution.h
src/phreeqcpp/SolutionIsotope.cxx
src/phreeqcpp/SolutionIsotope.h
src/phreeqcpp/spread.cpp
src/phreeqcpp/SS.cxx
src/phreeqcpp/SS.h
src/phreeqcpp/SSassemblage.cxx
src/phreeqcpp/SSassemblage.h
src/phreeqcpp/SScomp.cxx
src/phreeqcpp/SScomp.h
src/phreeqcpp/step.cpp
src/phreeqcpp/StorageBin.cxx
src/phreeqcpp/StorageBin.h
src/phreeqcpp/StorageBinList.cpp
src/phreeqcpp/StorageBinList.h
src/phreeqcpp/structures.cpp
src/phreeqcpp/sundialsmath.cpp
src/phreeqcpp/sundialsmath.h
src/phreeqcpp/sundialstypes.h
src/phreeqcpp/Surface.cxx
src/phreeqcpp/Surface.h
src/phreeqcpp/SurfaceCharge.cxx
src/phreeqcpp/SurfaceCharge.h
src/phreeqcpp/SurfaceComp.cxx
src/phreeqcpp/SurfaceComp.h
src/phreeqcpp/System.cxx
src/phreeqcpp/System.h
src/phreeqcpp/tally.cpp
src/phreeqcpp/Temperature.cxx
src/phreeqcpp/Temperature.h
src/phreeqcpp/tidy.cpp
src/phreeqcpp/transport.cpp
src/phreeqcpp/Use.cpp
src/phreeqcpp/Use.h
src/phreeqcpp/UserPunch.cpp
src/phreeqcpp/UserPunch.h
src/phreeqcpp/utilities.cpp
src/thread.h
src/Var.c
src/Var.h
src/Version.h
)

# compile Var.c as c++
SET_SOURCE_FILES_PROPERTIES(src/Var.c PROPERTIES LANGUAGE CXX)

# fwrap files
if (NOT IPHREEQC_ENABLE_MODULE)
  add_definitions(-DIPHREEQC_NO_FORTRAN_MODULE)
  SET(IPhreeqc_SOURCES
  ${IPhreeqc_SOURCES}
  src/fimpl.h
  src/fwrap.cpp
  src/fwrap1.cpp
  src/fwrap2.cpp
  src/fwrap3.cpp
  src/fwrap4.cpp
  src/fwrap5.cpp
  src/fwrap6.cpp      
  src/fwrap7.cpp
  src/fwrap8.cpp      
 )
endif()

# setup include directories
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/phreeqcpp")
include_directories("${PROJECT_SOURCE_DIR}/src/phreeqcpp/common")
include_directories("${PROJECT_SOURCE_DIR}/src/phreeqcpp/PhreeqcKeywords")


if (STANDALONE_BUILD EQUAL 1)
  option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
  set(LIB_TYPE STATIC)
endif()

if (BUILD_SHARED_LIBS)
  set(LIB_TYPE SHARED)
  if (STANDALONE_BUILD EQUAL 1)
    if (MSVC)
      option(BUILD_CLR_LIBS "Build CLR Libraries" OFF)
    endif()
  endif()
endif()
add_library(IPhreeqc ${LIB_TYPE} ${IPhreeqc_SOURCES})
set_target_properties(IPhreeqc PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(IPhreeqc PROPERTIES MINSIZEREL_POSTFIX "msr")
set_target_properties(IPhreeqc PROPERTIES RELWITHDEBINFO_POSTFIX "rwdi")

SET(IPhreeqc_Headers
${PROJECT_SOURCE_DIR}/src/IPhreeqc.h
${PROJECT_SOURCE_DIR}/src/IPhreeqc.hpp
${PROJECT_SOURCE_DIR}/src/IPhreeqcCallbacks.h
${PROJECT_SOURCE_DIR}/src/phreeqcpp/PhreeqcKeywords/Keywords.h
${PROJECT_SOURCE_DIR}/src/phreeqcpp/common/PHRQ_io.h
${PROJECT_SOURCE_DIR}/src/Var.h
)

if (IPHREEQC_ENABLE_MODULE)
  SET(IPhreeqc_SRC
  ${PROJECT_SOURCE_DIR}/src/IPhreeqc_interface.F90
  ${PROJECT_SOURCE_DIR}/src/README.Fortran
  )
else()
  # always install README.Fortran
  SET(IPhreeqc_SRC
  ${PROJECT_SOURCE_DIR}/src/README.Fortran
  )
  # install old fortran include files
  SET(IPhreeqc_Headers
  ${IPhreeqc_Headers}
  ${PROJECT_SOURCE_DIR}/src/IPhreeqc.f.inc
  ${PROJECT_SOURCE_DIR}/src/IPhreeqc.f90.inc
  )
endif()

# Setup references for /CLR
if (MSVC AND BUILD_SHARED_LIBS AND BUILD_CLR_LIBS)
  set_target_properties(IPhreeqc PROPERTIES VS_DOTNET_REFERENCES "System;System.Drawing;System.Windows.Forms;${CMAKE_CURRENT_SOURCE_DIR}/src/phreeqcpp/ZedGraph.dll")  
  # Remove /EHsc from CMAKE_CXX_FLAGS
  STRING(REPLACE " /EHsc" " /EHa /clr" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  # Remove /RTC1 from CMAKE_CXX_FLAGS_DEBUG
  STRING(REPLACE " /RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
  if (MSVC10)
    # turn off warnings on VS2010
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4793 /wd4945")
  endif()  
  if (MSVC_VERSION LESS 1600)
    message(FATAL_ERROR "CLR options must be set manually for versions prior to Visual Studio 2010")
  endif()  
  ADD_DEFINITIONS(-DMULTICHART)
endif()


# install
if (WIN32)
  if (BUILD_CLR_LIBS)
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
      SET(CMAKE_INSTALL_LIBDIR "clrx64")
    else()
      SET(CMAKE_INSTALL_LIBDIR "clr")
    endif()
  else()
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
      if (BUILD_SHARED_LIBS)
        SET(CMAKE_INSTALL_LIBDIR "dllx64")
      else()
        SET(CMAKE_INSTALL_LIBDIR "libx64")
      endif()
    else()
      if (BUILD_SHARED_LIBS)
        SET(CMAKE_INSTALL_LIBDIR "dll")
      else()
        SET(CMAKE_INSTALL_LIBDIR "lib")
      endif()
    endif()
  endif()
endif()

# install
if (STANDALONE_BUILD EQUAL 1)
  install(TARGETS IPhreeqc DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(FILES ${IPhreeqc_Headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  if (WIN32)
    install(FILES ${IPhreeqc_SRC} DESTINATION src)
  else()
    install(FILES ${IPhreeqc_SRC} DESTINATION ${CMAKE_INSTALL_DOCDIR}/src)
  endif()
  if (BUILD_CLR_LIBS)
    install(FILES "${PROJECT_SOURCE_DIR}/src/phreeqcpp/ZedGraph.dll" DESTINATION ${CMAKE_INSTALL_LIBDIR})
  endif()
endif()

##IF(MSVC)
##  message("PROJECT_BINARY_DIR = ${PROJECT_BINARY_DIR}")
##  INSTALL(
##    DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
##    DESTINATION ${CMAKE_INSTALL_LIBDIR}
##    CONFIGURATIONS Debug
##    FILES_MATCHING
##    PATTERN "*.pdb"
##    )
##  INSTALL(
##    FILES
##    ${LIBRARY_OUTPUT_PATH}/Debug/${target_prefix}${target_name}.pdb
##    DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
##    DESTINATION ${CMAKE_INSTALL_LIBDIR}
##    CONFIGURATIONS Debug
##    )
##ENDIF(MSVC)

if (STANDALONE_BUILD EQUAL 1 AND MSVC)
  option (IPHREEQC_STATIC_RUNTIME "Build with a static runtime" OFF)  
  if (IPHREEQC_STATIC_RUNTIME)    
    # compile with static runtime
    
    set(CompilerFlags
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL
    CMAKE_CXX_FLAGS_RELWITHDEBINFO
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_C_FLAGS_RELWITHDEBINFO
    )

    foreach(CompilerFlag ${CompilerFlags})
      if(${CompilerFlag} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
      endif()
    endforeach()
  
    set(FortranCompilerFlags
    CMAKE_Fortran_FLAGS
    )
  
    foreach(FortranCompilerFlags ${FortranCompilerFlags})
      if(${FortranCompilerFlags} MATCHES "/libs:dll")
        string(REGEX REPLACE "/libs:dll" "/libs:static" ${FortranCompilerFlags} "${${FortranCompilerFlags}}")
      endif()  
    endforeach()
  endif()
endif()

if (STANDALONE_BUILD EQUAL 1)
  # tests
  enable_testing()

  add_subdirectory(database)
  add_subdirectory(doc)
  add_subdirectory(examples)
  add_subdirectory(tests)

  if (EXISTS "${PROJECT_SOURCE_DIR}/CTestConfig.cmake")
    # enable dashboard scripting
    include (CTest)
  endif()

  # enable distribution creation
  set(CPACK_PACKAGE_VERSION "3.2.0")
  set(CPACK_PACKAGE_VENDOR "USGS")
  include (CPack)
endif()
