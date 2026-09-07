FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Berlin
ENV DISPLAY=:1
ENV XDG_RUNTIME_DIR=/tmp/runtime-root

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      ca-certificates cmake curl fluxbox g++ gcc git libblas-dev libboost-dev \
      libdouble-conversion-dev liblapack-dev libnuma-dev libomp-dev \
      libopenmpi-dev libutfcpp-dev ninja-build novnc openmpi-bin paraview \
      paraview-dev python3 sudo websockify wget x11-utils x11vnc \
      x11-xserver-utils xterm xvfb && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

RUN git config --system user.name "Test User" && \
    git config --system user.email user@test.com

RUN git clone --depth 1 --branch master https://github.com/BioDynaMo/biodynamo.git /biodynamo && \
    cmake -S /biodynamo -B /biodynamo/build -G Ninja \
      -Dbenchmark=OFF \
      -Dcuda=OFF \
      -Dlibgit2=OFF \
      -Dopencl=OFF \
      -Dparaview=ON \
      -Dsbml=OFF \
      -Dtest=OFF \
      -Dvalgrind=OFF \
      -DCMAKE_BUILD_TYPE=Release && \
    cmake --build /biodynamo/build --parallel

RUN useradd -m -s /bin/bash codespace && \
    echo "codespace ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers && \
    mkdir -p /tmp/runtime-root /opt/scripts && \
    chmod 700 /tmp/runtime-root

RUN echo 'source /biodynamo/build/bin/thisbdm.sh 2>/dev/null' >> /etc/bash.bashrc

COPY novnc-index.html /usr/share/novnc/index.html
COPY start-vnc.sh demo-picker.sh /opt/scripts/
RUN chmod +x /opt/scripts/start-vnc.sh /opt/scripts/demo-picker.sh

COPY paraview-wrapper.sh /usr/local/bin/paraview
RUN chmod +x /usr/local/bin/paraview
