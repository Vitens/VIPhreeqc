set(phreeqc_HTML
  index.html
  serve.py
)

foreach(f ${phreeqc_HTML})
  configure_file(${f} ${f} COPYONLY)
endforeach()

if(WIN32)
  install(FILES ${phreeqc_DATABASE} DESTINATION database)
else()
  install(FILES ${phreeqc_DATABASE} DESTINATION ${CMAKE_INSTALL_DOCDIR}/database)
endif()