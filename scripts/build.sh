#!/usr/bin/env bash

set -euo pipefail

preset="linux-ninja-debug"
jobs=""
run_tests="true"
run_install="false"
qt_root=""
fresh="false"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --preset)
            preset="$2"
            shift 2
            ;;
        --jobs)
            jobs="$2"
            shift 2
            ;;
        --skip-tests)
            run_tests="false"
            shift
            ;;
        --install)
            run_install="true"
            shift
            ;;
        --fresh)
            fresh="true"
            shift
            ;;
        --qt-root)
            qt_root="$2"
            shift 2
            ;;
        *)
            echo "Unknown argument: $1" >&2
            exit 1
            ;;
    esac
done

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if ! command -v cmake >/dev/null 2>&1; then
    echo "cmake was not found in PATH." >&2
    exit 1
fi

if ! command -v ctest >/dev/null 2>&1; then
    echo "ctest was not found in PATH." >&2
    exit 1
fi

if [[ -n "$qt_root" ]]; then
    export QT_ROOT_DIR="$qt_root"
    export CMAKE_PREFIX_PATH="$qt_root${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
    export PATH="$qt_root/bin:$PATH"
fi

cd "$repo_root"

if [[ "$fresh" == "true" ]]; then
    echo
    echo "> rm -rf $repo_root/build/$preset"
    rm -rf "$repo_root/build/$preset"
fi

echo
echo "> cmake --preset $preset"
cmake --preset "$preset"

build_args=(--build --preset "$preset")
if [[ -n "$jobs" ]]; then
    build_args+=(--parallel "$jobs")
else
    build_args+=(--parallel)
fi

echo
echo "> cmake ${build_args[*]}"
cmake "${build_args[@]}"

if [[ "$run_tests" == "true" ]]; then
    echo
    echo "> ctest --preset $preset"
    ctest --preset "$preset"
fi

if [[ "$run_install" == "true" ]]; then
    install_args=(--install "$repo_root/build/$preset")
    case "$preset" in
        windows-msvc-debug)
            install_args+=(--config Debug)
            ;;
        windows-msvc-release)
            install_args+=(--config Release)
            ;;
    esac

    echo
    echo "> cmake ${install_args[*]}"
    cmake "${install_args[@]}"
fi
