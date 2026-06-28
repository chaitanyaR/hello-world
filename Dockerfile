# ────────────────────────────────────────────────────────────────────────────
# AUTOSAR SOME/IP SDK – Multi-stage Dockerfile
#
# Three build targets:
#
#   builder  (internal)  Compiles the full project; not shipped directly.
#   runtime              Slim demo image – runs the 5-node vehicle network.
#   sdk                  Full developer image – build tools, pre-built libs,
#                        and example sources ready for experimentation.
#
# Build commands:
#   docker build --target runtime -t someip-vehicle:runtime .
#   docker build --target sdk     -t someip-vehicle:sdk     .
#
# Run the demo (single container, all 5 ECU nodes on loopback):
#   docker run --rm -it someip-vehicle:runtime
#
# Open a development shell:
#   docker run --rm -it -v "$(pwd):/workspace" someip-vehicle:sdk
# ────────────────────────────────────────────────────────────────────────────

ARG UBUNTU_VERSION=24.04

# ─── Stage 1: builder ────────────────────────────────────────────────────────
FROM ubuntu:${UBUNTU_VERSION} AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        git \
        ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/someip-sdk

# Copy entire repo so SOMEIP_ROOT (./someip) is available to the build
COPY . .

RUN cmake -S examples/VehicleNetwork \
          -B /build \
          -G Ninja \
          -DCMAKE_BUILD_TYPE=Release \
          -DSOMEIP_ROOT=/opt/someip-sdk/someip \
          -DVEHICLE_BUILD_MCAL_TC34XX=ON \
 && cmake --build /build --parallel "$(nproc)"

# ─── Stage 2: runtime (minimal demo image ~30 MB) ────────────────────────────
FROM ubuntu:${UBUNTU_VERSION} AS runtime

LABEL org.opencontainers.image.title="AUTOSAR SOME/IP Vehicle Network" \
      org.opencontainers.image.description="5-node virtual ECU SOME/IP SD demo (BCM/ECM/ADAS/Gateway/IPC)" \
      org.opencontainers.image.version="1.0.0"

# Non-root user for the demo
RUN useradd --create-home --shell /bin/bash someip

COPY --from=builder /build/bcm_node     /usr/local/bin/
COPY --from=builder /build/ecm_node     /usr/local/bin/
COPY --from=builder /build/adas_node    /usr/local/bin/
COPY --from=builder /build/gateway_node /usr/local/bin/
COPY --from=builder /build/ipc_node     /usr/local/bin/
COPY examples/VehicleNetwork/launch_network.sh /usr/local/bin/launch_network.sh
RUN chmod +x /usr/local/bin/launch_network.sh

WORKDIR /home/someip
USER someip

# Default: run all 5 nodes concurrently on the container's loopback
CMD ["bash", "/usr/local/bin/launch_network.sh", "/usr/local/bin"]

# ─── Stage 3: sdk (full developer environment ~500 MB) ───────────────────────
FROM builder AS sdk

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        gdb \
        valgrind \
        clang \
        clang-format \
        clang-tidy \
        cppcheck \
        vim \
        nano \
        bash-completion \
        less \
 && rm -rf /var/lib/apt/lists/*

# Make SOME/IP headers and pre-built binaries discoverable without extra cmake flags
ENV SOMEIP_ROOT=/opt/someip-sdk/someip
ENV VEHICLE_PREBUILT=/build
ENV PATH="/build:${PATH}"

# Mount point for user's own projects
WORKDIR /workspace

RUN printf '\n\
echo ""\n\
echo "AUTOSAR SOME/IP SDK ready."\n\
echo "  Sources     : /opt/someip-sdk"\n\
echo "  SOMEIP_ROOT : ${SOMEIP_ROOT}"\n\
echo "  Pre-built   : /build  (bcm_node ecm_node adas_node gateway_node ipc_node)"\n\
echo "  Your work   : /workspace  (mount your project here)"\n\
echo ""\n\
echo "Quick start:"\n\
echo "  cmake -S /opt/someip-sdk/examples/VehicleNetwork -B /tmp/build \\\\"\n\
echo "        -DSOMEIP_ROOT=\${SOMEIP_ROOT}"\n\
echo "  cmake --build /tmp/build --parallel"\n\
echo "  bash /opt/someip-sdk/examples/VehicleNetwork/launch_network.sh /tmp/build"\n\
echo ""\n\
' >> /etc/bash.bashrc

CMD ["bash"]
