BDM_ECHO_PURPLE='\033[95m'
BDM_ECHO_CYAN='\033[96m'
BDM_ECHO_DARKCYAN='\033[36m'
BDM_ECHO_BLUE='\033[94m'
BDM_ECHO_GREEN='\033[92m'
BDM_ECHO_YELLOW='\033[93m'
BDM_ECHO_RED='\033[91m'
BDM_ECHO_BOLD='\033[1m'
BDM_ECHO_UNDERLINE='\033[4m'
BDM_ECHO_RESET='\033[0m'

function EchoSuccess {
  echo -e "${BDM_ECHO_BOLD}${BDM_ECHO_GREEN}$@${BDM_ECHO_RESET}"
}

function EchoError {
  echo -e "${BDM_ECHO_BOLD}${BDM_ECHO_RED}$@${BDM_ECHO_RESET}"
}

function EchoWarning {
  echo -e "${BDM_ECHO_BOLD}${BDM_ECHO_YELLOW}$@${BDM_ECHO_RESET}"
}

function EchoNewStep {
  echo -e "${BDM_ECHO_BOLD}${BDM_ECHO_BLUE}$@${BDM_ECHO_RESET}"
}

function EchoInfo {
  echo -e "${BDM_ECHO_PURPLE}$@${BDM_ECHO_RESET}"
}

function EchoFinishInstallation {
  if [[ $# -ne 1 ]]; then
    echo "ERROR in EchoFinishInstallation: Wrong number of arguments"
    exit 1
  fi

  local addToConfigStr="   ${BDM_ECHO_UNDERLINE}echo \"alias thisbdm='source $1/bin/thisbdm"
  local setQuietEnvVarStr

  EchoInfo "Before running BioDynaMo execute:"
  case $SHELL in
    *bash | *zsh)
      EchoNewStep "   ${BDM_ECHO_UNDERLINE}source $1/bin/thisbdm.sh"

      case $SHELL in
        *bash) addToConfigStr="$addToConfigStr.sh'\" >> $(BashrcFile)" ;;
        *zsh)  addToConfigStr="$addToConfigStr.sh'\" >> $(ZshrcFile)" ;;
      esac
      setQuietEnvVarStr="'export BDM_THISBDM_QUIET=true'"
    ;;
    *fish)
      EchoNewStep "   ${BDM_ECHO_UNDERLINE}source $1/bin/thisbdm.fish"
      addToConfigStr="$addToConfigStr.fish'\" >> (fish -c 'echo "'$__fish_config_dir'"/config.fish')"
      setQuietEnvVarStr="'set -gx BDM_THISBDM_QUIET true'"
    ;;
    *)
      EchoNewStep "   ${BDM_ECHO_UNDERLINE}source $1/bin/thisbdm.{sh|fish}"
      echo
      EchoWarning "WARN: Your login shell appears to be '$SHELL'."
      EchoWarning "BioDynaMo currently supports the following shells:"
      EchoWarning "   ${BDM_ECHO_UNDERLINE}bash, zsh, and fish."
      return
    ;;
  esac

  EchoInfo "For added convenience, run (in your terminal):"
  EchoNewStep "$addToConfigStr"
  EchoInfo "to be able to just type 'thisbdm', instead of"
  EchoInfo "'source .../bin/thisbdm.[fi]sh'"
  echo
  EchoInfo "${setQuietEnvVarStr} will disable the prompt indicator,"
  EchoInfo "and silence all non-critical output."
  echo
  EchoNewStep "NOTE: Your login shell appears to be '$SHELL'."
  EchoNewStep "The instructions above are for this shell."
  EchoNewStep "For other shells, or for more information, see:"
  EchoNewStep "   ${BDM_ECHO_UNDERLINE}https://biodynamo.org/docs/userguide/first_steps/"
  echo
  # any warnings about users' post-install shell
  # configuration may be added to the function called below
  WarnPossibleBadShellConfigs || true
}

function CPUCount {
  if [ `uname` = "Darwin" ]; then
    sysctl -n hw.ncpu
  else
    grep -c ^processor /proc/cpuinfo
  fi
}

