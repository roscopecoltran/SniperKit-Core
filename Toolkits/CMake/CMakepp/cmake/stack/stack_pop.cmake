
function(stack_pop stack)
  map_tryget("${stack}" back)
  ans(current_index)
  if(NOT current_index)
    return()
  endif()
  map_tryget("${stack}" "${current_index}")
  ans(res)
  math(EXPR current_index "${current_index} - 1")
  map_set_hidden("${stack}" back "${current_index}")
  return_ref(res)
endfunction()
