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
# Conveniently an alias like this can be defined in your config.fish:
#   alias thisbdm="source path/to/biodynamo/bin/thisbdm.fish -q"
#
# This script is for fish, see thisbdm.sh for bash like shells.
#
# 'thisbdm.sh' author: Fons Rademakers, 16/5/2019
# fish port author: M. Dorukhan Arslan, 26/7/2020

function _drop_from_var
    if test (count $argv) -ne 2
        echo "_drop_from_var: requires 2 arguments but has " (count $argv)
        return 1
    end

    set -l var $argv[1]
    set -l drop $argv[2]

    if  not set -q $var; or test -z "$drop"
        return 0
    end

    if set -l index (contains -i $drop $$var)
        set -e "$var"["$index"]
    end
end

function _bdm_err
    if test "$BDM_THISBDM_LOGLEVEL" -gt 0
        echo -e "\e[91m$argv\e[0m"
    end
end

function _bdm_warn
    if test "$BDM_THISBDM_LOGLEVEL" -gt 1
        echo -e "\e[93m$argv\e[0m"
    end
end

function _bdm_info
    if test "$BDM_THISBDM_LOGLEVEL" -gt 3
        echo -e "\e[95m$argv\e[0m"
    end
end

function _bdm_ok
    if test "$BDM_THISBDM_LOGLEVEL" -gt 4
        echo -e "\e[92m$argv\e[0m"
    end
end

function _thisbdm_cleanup
  functions -e source_thisbdm
  functions -e _drop_from_var
  functions -e _bdm_err
  functions -e _bdm_warn
  functions -e _bdm_info
  functions -e _bdm_ok
  functions -e _thisbdm_cleanup
end

