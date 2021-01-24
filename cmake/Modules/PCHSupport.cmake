include(cotire)

if(WIN32)
function(GEN_CXX_PCH TARGET_LIST PCH_HEADERS)
  # iterate through targets
  foreach(TARGET_HEADER ${TARGET_LIST})
    # unity builds disabled
    set_target_properties(${TARGET_HEADER} PROPERTIES COTIRE_ADD_UNITY_BUILD OFF)

    # prefix header
    set_target_properties(${TARGET_HEADER} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT ${PCH_HEADERS})
  endforeach()
  cotire(${TARGET_LIST})
endfunction(GEN_CXX_PCH)
else()
  function(GEN_CXX_PCH TARGET_LIST PCH_HEADERS)
    foreach(TARGET_HEADER ${TARGET_LIST})
      target_precompile_headers(${TARGET_HEADER} PRIVATE ${PCH_HEADERS})
    endforeach()
  endfunction(GEN_CXX_PCH)
endif()
