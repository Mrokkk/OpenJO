#!/usr/bin/env bash

set -e

BASE_DIR="$(readlink -f "$(dirname "${0}")")"

. "${BASE_DIR}/scripts/utils.sh"

CMAKE=${CMAKE:-cmake}
BUILD_DIR="${BASE_DIR}/build"
INSTALL_DIR=""
TOOLCHAIN=""
COMMAND=""
ARCH=""

function print_help()
{
    echo "usage: $(basename "${0}") [options]... command"
    echo ""
    echo "options:"
    echo "    -d, --dbg"
    echo "          print every executed shell command"
    echo "    -i, --install-dir INSTALL_DIR"
    echo "          set install dir to INSTALL_DIR"
    echo "    -b, --build-type BUILD_TYPE"
    echo "          select type of build; possible values: Debug, Release, RelWithDebInfo, MinSizeRel"
    echo "    -t, --toolchain TOOLCHAIN"
    echo "          select toolchain; possible values: i686-w64-mingw32, linux-i686, x86_64-w64-mingw32"
    echo "    -h, --help"
    echo "          print help"
    echo ""
    echo "commands:"
    echo "    assets-update"
    echo "          update openjo.pk3"
    echo "    build"
    echo "          build OpenJO"
    echo "    install"
    echo "          update openjo.pk3, build, install OpenJO"
    echo "    run"
    echo "          update openjo.pk3, build, install and run OpenJO"
    exit 0
}

while [[ $# -gt 0 ]]; do
    case "${1}" in
        -d|--dbg)
            set -x
            ;;
        -i|--install-dir)
            shift
            INSTALL_DIR="${1}"
            ;;
        -b|--build-type)
            shift
            BUILD_TYPE="${1}"
            ;;
        -t|--toolchain)
            shift
            TOOLCHAIN="${1}"
            ;;
        -h|--help)
            print_help
            ;;
        build|install|run|update-assets)
            COMMAND="${1}"
            ;;
        *)
            die "Unsupported flag/command: ${1}"
            ;;
    esac
    shift
done

if [ ! -d "${BUILD_DIR}" ] || [ ! -f "${BUILD_DIR}/Makefile" ] || [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]
then
    mkdir -p "${BUILD_DIR}"
    pushd_silent "${BUILD_DIR}"

    [ -z "${INSTALL_DIR}" ] && die "No install dir given"

    declare -a cmake_args

    cmake_args+=( "-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}" )
    cmake_args+=( "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" )
    [ -n "${BUILD_TYPE}" ] && cmake_args+=( "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" )
    [ -n "${TOOLCHAIN}"  ] && cmake_args+=( "-DCMAKE_TOOLCHAIN_FILE=cmake/Toolchains/${TOOLCHAIN}.cmake" )

    ${CMAKE} "${cmake_args[@]}" "${BASE_DIR}"

    popd_silent
else
    INSTALL_DIR=$(sed -n -e "s/^CMAKE_INSTALL_PREFIX:PATH=//p" "${BUILD_DIR}/CMakeCache.txt")
    echo "Deduced install dir: ${INSTALL_DIR}"
fi

ARCH=$(sed -n -e "s/^ARCHITECTURE:STRING=//p" "${BUILD_DIR}/CMakeCache.txt")

[ -z "${ARCH}" ] && die "Unknown architecture"

function assets_update()
{
    declare -a assets_to_update

    pushd_silent assets

    for file in $(find . -type f)
    do
        if [ "${file}" -nt "${INSTALL_DIR}/base/openjo.pk3" ]
        then
            assets_to_update+=( "${file#./}" )
        fi
    done

    if [ ${#assets_to_update[@]} -ne 0 ]
    then
        zip -r "${INSTALL_DIR}/base/openjo.pk3" "${assets_to_update[@]}"
    fi

    popd_silent
}

function build()
{
    assets_update
    pushd_silent build
    make -j"$(nproc)"
    popd_silent
}

function install()
{
    build
    pushd_silent build
    make install -j"$(nproc)"
    popd_silent
}

function run()
{
    install
    "${INSTALL_DIR}/openjo_sp.${ARCH}"
}

case "${COMMAND}" in
    build)
        build
        ;;
    install)
        install
        ;;
    run)
        run
        ;;
    assets-update)
        assets_update
        ;;
    *)
        echo "No command to run"
esac