function source_thisbdm
    ### Log verbosity config
    if test -z "$BDM_THISBDM_LOGLEVEL"
        set -gx BDM_THISBDM_LOGLEVEL 5 # enable everything
    end

    # these two have priority over loglevel
    if test "$BDM_THISBDM_QUIET" = true
        set -gx BDM_THISBDM_LOGLEVEL 2 # disable prompt,info,ok
    end
    if test "$BDM_THISBDM_SILENT" = true
        set -gx BDM_THISBDM_LOGLEVEL 0 # disable everything
    end

    if test "$BDM_THISBDM_LOGLEVEL" -le 2
        set -gx BDM_THISBDM_NOPROMPT true
    else
        set -gx BDM_THISBDM_NOPROMPT false
    end
    ########

    set -l old_bdmsys
    if test -n "$BDMSYS"
        set old_bdmsys $BDMSYS
    end

    set -l curr_filename (status --current-filename)
    set -gx BDMSYS (fish -c "cd (dirname $curr_filename)/..; and pwd"); or return 1
    
    if test -n "$old_bdmsys"; and test "$old_bdmsys" != "$BDMSYS"
        _bdm_warn "[WARN] You've already sourced another 'thisbdm' in your current shell session."
        _bdm_warn "       -> prev. installation='$old_bdmsys'"
        _bdm_warn "       -> this installation='$BDMSYS'"
        _bdm_warn "       You may encounter undefined behavior. It is recommended that you start"
        _bdm_warn "       a new shell session, or otherwise check if you automatically source"
        _bdm_warn "       'thisbdm' in one of your shell configuration files, e.g., 'config.fish'"
        _bdm_warn "       (this is no longer advised)."
    end

    # check if any config files naively source thisbdm
    set -l config_files "$__fish_config_dir/config.fish"
    for f in $config_files
       if test -f "$f"
            set -l source_pattern '^\s*(\.|source)\s+.*thisbdm\.(fish|sh).*'
            # one may append #IGNORE if match is a false positive, or they really want to keep that line
            set -l nr_matches (cat "$f" | grep -E "$source_pattern" | grep -vc '.*#IGNORE$')
            if test "$nr_matches" -gt '0'
                _bdm_warn "[WARN] You may have sourced thisbdm in '$f'."
                _bdm_warn "       Please check as this is not advised."
            end
        end
    end

    # Clear the env from previously set BioDynaMo paths.
    if test -n "$old_bdmsys"
        _drop_from_var PATH "$old_bdmsys/bin"
        _drop_from_var LD_LIBRARY_PATH "$old_bdmsys/lib"
        _drop_from_var DYLD_LIBRARY_PATH "$old_bdmsys/lib"
        _drop_from_var SHLIB_PATH "$old_bdmsys/lib"
        _drop_from_var MANPATH "$old_bdmsys/man"
        _drop_from_var CMAKE_PREFIX_PATH "$old_bdmsys"
    end

    # Clear the env from previously set PyEnv paths.
    set -l with_pyenv @with_pyenv@
    if test "$with_pyenv" = 'ON'
        if test -n "$old_bdmsys"
            if test -n "$PATH"
                _drop_from_var PATH "$PYENV_ROOT/bin"
                _drop_from_var PATH "$PYENV_ROOT/versions/@pythonvers@/bin"
                _drop_from_var PATH "$PYENV_ROOT/shims"
            end

            if test -n "$LD_LIBRARY_PATH"
                _drop_from_var LD_LIBRARY_PATH "$PYENV_ROOT/versions/$PYENV_VERSION/lib"
            end
        end
    end


    # If we run on macOS, we add the following exports for libomp support
    if test (uname) = 'Darwin'
        set BREWPREFIX (brew --prefix); or return 1
        if test -n "$CPPFLAGS"
          _drop_from_var CPPFLAGS "-I$BREWPREFIX/opt/libomp/include"
        end
        if test -n "$LDFLAGS"
          _drop_from_var LDFLAGS "-L$BREWPREFIX/opt/libomp/lib"
        end
        set -gx CPPFLAGS "-I$BREWPREFIX/opt/libomp/include $CPPFLAGS"
        set -gx LDFLAGS "-L$BREWPREFIX/opt/libomp/lib $LDFLAGS"
    end

    set -l with_paraview @with_paraview@

    set -l default_manpath
    if test -z "$MANPATH"
        # Grab the default man path before setting the path to avoid duplicates
        if type -qt manpath
            set default_manpath (manpath); or return 1
        else if type -qt man
            set default_manpath (man -w 2> /dev/null); or return 1
        end
    end

    if test -z "$PATH"
        set -gx PATH "$BDMSYS/bin"
    else
        set -gx PATH "$BDMSYS/bin:$PATH"
    end

    if test -z "$LD_LIBRARY_PATH"
        set -gx LD_LIBRARY_PATH "$BDMSYS/lib" # Linux, ELF HP-UX
    else
        set -pgx LD_LIBRARY_PATH "$BDMSYS/lib"
    end

    if test -z "$DYLD_LIBRARY_PATH"
        set -gx DYLD_LIBRARY_PATH "$BDMSYS/lib" # Mac OS X
    else
        set -pgx DYLD_LIBRARY_PATH "$BDMSYS/lib"
    end

    if test -z "$SHLIB_PATH"
        set -gx SHLIB_PATH "$BDMSYS/lib" # legacy HP-UX
    else
        set -pgx SHLIB_PATH "$BDMSYS/lib"
    end

    if test -z "$LIBPATH"
        set -gx LIBPATH "$BDMSYS/lib" # AIX
    else
        set -pgx LIBPATH "$BDMSYS/lib"
    end

    if test -z "$MANPATH"
        set -gx MANPATH "$BDMSYS/man":$default_manpath
    else
        set -pgx MANPATH "$BDMSYS/man"
    end

    ##### Python Specific Configurations #####
    if test "$with_pyenv" = 'ON'
        set -gx PYENV_ROOT @pyenvroot@
        if test -z "$PYENV_ROOT"
            set -gx PYENV_ROOT "$HOME/.pyenv"
        end

        set -pgx PATH "$PYENV_ROOT/bin"

        # Rehashing is not possible within a read-only Singularity container
        # and will cause sourcing of thisbdm to get stuck
        if test -n "$SINGULARITY_CONTAINER"
            set PYENV_NO_REHASH "--no-rehash"
        end

        pyenv init --path $PYENV_NO_REHASH | source; or return 1
        pyenv init - $PYENV_NO_REHASH | source; or return 1        
        pyenv shell @pythonvers@; or return 1

        # Location of jupyter executable (installed with `pip install` command)
        set -pgx PATH "$PYENV_ROOT/versions/@pythonvers@/bin"
        set -pgx LD_LIBRARY_PATH "$PYENV_ROOT/versions/@pythonvers@/lib"
    end
    ########

    ##### CMake Specific Configurations #####
    if test -z "$CMAKE_PREFIX_PATH"
        set -gx CMAKE_PREFIX_PATH "$BDMSYS/share/cmake"  # Linux, ELF HP-UX
    else
        set -pgx CMAKE_PREFIX_PATH "$BDMSYS/share/cmake"
    end
    ########

    #### ParaView Specific Configurations ####
    if test "$with_paraview" = 'ON'
        if test -z "$PV_PLUGIN_PATH"
            set -gx PV_PLUGIN_PATH "$BDMSYS/lib/pv_plugin"
        else
            set -pgx PV_PLUGIN_PATH "$BDMSYS/lib/pv_plugin"
        end
    end
    #######

    # OpenMP
    set -gx OMP_PROC_BIND true

    ###### Platform-specific Configuration
    # Apple specific
    if test (uname) = 'Darwin'
        # Nothing for now
        true
    else # GNU/Linux
        # CentOS specifics (no longer officially supported)
        set -l os_id (grep -oP '(?<=^ID=).+' /etc/os-release | tr -d '"'); or return 1
        if test "$os_id" = 'centos'
            set -gx MESA_GL_VERSION_OVERRIDE "3.3"
            if test -z "$CXX"; and test -z "$CC"
                . scl_source enable devtoolset-10; or return 1
            end

            . /etc/profile.d/modules.sh; or return 1
            module load mpi; or return 1

            # load llvm 6 required for libroadrunner
            if test -d "$BDMSYS"/third_party/libroadrunner
                . scl_source enable llvm-toolset-7; or return 1
            end
        end
    end
    #######

    ### Environment Indicator ###
    if not test "$BDM_THISBDM_NOPROMPT" = true
        set -gx __bdm_major_minor (biodynamo --shortversion)
        if not type -qt __bdm_fish_prompt_original
            functions --copy fish_prompt __bdm_fish_prompt_original
        end
        # NB: overrides prompt for current session only
        function fish_prompt
            echo "[bdm-$__bdm_major_minor] "(__bdm_fish_prompt_original)
        end
    end

    return 0
end

if source_thisbdm
    _bdm_ok "[OK] You have successfully sourced BioDynaMo's environment."
else
    _bdm_err "[ERR] BioDynaMo's environment could not be sourced."
end
_thisbdm_cleanup
