#!/bin/sh
# Build Enhanced PS2 Disc Launcher on any platform.
#
#   ./build.sh          build disc-launcher.elf
#   ./build.sh clean    remove build output
#   ./build.sh shell    open a shell in the toolchain environment
#
# Uses a native ps2dev toolchain when one is installed, otherwise falls back to
# the ps2dev container (built from Dockerfile.dev).
set -eu

IMAGE=enhanced-ps2-disc-launcher-dev
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

# --- native toolchain ---------------------------------------------------------
if [ -n "${PS2SDK:-}" ] && command -v mips64r5900el-ps2-elf-gcc >/dev/null 2>&1; then
    cd "$ROOT"
    if [ "${1:-}" = "shell" ]; then
        exec "${SHELL:-sh}"
    fi
    exec make "$@"
fi

# --- container ---------------------------------------------------------------
if command -v docker >/dev/null 2>&1; then
    ENGINE=docker
elif command -v podman >/dev/null 2>&1; then
    ENGINE=podman
else
    cat >&2 <<'EOF'
No PS2 toolchain and no container engine found.

Pick one:
  * Install Docker (https://docs.docker.com/get-started/get-docker/) and re-run
    this script -- the ps2dev image runs natively on both arm64 and amd64.
  * Or install the toolchain natively: https://github.com/ps2dev/ps2toolchain
    then run `make` directly.
EOF
    exit 1
fi

if ! $ENGINE image inspect "$IMAGE" >/dev/null 2>&1; then
    echo "Building $IMAGE (first run only)..."
    $ENGINE build -t "$IMAGE" -f "$ROOT/Dockerfile.dev" "$ROOT"
fi

# Keep build output owned by the current user (matters on Linux hosts).
USERFLAG=""
if [ "$ENGINE" = docker ] && command -v id >/dev/null 2>&1; then
    USERFLAG="-u $(id -u):$(id -g)"
fi

if [ "${1:-}" = "shell" ]; then
    # shellcheck disable=SC2086
    exec $ENGINE run --rm -it $USERFLAG -v "$ROOT:/src" -w /src "$IMAGE" sh
fi

# shellcheck disable=SC2086
exec $ENGINE run --rm $USERFLAG -v "$ROOT:/src" -w /src "$IMAGE" make "$@"
