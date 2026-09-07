Bootstrap: docker
From: ubuntu:24.04

%files
  . /biodynamo

%post -c /bin/bash
  set -eux
  export DEBIAN_FRONTEND=noninteractive
  apt-get update
  apt-get install -y --no-install-recommends \
    ca-certificates cmake g++ gcc git libblas-dev libboost-dev liblapack-dev \
    libnuma-dev libomp-dev libopenmpi-dev ninja-build openmpi-bin python3
  rm -rf /var/lib/apt/lists/*
  cmake -S /biodynamo -B /biodynamo/build -G Ninja \
    -Dbenchmark=OFF \
    -Dcuda=OFF \
    -Dlibgit2=OFF \
    -Dopencl=OFF \
    -Dparaview=OFF \
    -Dsbml=OFF \
    -Dtest=OFF \
    -Dvalgrind=OFF \
    -DCMAKE_BUILD_TYPE=Release
  cmake --build /biodynamo/build --parallel

%environment
  export BDMSYS=/biodynamo/build
  export PATH=/biodynamo/build/bin:$PATH
  export LD_LIBRARY_PATH=/biodynamo/build/lib:$LD_LIBRARY_PATH

%runscript
  exec /bin/bash "$@"
