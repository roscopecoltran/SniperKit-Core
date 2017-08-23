
## <a name="package_source_default"></a> Default Package Source
### <a href="packages_default_methods"></a> Default Package Source nad Default Package Functions

The default package source combines access to github, bitbucket,webarchives, git, svn, hg, local archives and local dirs in a single package source. 

It can be accessed conveniently by these global functions

* `default_package_source() -> <default package source>`
* `query_package(<~uri>):<package uri...>`
* `resolve_package(<~uri>):<package handle?>`
* `pull_package(<~uri> <target dir?>):<package handle>`

*Examples*

```

## pull a github package to current user's home dir from github
pull_package("toeb/cmakepp" "~/current_cmakepp")

## pull a bitbucket package to pwd
pull_package("eigen/eigen")

## pull a package which exists in both bitbucket and github under the same user/name from github
pull_package("github:toeb/test_repo")

## find all packages from user toeb in bitbucket and github

assign(package_uris = query_package(toeb))
foreach(package_uri ${package_uris})
  message("  ${package_uri}")
endforeach()

```
