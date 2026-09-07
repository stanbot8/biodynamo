#!/usr/bin/env bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------
# Source this script to set up the BioDynaMo build that this script is part of.
#
# Conveniently an alias like this can be defined in .bashrc or .zshrc:
#   alias thisbdm="source path/to/biodynamo/bin/thisbdm.sh"
#
# This script is for bash like shells, see thisbdm.fish for fish.
#
# Author: Fons Rademakers, 16/5/2019

_bdm_err()
{
  if [ "$BDM_THISBDM_LOGLEVEL" -gt 0 ]; then echo -e "\e[91m$*\e[0m"; fi
}

_bdm_warn()
{
  if [ "$BDM_THISBDM_LOGLEVEL" -gt 1 ]; then echo -e "\e[93m$*\e[0m"; fi
}

_bdm_info()
{
  if [ "$BDM_THISBDM_LOGLEVEL" -gt 3 ]; then echo -e "\e[95m$*\e[0m"; fi
}

_bdm_ok()
{
  if [ "$BDM_THISBDM_LOGLEVEL" -gt 4 ]; then echo -e "\e[92m$*\e[0m"; fi
}

_drop_bdm_from_path()
{
  # Assert that we got enough arguments
  if test $# -ne 2 ; then
    _bdm_err "[ERR] drop_bdm_from_path: needs 2 arguments"
    return 1
  fi

  local p=$1
  local drop=$2

  _newpath=$(echo "$p" | sed -e "s;:${drop}:;:;g"\
                             -e "s;:${drop}\$;;g"\
                             -e "s;^${drop}:;;g" \
                             -e "s;^${drop}\$;;g"\
                             -e "s; ${drop}\$;;g"\
                             -e "s;^${drop} ;;g")
}

_bdm_define_command()
{
  unset -f "$1" >/dev/null 2>&1 || true
  unalias  "$1" >/dev/null 2>&1 || true

  if [ -n  "$ZSH_VERSION" ]; then
    autoload -Uz "$1" || return 1
  else
    # shellcheck disable=SC1090
    source "${BDMSYS}/bin/sh_functions/$1" || return 1
    export -f "${1?}" || return 1
  fi
  return 0
}

_bdm_unset()
{
  unset "$@" >/dev/null 2>&1 || true
}

_thisbdm_cleanup() {
  _bdm_unset thisbdm
  _bdm_unset _newpath
  _bdm_unset -f _drop_bdm_from_path
  _bdm_unset -f _source_thisbdm
  _bdm_unset -f _bdm_define_command
  _bdm_unset -f _bdm_err
  _bdm_unset -f _bdm_warn
  _bdm_unset -f _bdm_info
  _bdm_unset -f _bdm_ok
  unset -f _bdm_unset >/dev/null 2>&1 || true
  unset -f _thisbdm_cleanup >/dev/null 2>&1 || true
}

