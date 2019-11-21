#!/bin/bash


function show_help()
{
    echo "usage: impair.sh net_if delay_ms variance_ms [--help]"
}


echo "================================================================================"
echo "Linux Network Impairment Tool"
echo "Copyright (c) 2019 Rally Tactical Systems, Inc."
echo "================================================================================"

# Process the command-line
for ARG in "$@"
do
    if [[ "$ARG" == "--help" ]]; then
        show_help
        exit 1

    elif [[ "$ARG" == "-debugcmake" ]]; then
        CMAKE_EXTRA_DEFINES="${CMAKE_EXTRA_DEFINES} -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"
    
    elif [[ "$ARG" == "-teprofiling" ]]; then
        CMAKE_EXTRA_DEFINES="${CMAKE_EXTRA_DEFINES} -DRTS_TE_ATT=ON"

    elif [[ "$ARG" == "-debug" ]]; then
        CMAKE_EXTRA_DEFINES="${CMAKE_EXTRA_DEFINES} -DRTS_DEBUG=ON"

    elif [[ "$ARG" == "-clearcache" ]]; then
        CLEAR_CMAKE_CACHE=1

    elif [[ "$ARG" == "-sanitize" ]]; then
        #if [[ "$OSTYPE" != "linux"* ]]; then
        #    echo "ERROR: sanitization is not available on non-linux platforms"
        #    show_help
        #    exit 1
        #fi
        CMAKE_EXTRA_DEFINES="${CMAKE_EXTRA_DEFINES} -DRTS_SANITIZE_BUILD=ON"


    elif [[ "$ARG" == "-host" ]]; then
        BUILD_HOST=1
        RTS_BUILD_ROOT="${RTS_BUILD_BASE}/${RTS_OS}_${RTS_BITS_DIR}"
        RTS_PREBUILT_ROOT="${RTS_BUILD_ROOT}/prebuilt"
        RTS_TARGET_PLATFORM=${RTS_OS}_${RTS_BITS_DIR}
        RTS_TOOLCHAIN_FILE=${RTS_ROOT}/host.toolchain.cmake
        
        RTS_OPEN_SSL_PLATFORM=
        RTS_CROSSCOMP_DIR=
        OPENSSL_DIR=${RTS_PREBUILT_ROOT}/libopenssl
        RTS_OPENSSL_CONFIGURE_EXTRA="--prefix=${OPENSSL_DIR} --openssldir=${OPENSSL_DIR}"


    elif [[ "$ARG" == "-rpi32" ]]; then
        BUILD_PI=1
        RTS_BUILD_ROOT="${RTS_BUILD_BASE}/linux_rpi32_arm32"
        RTS_PREBUILT_ROOT="${RTS_BUILD_ROOT}/prebuilt"
        RTS_TARGET_PLATFORM=raspberry_pi_linux_arm32
        RTS_TOOLCHAIN_FILE=${RTS_ROOT}/rpi32.toolchain.cmake

        RTS_OPEN_SSL_PLATFORM=linux-generic32
        RTS_CROSSCOMP_DIR=${RTS_BUILD_TOOLS_ROOT}/pi/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin
        OPENSSL_DIR=${RTS_PREBUILT_ROOT}/libopenssl
        RTS_OPENSSL_CONFIGURE_EXTRA="--prefix=${OPENSSL_DIR} --openssldir=${OPENSSL_DIR} --cross-compile-prefix=${RTS_CROSSCOMP_DIR}/arm-linux-gnueabihf- -pthread"


    elif [[ "$ARG" == -android* ]]; then
        BUILD_ANDROID=1


    elif [[ "$ARG" == -publish ]]; then
        PUBLISH=1


    else
        echo "ERROR: Unknown option \"$ARG\""
        show_help
        exit 1
    fi
done
