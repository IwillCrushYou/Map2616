#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EMSDK_DIR="${HOME}/emsdk"
EMSDK_VERSION="${EMSDK_VERSION:-latest}"

if [[ ! -d "${EMSDK_DIR}" ]]; then
  git clone --depth 1 https://github.com/emscripten-core/emsdk.git "${EMSDK_DIR}"
fi

cd "${EMSDK_DIR}"

if [[ ! -x "${EMSDK_DIR}/upstream/emscripten/emcc" ]]; then
  ./emsdk install "${EMSDK_VERSION}"
  ./emsdk activate "${EMSDK_VERSION}"
fi

# shellcheck disable=SC1091
source "${EMSDK_DIR}/emsdk_env.sh"

cd "${REPO_ROOT}"
emcmake cmake -S . -B build-wasm
cmake --build build-wasm -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"

cd frontend/client
npm ci
npm run build
