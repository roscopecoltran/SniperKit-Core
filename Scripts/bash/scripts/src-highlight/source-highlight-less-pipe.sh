#! /bin/sh

# My extensions to the standard src-hilite-lesspipe.sh provided by gnu source-highlight
# Adds an environment variable SOURCE_HIGHLIGHT_BASH_EXT that defines additional specific files
# that should be highlighted as bash

for source in "$@"; do
    case $source in
  *ChangeLog|*changelog)
        source-highlight --failsafe -f esc --lang-def=changelog.lang --style-file=esc.style -i "$source" ;;
  *Makefile|*makefile)
        source-highlight --failsafe -f esc --lang-def=makefile.lang --style-file=esc.style -i "$source" ;;
  *)
    grep -q `basename $source` <<< $SOURCE_HIGHLIGHT_BASH_EXT &&
      source-highlight --failsafe -f esc --lang-def=sh.lang --style-file=esc.style -i "$source"

    source-highlight --failsafe --infer-lang -f esc --style-file=esc.style -i "$source"
    ;;
    esac
done