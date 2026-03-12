#!/usr/bin/env bash
# Build ROOT 6.38.02 tarball for BioDynaMo
#
# Produces a self-contained tarball with builtin dependencies so
# the result is portable across machines of the same OS/arch.
#
# Usage:
#   ./util/build_root638.sh [--jobs N] [--prefix /path/to/install]
#
# Output:
#   root_v6.38.02_cxx17_python3.9_<os>.tar.gz   (in working directory)
#   Prints the SHA256 digest at the end for SHA256Digests.cmake
#
# Requirements (Ubuntu):
#   sudo apt-get install -y git cmake g++ gcc gfortran \
#       libx11-dev libxext-dev libxft-dev libxpm-dev \
#       libssl-dev python3-dev python3-numpy

set -euo pipefail

ROOT_VERSION="6.38.02"
ROOT_TAG="v6-38-02"
JOBS="${1:---jobs}"
if [[ "$JOBS" == "--jobs" ]]; then
  shift || true
  JOBS="${1:-$(nproc)}"
  shift || true
else
  JOBS="$(nproc)"
fi

PREFIX="${1:-$(pwd)/root_install}"

# Detect OS for tarball naming (mirrors BioDynaMo cmake/utils.cmake detect_os)
detect_os() {
  if [[ "$(uname)" == "Darwin" ]]; then
    local ver
    ver=$(sw_vers -productVersion | cut -d. -f1-2)
    local arch
    arch=$(uname -m)
    [[ "$arch" == "x86_64" ]] && arch="i386"
    echo "osx-${ver}-${arch}"
  else
    if [[ -f /etc/os-release ]]; then
      local id ver
      id=$(. /etc/os-release && echo "$ID")
      ver=$(. /etc/os-release && echo "$VERSION_ID")
      local arch
      arch=$(uname -m)
      if [[ "$arch" == "aarch64" ]]; then
        echo "${id}-${ver}-${arch}"
      else
        echo "${id}-${ver}"
      fi
    else
      echo "unknown"
    fi
  fi
}

OS_TAG=$(detect_os)
TAR_NAME="root_v${ROOT_VERSION}_cxx17_python3.9_${OS_TAG}.tar.gz"

echo "=== Building ROOT ${ROOT_VERSION} for BioDynaMo ==="
echo "  OS tag:   ${OS_TAG}"
echo "  Tarball:  ${TAR_NAME}"
echo "  Jobs:     ${JOBS}"
echo "  Install:  ${PREFIX}"
echo ""

SRCDIR="$(pwd)/root_src"
BUILDDIR="$(pwd)/root_build_bdm"

# Clone source if needed
if [[ ! -d "$SRCDIR" ]]; then
  echo ">>> Cloning ROOT ${ROOT_TAG} ..."
  git clone --depth 1 --branch "${ROOT_TAG}" \
    https://github.com/root-project/root.git "$SRCDIR"
else
  echo ">>> Source dir exists, skipping clone"
fi

mkdir -p "$BUILDDIR"
cd "$BUILDDIR"

# Configure with builtin dependencies for portability (matching BDM 6.30 style)
# Key: builtin everything that BDM's current ROOT builds in, disable what BDM
# does not use, and enable cxx17.
echo ">>> Configuring ..."
cmake "$SRCDIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DCMAKE_CXX_STANDARD=17 \
  \
  -Dbuiltin_clang=ON \
  -Dbuiltin_cling=ON \
  -Dbuiltin_llvm=ON \
  -Dbuiltin_openui5=ON \
  -Dbuiltin_davix=ON \
  -Dbuiltin_fftw3=ON \
  -Dbuiltin_freetype=ON \
  -Dbuiltin_ftgl=ON \
  -Dbuiltin_gl2ps=ON \
  -Dbuiltin_glew=ON \
  -Dbuiltin_gsl=ON \
  -Dbuiltin_lz4=ON \
  -Dbuiltin_lzma=ON \
  -Dbuiltin_nlohmannjson=ON \
  -Dbuiltin_pcre=ON \
  -Dbuiltin_tbb=ON \
  -Dbuiltin_unuran=ON \
  -Dbuiltin_vdt=ON \
  -Dbuiltin_xrootd=ON \
  -Dbuiltin_xxhash=ON \
  -Dbuiltin_zlib=ON \
  -Dbuiltin_zstd=ON \
  -Dbuiltin_afterimage=ON \
  \
  -Dclad=ON \
  -Ddataframe=ON \
  -Dfftw3=ON \
  -Dgdml=ON \
  -Dhttp=ON \
  -Dimt=ON \
  -Dmathmore=ON \
  -Dopengl=ON \
  -Dpyroot=ON \
  -Droofit=ON \
  -Dwebgui=ON \
  -Droot7=ON \
  -Druntime_cxxmodules=ON \
  -Dspectrum=ON \
  -Dsqlite=ON \
  -Dssl=ON \
  -Dtmva=ON \
  -Dtmva-cpu=ON \
  -Dvdt=ON \
  -Dx11=ON \
  -Dxml=ON \
  -Dxrootd=ON \
  \
  -Dtesting=OFF \
  -Droottest=OFF \
  -Dminimal=OFF

echo ">>> Building (${JOBS} jobs) ..."
cmake --build . -j"${JOBS}"

echo ">>> Installing to ${PREFIX} ..."
cmake --install .

# Package tarball
cd "$(dirname "$PREFIX")"
INSTALL_DIRNAME="$(basename "$PREFIX")"

echo ">>> Creating tarball ${TAR_NAME} ..."
tar czf "$(pwd)/${TAR_NAME}" \
  --transform="s/^${INSTALL_DIRNAME}/root/" \
  "${INSTALL_DIRNAME}"

SHA=$(sha256sum "$(pwd)/${TAR_NAME}" | awk '{print $1}')

echo ""
echo "=== Done ==="
echo "  Tarball: $(pwd)/${TAR_NAME}"
echo "  Size:    $(du -h "$(pwd)/${TAR_NAME}" | cut -f1)"
echo "  SHA256:  ${SHA}"
echo ""
echo "Add to SHA256Digests.cmake:"
echo "  SET(${OS_TAG}-ROOT ${SHA})"
echo ""
echo "Update ROOT.cmake line 6:"
echo "  set(ROOT_TAR_FILE root_v${ROOT_VERSION}_cxx17_python3.9_\${DETECTED_OS_VERS}.tar.gz)"
