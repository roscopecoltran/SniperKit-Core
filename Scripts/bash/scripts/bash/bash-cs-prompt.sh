#!/bin/sh

# setting new escape sequences
echo $PS

# \\a an ASCII bell character (07)
#\\d the date 
#\\e an ASCII escape character (033)
#\\h the hostname up to the first `.'
#\\H the hostname
#\\j the number of jobs currently managed by the shell
#\\l the basename of the shell's terminal device name
#\\n newline
#\\r carriage return
#\\s the name of the shell, the basename of $0 (the portion following the final slash)
#\\t the current time in 24-hour HH:MM:SS format 
#\\T the current time in 12-hour HH:MM:SS format \\@ the current time in 12-hour am/pm format
#\\u the username of the current user
#\\v the version of bash (e.g., 2.00)
#\\V the release of bash, version + patchlevel
#\\w the current working directory
#\\W the basename of the current working directory
#\\! the history number of this command
#\\# the command number of this command
#\\$ if the effective UID is 0, a #, otherwise a $
#\\nnn the character corresponding to the octalnumber nnn 
#\\\\ a backslash \\[ begin a sequence of non-printing characters
# \\] end a sequence of non-printing character
PS1="[\\t][\\u@\\h:\\W]\\$ "
[\\t][\\u@\\h:\\W]$


# EXPORTS PROMPT
export PS1=“$NM[ $HI\u $HII\h $SI\w$NM ] $IN”

# naming tabs
echo $PROMPT_COMMAND
echo -ne "\\033]0;${PWD/#HOME/~}\\007"
export PROMPT_COMMAND='echo -ne "\\033]0;YOUR NAME HERE\\007"'
export PROMPT_COMMAND='echo -ne "\\033]0;${PWD/#$HOME/~}\\007"'