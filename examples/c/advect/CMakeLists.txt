# project
project(example_advect_c C)

# files
SET(C_Advect_Files
advect.c
ic
phreeqc.dat
)

# src
SET(C_Advect_SRC
advect.c
)

# executable
add_executable(example_advect_c ${C_Advect_SRC})

# library dependencies
SET(EXTRA_LIBS ${EXTRA_LIBS} IPhreeqc)

# link 
target_link_libraries(example_advect_c ${EXTRA_LIBS})

# install directory
SET(C_Advect_Dir ${EXAMPLES_DIR}/c/advect)

# install
install(FILES ${C_Advect_Files} DESTINATION ${C_Advect_Dir})



