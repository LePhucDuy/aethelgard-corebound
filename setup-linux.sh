#!/usr/bin/env bash
# =============================================================================
# Aethelgard: Corebound - Chuẩn bị môi trường build trên Linux
# Tải và build raylib 5.5 + GLFW 3.4 (static) vào lib/ để `make` dùng được
# mà không cần cài raylib vào hệ thống.
#
# Cách dùng:
#   ./setup-linux.sh            # tất cả các bước (bỏ qua nếu đã build sẵn)
#   ./setup-linux.sh --force    # build lại từ đầu
#   ./setup-linux.sh download   # chỉ tải + giải nén nguồn
#   ./setup-linux.sh glfw       # chỉ build GLFW
#   ./setup-linux.sh raylib     # chỉ build raylib
# =============================================================================
set -euo pipefail

RAYLIB_VERSION="5.5"
GLFW_VERSION="3.4"
PROJ_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIB_DIR="${PROJ_DIR}/lib"
WORK_DIR="/tmp/aethelgard-deps"

FORCE=0
STEP="all"
for arg in "$@"; do
    case "$arg" in
        --force) FORCE=1 ;;
        download|glfw|raylib) STEP="$arg" ;;
        *) echo "Tham so khong hop le: $arg"; exit 1 ;;
    esac
done

log() { echo "[setup-linux] $*"; }