function CleanBuild {
  if [[ $# -ne 1 ]]; then
    echo "ERROR in CleanBuild: Wrong number of arguments"
    exit 1
  fi

  local BUILD_DIR=$1

  if [ -d "${BUILD_DIR}" ]; then
    rm -rf $BUILD_DIR/*
  else
    mkdir -p $BUILD_DIR
  fi
  cd $BUILD_DIR
  BDM_CMAKE_FLAGS=$BDM_CMAKE_FLAGS+" -DParaview=ON"
  echo "CMAKEFLAGS $BDM_CMAKE_FLAGS"
  cmake $BDM_CMAKE_FLAGS ..
  make -j$(CPUCount) && make install
}


#Load Modules
echo "Loading Modules:"
module use /share/apps/eb/modules/all/
module load Tkinter/3.9.6-GCCcore-11.2.0
module load git/2.33.1-GCCcore-11.2.0-nodocs
module load Bazel/4.2.2-GCCcore-11.2.0
module load OpenBLAS/0.3.18-GCC-11.2.0
module load CMake/3.22.1-GCCcore-11.2.0
module load OpenMPI/4.1.1-GCC-11.2.0
module load Doxygen/1.9.1-GCCcore-11.2.0
#module load Mesa/22.0.3-GCCcore-11.3.0
#module load glew/2.2.0-GCCcore-11.3.0-egl
#module load libglvnd/1.4.0-GCCcore-11.3.0
module load freeglut/3.2.1-GCCcore-11.2.0
module load libGLU/9.0.2-GCCcore-11.2.0
module load LLVM/12.0.1-GCCcore-11.2.0
module unload Python

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

rm -rf $BDM_PROJECT_DIR/openblas-fake-lib
mkdir $BDM_PROJECT_DIR/openblas-fake-lib
cd $BDM_PROJECT_DIR/openblas-fake-lib
ln -s $EBROOTOPENBLAS/lib/libopenblas.so libblas.so
ln -s $EBROOTOPENBLAS/lib/libopenblas.so liblapack.so
cd ..
export LIBRARY_PATH=$BDM_PROJECT_DIR/openblas-fake-lib:$LIBRARY_PATH
export LD_LIBRARY_PATH=$BDM_PROJECT_DIR/openblas-fake-lib:$LD_LIBRARY_PATH


export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init --path)"
eval "$(pyenv init -)"
pyenv install 3.9.1
pyenv shell 3.9.1
set -e

BUILD_DIR=$BDM_PROJECT_DIR/build

cd $BDM_PROJECT_DIR/util/build-third-party/
./build-gcc_new.sh $BDM_PROJECT_DIR
cd $BUILD_DIR
if [ -d $BDM_PROJECT_DIR_2/third_party/gcc ]; then

        echo "******************************************************************"
        echo "Found suitable gcc version installed in the
third party directory.
Using that."
        echo "******************************************************************"
        export CC=$BDM_PROJECT_DIR_2/third_party/gcc/bin/gcc
        export CXX=$BDM_PROJECT_DIR_2/third_party/gcc/bin/g++
        export FC=$BDM_PROJECT_DIR_2/third_party/gcc/bin/gfortran
        export OMPI_CC=$BDM_PROJECT_DIR_2/third_party/gcc/bin/gcc
        export OMPI_CXX=$BDM_PROJECT_DIR_2/third_party/gcc/bin/g++
        export OMPI_FC=$BDM_PROJECT_DIR_2/third_party/gcc/bin/gfortran
	export QMAKE_CC=$BDM_PROJECT_DIR_2/third_party/gcc/bin/gcc
        export QMAKE_CXX=$BDM_PROJECT_DIR_2/third_party/gcc/bin/g++
        export LINK=$BDM_PROJECT_DIR_2/third_party/gcc/bin/g++

fi

CleanBuild $BUILD_DIR

# print final steps
echo
EchoSuccess "Installation of BioDynaMo finished successfully!"

BDM_VERSION=$(cat $BUILD_DIR/version/bdm_shortversion)
INSTALL_DIR=${HOME}/biodynamo-v${BDM_VERSION}

EchoFinishInstallation $INSTALL_DIR