_source_thisbdm()
{
  ### Log verbosity config
  if [ -z "$BDM_THISBDM_LOGLEVEL" ]; then
    export BDM_THISBDM_LOGLEVEL=5 # enable everything
  fi

  # these two have priority over loglevel
  if [ "$BDM_THISBDM_QUIET" = true ]; then
    export BDM_THISBDM_LOGLEVEL=2 # disable prompt,info,ok
  fi
  if [ "$BDM_THISBDM_SILENT" = true ]; then
    # silent->quiet
    export BDM_THISBDM_LOGLEVEL=0 # disable everything
  fi
  
  if [ "$BDM_THISBDM_LOGLEVEL" -le 2 ]; then
    export BDM_THISBDM_NOPROMPT=true
  else
    export BDM_THISBDM_NOPROMPT=false
  fi
  ########

  ### Detect bash-like shell, and act accordingly ###
  local bdm_shell
  if [ -n "$BASH_VERSION" ]; then
    bdm_shell="bash"
  elif [ -n "$ZSH_VERSION" ]; then
    bdm_shell="zsh"
    # The reason why this script is wrapped in this god function
    # is to enable bash compatibility *only* inside its respective
    # scope. Local vars are a nice bonus.
    emulate -LR sh
  else
    # In case we encounter a shell (e.g., ksh) that can run this script up to here.
    # In reality, most shells (csh, fish, etc.) will fail immediately due to parse errors.
    _bdm_err "[ERR] Your shell is not supported, please use bash or zsh."
    _bdm_err "      For fish, please source 'thisbdm.fish' instead."
    return 1
  fi
  ########
  
  local old_bdmsys
  if [ -n "${BDMSYS}" ]; then
     old_bdmsys=${BDMSYS}
  fi

  # set bdmsys and collect relevant config files
  local config_files=("$HOME/.profile")
  case $bdm_shell in
    bash)
      BDMSYS=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd) || return 1
      config_files+=("$HOME/.bashrc" "$HOME/.bash_profile")
      ;;
    zsh)
      # The zsh equivalent of ${BASH_SOURCE[0]} is ${(%):-%x}
      # shellcheck disable=SC2154
      BDMSYS=$(cd "$(dirname "${(%):-%x}")/.." && pwd) || return 1
      config_files+=("$HOME/.zshrc" "$HOME/.zshenv" "$HOME/.zprofile")
      ;;
  esac
  export BDMSYS

  if [ -n "$old_bdmsys" ] && [ "$old_bdmsys" != "$BDMSYS" ]; then
    _bdm_warn "[WARN] You've already sourced another 'thisbdm' in your current shell session."
    _bdm_warn "       -> prev. installation='$old_bdmsys'"
    _bdm_warn "       -> this installation='$BDMSYS'"
    _bdm_warn "       You may encounter undefined behavior. It is recommended that you start"
    _bdm_warn "       a new shell session, or otherwise check if you automatically source"
    _bdm_warn "       'thisbdm' in one of your shell configuration files, e.g., '.bashrc'"
    _bdm_warn "       (this is no longer advised)."
  fi

  # check if any config files naively source thisbdm
  if [ "$BDM_THISBDM_LOGLEVEL" -gt 1 ]; then
    for f in "${config_files[@]}"; do
      if [ -f "$f" ]; then
        local source_pattern='^\s*(\.|source)\s+.*thisbdm\.(fish|sh).*'
        # one may append #IGNORE if match is a false positive, or they really want to keep that line
        local nr_matches=$(cat "$f" | grep -E "$source_pattern" | grep -vc '.*#IGNORE$')
        if [ "$nr_matches" -gt '0' ]; then
          _bdm_warn "[WARN] You may have sourced thisbdm in '$f'."
          _bdm_warn "       Please check as this is not advised."
        fi
      fi
    done
  fi

  # Clear the env from previously set BioDynaMo paths.
  if [ -n "${old_bdmsys}" ] ; then
     if [ -n "${PATH}" ]; then
      _drop_bdm_from_path "$PATH" "${old_bdmsys}/bin"
      PATH=$_newpath
     fi
     if [ -n "${LD_LIBRARY_PATH}" ]; then
      _drop_bdm_from_path "$LD_LIBRARY_PATH" "${old_bdmsys}/lib"
      LD_LIBRARY_PATH=$_newpath
     fi
     if [ -n "${DYLD_LIBRARY_PATH}" ]; then
      _drop_bdm_from_path "$DYLD_LIBRARY_PATH" "${old_bdmsys}/lib"
      DYLD_LIBRARY_PATH=$_newpath
     fi
     if [ -n "${SHLIB_PATH}" ]; then
      _drop_bdm_from_path "$SHLIB_PATH" "${old_bdmsys}/lib"
      SHLIB_PATH=$_newpath
     fi
     if [ -n "${LIBPATH}" ]; then
      _drop_bdm_from_path "$LIBPATH" "${old_bdmsys}/lib"
      LIBPATH=$_newpath
     fi
     if [ -n "${MANPATH}" ]; then
      _drop_bdm_from_path "$MANPATH" "${old_bdmsys}/man"
      MANPATH=$_newpath
     fi
     if [ -n "${CMAKE_PREFIX_PATH}" ]; then
      _drop_bdm_from_path "$CMAKE_PREFIX_PATH" "${old_bdmsys}"
      CMAKE_PREFIX_PATH=$_newpath
     fi
  fi

  # Clear the env from previously set PyEnv paths.
  local with_pyenv=@with_pyenv@
  if [ "$with_pyenv" = 'ON' ]; then
    if [ -n "${old_bdmsys}" ] ; then
      if [ -n "${PATH}" ]; then
        _drop_bdm_from_path "$PATH" "$PYENV_ROOT/bin"
        PATH=$_newpath
        _drop_bdm_from_path "$PATH" "$PYENV_ROOT/versions/@pythonvers@/bin"
        PATH=$_newpath
        _drop_bdm_from_path "$PATH" "$PYENV_ROOT/shims"
        PATH=$_newpath
      fi

      if [ -n "${LD_LIBRARY_PATH}" ]; then
        _drop_bdm_from_path "$LD_LIBRARY_PATH" "$PYENV_ROOT/versions/$PYENV_VERSION/lib"
        LD_LIBRARY_PATH=$_newpath
      fi
    fi
  fi    

  # If we run on macOS, we add the following exports for libomp support
  if [ "$(uname)" = "Darwin" ]; then
    BREWPREFIX=$(brew --prefix)
    if [ -n "${CPPFLAGS}" ]; then
      _drop_bdm_from_path "$CPPFLAGS" "-I$BREWPREFIX/opt/libomp/include"
      CPPFLAGS=$_newpath
    fi
    if [ -n "${LDFLAGS}" ]; then
      _drop_bdm_from_path "$LDFLAGS" "-L$BREWPREFIX/opt/libomp/lib"
      LDFLAGS=$_newpath
    fi
    export CPPFLAGS="-I$BREWPREFIX/opt/libomp/include $CPPFLAGS"
    export LDFLAGS="-L$BREWPREFIX/opt/libomp/lib $LDFLAGS"
  fi

  local with_paraview=@with_paraview@

  if [ -z "${MANPATH}" ]; then
     local default_manpath
     # Grab the default man path before setting the path to avoid duplicates
     if command -v manpath >/dev/null; then
      default_manpath=$(manpath)
     elif command -v man >/dev/null; then
      default_manpath=$(man -w 2> /dev/null)
     else
      default_manpath=''
     fi
  fi

  if [ -z "${PATH}" ]; then
    PATH="${BDMSYS}/bin"
  else
    PATH="${BDMSYS}/bin":$PATH
  fi
  export PATH

  if [ -z "${LD_LIBRARY_PATH}" ]; then
    LD_LIBRARY_PATH="${BDMSYS}/lib" # Linux, ELF HP-UX
  else
    LD_LIBRARY_PATH="${BDMSYS}/lib":$LD_LIBRARY_PATH
  fi
  export LD_LIBRARY_PATH

  if [ -z "${DYLD_LIBRARY_PATH}" ]; then
    DYLD_LIBRARY_PATH="${BDMSYS}/lib"; # Mac OS X
  else
    DYLD_LIBRARY_PATH="${BDMSYS}/lib":$DYLD_LIBRARY_PATH
  fi
  export DYLD_LIBRARY_PATH

  if [ -z "${SHLIB_PATH}" ]; then
    SHLIB_PATH="${BDMSYS}/lib" # legacy HP-UX
  else
    SHLIB_PATH="${BDMSYS}/lib":$SHLIB_PATH
  fi
  export SHLIB_PATH

  if [ -z "${LIBPATH}" ]; then
    LIBPATH="${BDMSYS}/lib" # AIX
  else
    LIBPATH="${BDMSYS}/lib":$LIBPATH
  fi
  export LIBPATH

  if [ -z "${MANPATH}" ]; then
    MANPATH="${BDMSYS}/man":${default_manpath}
  else
    MANPATH="${BDMSYS}/man":$MANPATH
  fi
  export MANPATH

  ##### Python Specific Configurations #####
  if [ "$with_pyenv" = 'ON' ]; then
    export PYENV_ROOT=@pyenvroot@
    if [ -z "${PYENV_ROOT}" ]; then
      export PYENV_ROOT="$HOME/.pyenv"
    fi
    export PATH="$PYENV_ROOT/bin:$PATH"

    # Rehashing is not possible within a read-only Singularity container
    # and will cause sourcing of thisbdm to get stuck
    if [ -n "$SINGULARITY_CONTAINER" ]; then
      PYENV_NO_REHASH="--no-rehash"
    fi

    eval "$(pyenv init --path $PYENV_NO_REHASH)" || return 1
    eval "$(pyenv init - $PYENV_NO_REHASH)" || return 1
    pyenv shell @pythonvers@ || return 1

    # Location of jupyter executable (installed with `pip install` command)
    export PATH="$PYENV_ROOT/versions/@pythonvers@/bin:$PATH"
    export LD_LIBRARY_PATH="$PYENV_ROOT/versions/@pythonvers@/lib":$LD_LIBRARY_PATH
  fi
  ########

  ##### CMake Specific Configurations #####
  if [ -z "${CMAKE_PREFIX_PATH}" ]; then
    CMAKE_PREFIX_PATH="${BDMSYS}/share/cmake" # Linux, ELF HP-UX
  else
    CMAKE_PREFIX_PATH="${BDMSYS}/share/cmake":$CMAKE_PREFIX_PATH
  fi
  export CMAKE_PREFIX_PATH
  ########

  #### ParaView Specific Configurations ####
  if [ "$with_paraview" = 'ON' ]; then
     if [ -z "${PV_PLUGIN_PATH}" ]; then
      PV_PLUGIN_PATH="${BDMSYS}/lib/pv_plugin"
     else
      PV_PLUGIN_PATH="${BDMSYS}/lib/pv_plugin":$PV_PLUGIN_PATH
     fi
     export PV_PLUGIN_PATH
  fi
  #######

  # OpenMP
  export OMP_PROC_BIND=true

  ###### Platform-specific Configuration
  # Apple specific
  if [ "$(uname)" = 'Darwin' ]; then
    # Nothing for now
    true
  else # GNU/Linux
    # CentOS specifics (no longer officially supported)
    local os_id
    os_id=$(grep -oP '(?<=^ID=).+' /etc/os-release | tr -d '"') || return 1
    if [ "$os_id" = 'centos' ]; then
        export MESA_GL_VERSION_OVERRIDE=3.3
        if [ -z "${CXX}" ] && [ -z "${CC}" ] ; then
            . scl_source enable devtoolset-10 || return 1
        fi
        . /etc/profile.d/modules.sh || return 1
        module load mpi || return 1

        # load llvm 6 required for libroadrunner
        if [ -d "${BDMSYS}"/third_party/libroadrunner ]; then
          . scl_source enable llvm-toolset-7 || return 1
        fi
    fi
  fi
  #######

  # completions for bash: really primitive (but useful nonetheless) 
  if [ -n "$BASH_VERSION" ]; then
    complete -W "new build clean run demo" biodynamo
  fi

  if [ -n "$ZSH_VERSION" ]; then
    # for autoload
    if [ -n "${old_bdmsys}" ]; then
      _drop_bdm_from_path "$FPATH" "${old_bdmsys}/bin/sh_functions"
      FPATH=$_newpath
    fi

    FPATH="${BDMSYS}/bin/sh_functions:$FPATH"
    export FPATH

    # completions for zsh
    autoload -Uz __bdm_zsh_completions || return 1
    # compinit || return 1 # FIXME zsh completion broken

  fi

  ### Environment Indicator ###
  if ! [ "$BDM_THISBDM_NOPROMPT" = true ]; then
    local bdm_major_minor='';
    bdm_major_minor=$(biodynamo --shortversion)
    if [ -z "$__bdm_sh_prompt_original" ]; then
      case $bdm_shell in
        bash) __bdm_sh_prompt_original=$PS1 ;;
        zsh)  __bdm_sh_prompt_original=$PROMPT ;;
      esac
    fi

    # NB: overrides prompt for current session only
    case $bdm_shell in
      bash) export PS1="[bdm-$bdm_major_minor] $__bdm_sh_prompt_original" ;;
      zsh)  export PROMPT="[bdm-$bdm_major_minor] $__bdm_sh_prompt_original" ;;
    esac
  fi

  return 0
}

# Run
if _source_thisbdm; then
  _bdm_ok "[OK] You have successfully sourced BioDynaMo's environment."
  _thisbdm_cleanup
  return 0
else
  _bdm_err "[ERR] BioDynaMo's environment could not be sourced."
  # traps don't particularly work well (for this use-case)
  # in zsh, so we'll repeat ourselves just this once
  _thisbdm_cleanup
  return 1
fi
