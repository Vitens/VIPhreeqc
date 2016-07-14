# project
if (IPHREEQC_FORTRAN_TESTING AND IPHREEQC_ENABLE_MODULE)
  project(example_advect_f90 Fortran)
endif()

# files
SET(FORTRAN_Advect_Files
advect.F90
ic
phreeqc.dat
)

if (IPHREEQC_FORTRAN_TESTING AND IPHREEQC_ENABLE_MODULE)
  # src
  SET(FORTRAN_Advect_SRC
  advect.F90
  ../../../src/IPhreeqc_interface.F90
  )
  
  # place [*.F90] files into "Source Files"
  source_group("Source Files" FILES ${FORTRAN_Advect_SRC})  

  # executable
  add_executable(example_advect_f90 ${FORTRAN_Advect_SRC})

  # library dependencies
  SET(EXTRA_LIBS ${EXTRA_LIBS} IPhreeqc)

  # link 
  target_link_libraries(example_advect_f90 ${EXTRA_LIBS})
endif()

# install directory
SET(FORTRAN_Advect_Dir ${EXAMPLES_DIR}/fortran/advect)

# install
install(FILES ${FORTRAN_Advect_Files} DESTINATION ${FORTRAN_Advect_Dir})
