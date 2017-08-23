## `(<start:<cmake token>> [<end: <cmake token>])-><cmake token>...`
##
## returns all tokens for the specified range (or the end of the tokens)
function(cmake_token_range_to_list range)
  list_extract(range begin end)
  set(current ${begin})
  set(tokens)
  while(true)
    if(NOT current OR "${current}" STREQUAL "${end}")
      break()
    endif() 
    list(APPEND tokens ${current})
    map_tryget(${current} next)
    ans(current)
  endwhile()
  return_ref(tokens)
endfunction()