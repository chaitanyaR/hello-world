# ── Stage 1: deps ────────────────────────────────────────────────────────────
# Install all build and documentation dependencies once.
FROM ubuntu:24.04 AS deps

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        cmake \
        ninja-build \
        gcc \
        g++ \
        libgtest-dev \
        nlohmann-json3-dev \
        doxygen \
        graphviz \
        python3 \
    && rm -rf /var/lib/apt/lists/*

# ── Stage 2: builder ─────────────────────────────────────────────────────────
# Copy source, configure with the debug preset (simulation mode ON,
# coverage OFF for speed), build all targets, and run unit tests.
FROM deps AS builder

WORKDIR /workspace
COPY . .

RUN cmake --preset=debug \
    && cmake --build build/debug --parallel "$(nproc)"

RUN ctest --preset=unit --output-on-failure

# ── Stage 3: docs-gen ────────────────────────────────────────────────────────
# Generate Doxygen HTML documentation from headers + architecture.md.
# Output lands in build/docs/html/ (set by docs/Doxyfile).
FROM builder AS docs-gen

RUN cmake --build build/debug --target docs

# ── Stage 4: release-build ───────────────────────────────────────────────────
# Build optimised release binaries (no tests, no debug info).
FROM deps AS release-build

WORKDIR /workspace
COPY . .

RUN cmake --preset=release \
    && cmake --build build/release --parallel "$(nproc)"

# ── Stage 5: final ───────────────────────────────────────────────────────────
# Minimal runtime image:
#   /app/docs/  — Doxygen HTML site (served on port 8080)
#   /app/bin/   — Release executables (sdv_hpc, sdv_powertrain, …)
#
# Run:  docker run -p 8080:8080 sdv-platform
# Then: open http://localhost:8080
FROM ubuntu:24.04 AS final

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        python3 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=docs-gen    /workspace/build/docs/html  /app/docs
COPY --from=release-build /workspace/build/release/src /app/bin

EXPOSE 8080

LABEL org.opencontainers.image.title="SDV HPC + Zonal ECU Platform" \
      org.opencontainers.image.description="Build + documentation image for the SDV platform" \
      org.opencontainers.image.version="0.1.0"

ENTRYPOINT ["python3", "-m", "http.server", "8080", "--directory", "/app/docs"]
