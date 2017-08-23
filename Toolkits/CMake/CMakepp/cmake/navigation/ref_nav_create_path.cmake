
  function(ref_nav_create_path expression)
    navigation_expression_parse("${expression}")
    ans(expression)
    set(current_value ${ARGN})
    while(true)
      list(LENGTH expression continue)
      if(NOT continue)
        break()
      endif()

      list_pop_back(expression)
      ans(current_expression)
      if(NOT "${current_expression}" STREQUAL "[]")
        if("${current_expression}" MATCHES "^[<>].*[<>]$")
          message(FATAL_ERROR "invalid range: ${current_expression}")
        endif()
        map_new()
        ans(next_value)
        map_set("${next_value}" "${current_expression}" "${current_value}")
        set(current_value "${next_value}")
      endif()
    endwhile()
    return_ref(current_value)
  endfunction()


