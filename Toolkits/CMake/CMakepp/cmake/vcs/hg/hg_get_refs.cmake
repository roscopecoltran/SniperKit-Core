
function(hg_get_refs)
  hg(branches)
  ans(branches)
  string_split("${branches}" "\n")
  hg(tags)
  ans(tags)  
  string_split("${tags}" "\n")
  ans(tags)


  set(refs)
  foreach(ref ${tags}  )
    hg_parse_ref("${ref}")
    ans(ref)
    map_set("${ref}" type "tag")
    list(APPEND refs "${ref}") 
  endforeach()
  foreach(ref ${branches}  )
    hg_parse_ref("${ref}" )
    ans(ref)
    map_set("${ref}" type "branch")
    list(APPEND refs "${ref}") 
  endforeach()
  return_ref(refs)
endfunction()