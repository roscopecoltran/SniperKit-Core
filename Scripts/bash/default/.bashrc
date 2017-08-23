#move user executables to beginning of path
grep -q '/usr/local/bin' <<< "$PATH" &&
  export PATH=`echo "$PATH" | sed -E 's|(.*):/usr/local/bin(.*)|/usr/local/bin:\1\2|'`

#for coreutils and gnu-sed
export MANPATH="/usr/local/opt/coreutils/libexec/gnuman:$MANPATH"
export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"
export MANPATH="/usr/local/opt/gnu-sed/libexec/gnuman:$MANPATH"
export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:$PATH"

#add timestamps to bash history
export HISTTIMEFORMAT='%F %T '

#enable shell colors (ls and others)
#from https://github.com/seebi/dircolors-solarized
eval `dircolors ~/.dircolors`

#add syntax highlighting to less via gnu source-highlight
export LESSOPEN="| `brew --prefix`/bin/src-hilite-lesspipe.sh %s"
export LESS=' -R '

#extended source-highlight definitions (see src-hilite-lesspipe.sh)
export SOURCE_HIGHLIGHT_BASH_EXT=".bashrc .profile"

alias grep='grep --color'
alias ls='ls --color'

alias ll='ls -lArth'
alias la='ls -A'
alias rm=trash

#growl() {
#  echo -ne "\e]9;${*}\007"
#}

if [ -f $(brew --prefix)/etc/bash_completion ]; then
	. $(brew --prefix)/etc/bash_completion
fi

source ~/.git-completion.sh

if [ -f $(brew --prefix)/etc/autojump.bash ]; then
  . $(brew --prefix)/etc/autojump.bash
fi

if [ -f $(brew --prefix)/etc/bash_completion.d/vagrant ]; then
  . $(brew --prefix)/etc/bash_completion.d/vagrant
fi

complete -C '/usr/local/bin/aws_completer' aws

export WORKON_HOME=~/.virtualenvs
export PROJECT_HOME=~
export VIRTUAL_ENV_DISABLE_PROMPT=1
source /usr/local/bin/virtualenvwrapper.sh

export ANSIBLE_NOCOWS=1

source ~/.bash_colors

#Handy git aliases
lastm() { git --no-pager log --merges -n1 --format='%H'; }
lastp() { git --no-pager rev-parse "@{u}"; }
alias gitcd='cd $(git rev-parse --show-toplevel)'
  

#-- Prompt --
shorten_path () {
  #arg: path to shorten (defaults to PWD)
  path="${1:-$PWD}"
  shortened=$(echo "${path/#$HOME/\~}" | awk -F "/" '
    {if (length($0) > 35)
      {if (NF>4)
        print $1 "/" $2 "/.../" $(NF-1) "/" $NF;
      else if (NF>3) 
        print $1 "/" $2 "/.../" $NF; 
      else 
        print $1 "/.../" $NF; 
    } else 
      print $0;}
  ')
  echo "${BGreen}${shortened}${NoColor}"
}

user_at_loc () {
  echo "${BBlue}\u${NoColor}@${BPurple}\h${NoColor}"
}

git_prompt () {
  git_branch=$(git symbolic-ref HEAD 2>&1)
  r=$?
  if [ "$git_branch" = "fatal: ref HEAD is not a symbolic ref" ]; then 
    echo "${Red}???${NoColor} "
  
  elif [ $r -eq 0 ]; then
    git_branch="${git_branch#refs/heads/}"
    s=`/usr/bin/git status`
    
    if grep -q "ahead of" <<<"$s"; then
      symbol="+"
    elif grep -q "behind" <<<"$s"; then
      symbol="-"
    elif grep -q "diverged" <<<"$s"; then
      symbol='!!'
    fi

    if grep -q "nothing to commit" <<<"$s"; then
      git_branch="${Green}($git_branch)${NoColor}"
    elif grep -q -E "Untracked files|not staged" <<<"$s"; then
      git_branch="${Yellow}($git_branch)${NoColor}"
    elif grep -q "Changes to be committed" <<<"$s"; then
      git_branch="${Red}($git_branch)${NoColor}"
    fi
    
    [ -n "$git_branch" ] && echo "${BYellow}${symbol}${NoColor}${git_branch}"
  fi
}

git_branch () {
  toplevel=$(git rev-parse --show-toplevel)
  echo "${BPurple}"$(basename "$toplevel")"${NoColor}"
}

git_path () {
  toplevel="$(git rev-parse --show-toplevel)"
  shortened=-$([ "$toplevel" != "$PWD" ] && shorten_path ${PWD#$toplevel})
  echo "${BGreen}$shortened${NoColor}"
}

set_prompt () {
  gitstuff=$(git_prompt)
  if [ -n "$VIRTUAL_ENV" ]; then
    venv="${Yellow}!${NoColor} "
  else
    venv=""
  fi

  if [ -n "$gitstuff" ]; then
    prompt="$(git_branch) $(git_prompt): $(git_path)"
  else
    prompt="$(user_at_loc): $(shorten_path)"
  fi
  export PS1="${venv}${prompt} ${BBlue}\$${NoColor} "
}

PROMPT_COMMAND="$PROMPT_COMMAND; set_prompt"

export SUDO_PS1="${IWhite}${On_Red}\u@\h: \W#${NoColor} "

# Use https://github.com/huyng/bashmarks and/or https://github.com/joelthelion/autojump
# for autojump, remember to source its .bash file