# -----------------------------------------------------------------------------
# 0. Kiem tra cong cu va thu vien he thong can thiet
# -----------------------------------------------------------------------------
check_deps() {
    local missing=()
    for cmd in g++ gcc make ar curl; do
        command -v "$cmd" >/dev/null 2>&1 || missing+=("$cmd")
    done
    if [ ! -f /usr/include/X11/Xlib.h ]; then
        missing+=("X11 dev headers (libx11-dev / libX11-devel)")
    fi
    if [ ! -f /usr/include/GL/gl.h ]; then
        missing+=("OpenGL dev headers (libgl1-mesa-dev / mesa-libGL-devel)")
    fi
    if [ ${#missing[@]} -ne 0 ]; then
        echo "[setup-linux][ERROR] Thieu cac thanh phan:"
        for m in "${missing[@]}"; do echo "  - $m"; done
        echo ""
        echo "Cai dat theo distro cua ban:"
        echo "  Ubuntu/Debian: sudo apt install -y build-essential curl libx11-dev libgl1-mesa-dev"
        echo "  Fedora:        sudo dnf install -y gcc-c++ make curl mesa-libGL-devel libX11-devel"
        exit 1
    fi
}

# -----------------------------------------------------------------------------
# 1. Tai va giai nen nguon
# -----------------------------------------------------------------------------
do_download() {
    if [ "$FORCE" -eq 0 ] && [ -d "${WORK_DIR}/raylib-${RAYLIB_VERSION}" ] \
        && [ -d "${WORK_DIR}/glfw-${GLFW_VERSION}" ]; then
        log "Nguon da ton tai tai ${WORK_DIR}, bo qua buoc tai (dung --force de tai lai)."
        return 0
    fi
    mkdir -p "${WORK_DIR}"
    log "Tai raylib ${RAYLIB_VERSION}..."
    curl -fL --retry 3 -o "${WORK_DIR}/raylib.tar.gz" \
        "https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz"
    log "Tai GLFW ${GLFW_VERSION}..."
    curl -fL --retry 3 -o "${WORK_DIR}/glfw.tar.gz" \
        "https://github.com/glfw/glfw/archive/refs/tags/${GLFW_VERSION}.tar.gz"
    log "Giai nen..."
    rm -rf "${WORK_DIR}/raylib-${RAYLIB_VERSION}" "${WORK_DIR}/glfw-${GLFW_VERSION}"
    tar -xzf "${WORK_DIR}/raylib.tar.gz" -C "${WORK_DIR}"
    tar -xzf "${WORK_DIR}/glfw.tar.gz" -C "${WORK_DIR}"
    log "Tai nguon hoan tat."
}

# -----------------------------------------------------------------------------
# 2. Build GLFW static (X11)
# -----------------------------------------------------------------------------
do_glfw() {
    local glfw_src="${WORK_DIR}/glfw-${GLFW_VERSION}"
    [ -d "$glfw_src" ] || { echo "[setup-linux][ERROR] Chua co nguon GLFW - chay: ./setup-linux.sh download"; exit 1; }

    if [ "$FORCE" -eq 0 ] && [ -f "${LIB_DIR}/libglfw3.a" ]; then
        log "lib/libglfw3.a da ton tai, bo qua. (dung --force de build lai)"
        return 0
    fi

    log "Build GLFW ${GLFW_VERSION} (static, X11)..."
    cd "$glfw_src"
    rm -f ./*.o libglfw3.a
    gcc -c src/context.c src/init.c src/input.c src/monitor.c src/platform.c \
        src/posix_module.c src/posix_poll.c src/posix_thread.c src/posix_time.c \
        src/vulkan.c src/egl_context.c src/glx_context.c src/linux_joystick.c \
        src/null_init.c src/null_joystick.c src/null_monitor.c src/null_window.c \
        src/osmesa_context.c src/window.c src/x11_init.c src/x11_monitor.c \
        src/x11_window.c src/xkb_unicode.c \
        -O2 -D_GLFW_X11 -Iinclude -Isrc
    ar rcs libglfw3.a ./*.o
    mkdir -p "${LIB_DIR}"
    cp -f libglfw3.a "${LIB_DIR}/"
    log "GLFW build xong -> lib/libglfw3.a"
}

# -----------------------------------------------------------------------------
# 3. Build raylib static (PLATFORM_DESKTOP + OpenGL 3.3), link voi GLFW vua build
# -----------------------------------------------------------------------------
do_raylib() {
    local raylib_src="${WORK_DIR}/raylib-${RAYLIB_VERSION}"
    [ -d "$raylib_src" ] || { echo "[setup-linux][ERROR] Chua co nguon raylib - chay: ./setup-linux.sh download"; exit 1; }
    [ -f "${LIB_DIR}/libglfw3.a" ] || { echo "[setup-linux][ERROR] Chua co lib/libglfw3.a - chay: ./setup-linux.sh glfw"; exit 1; }

    if [ "$FORCE" -eq 0 ] && [ -f "${LIB_DIR}/libraylib.a" ]; then
        log "lib/libraylib.a da ton tai, bo qua. (dung --force de build lai)"
        return 0
    fi

    log "Build raylib ${RAYLIB_VERSION} (static, OpenGL 3.3)..."
    cd "$raylib_src/src"
    rm -f ./*.o libraylib.a
    # Luu y: KHONG them -Iexternal (de dung dirent.h cua he thong, khong phai ban Windows)
    gcc -c rcore.c rshapes.c rtextures.c rtext.c rmodels.c raudio.c utils.c \
        -O2 -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -D_GNU_SOURCE \
        -fno-strict-aliasing -std=c99 \
        -I. -I"${WORK_DIR}/glfw-${GLFW_VERSION}" -I"${WORK_DIR}/glfw-${GLFW_VERSION}/include"
    ar rcs libraylib.a rcore.o rshapes.o rtextures.o rtext.o rmodels.o raudio.o utils.o
    mkdir -p "${LIB_DIR}/include"
    cp -f libraylib.a "${LIB_DIR}/"
    cp -f raylib.h "${LIB_DIR}/include/"
    log "raylib build xong -> lib/libraylib.a + lib/include/raylib.h"
}

# -----------------------------------------------------------------------------
check_deps
mkdir -p "${LIB_DIR}/include"

case "$STEP" in
    download) do_download ;;
    glfw)     do_download; do_glfw ;;
    raylib)   do_download; do_raylib ;;
    all)      do_download; do_glfw; do_raylib ;;
esac

log "Hoan tat! Gio ban co the chay:  make  roi  ./bin/aethelgard"
