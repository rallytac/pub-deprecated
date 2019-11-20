#!/bin/bash

echo "================================================================================"
echo "Engage NPM Publisher"
echo "Copyright (c) 2019 Rally Tactical Systems, Inc."
echo "================================================================================"

THIS_ROOT=`pwd`
BUILD_ROOT=${THIS_ROOT}/.build
API_ROOT=${THIS_ROOT}/../
BIN_ROOT=${THIS_ROOT}/../../bin
SRC_BIN_VERSION="${1}"
VERSION_EXTENSION="${2}"

if [ "${SRC_BIN_VERSION}" == "" -o "${VERSION_EXTENSION}" == "" ]; then
    echo "usage ${0} src_version pub_extension"
    exit 1
fi

#function determine_src_bin_version()
#{
#    FILES=(`ls -r ${BIN_ROOT}`)
#    SRC_BIN_VERSION=${FILES[0]}
#}

function publish_it()
{
    CURRDIR=`pwd`

    rm -rf ${BUILD_ROOT}
    mkdir ${BUILD_ROOT}
    cd ${BUILD_ROOT}

    cp ../binding.gyp .
    cp ../engage.cpp .
    cp ../index.js .
    cp ../package.json .
    cp ../README.md .    

    mkdir include
    cp ${API_ROOT}/c/include/* include

    mkdir -p lib/darwin.x64
    cp ${BIN_ROOT}/${SRC_BIN_VERSION}/darwin_x64/libengage-shared.dylib lib/darwin.x64

    mkdir -p lib/linux.x64
    cp ${BIN_ROOT}/${SRC_BIN_VERSION}/linux_x64/libengage-shared.so lib/linux.x64

    mkdir -p lib/win32.x64
    cp ${BIN_ROOT}/${SRC_BIN_VERSION}/win_x64/engage-shared.dll lib/win32.x64
    cp ${BIN_ROOT}/${SRC_BIN_VERSION}/win_x64/engage-shared.lib lib/win32.x64

    mkdir -p lib/win32.ia32
    cp ${BIN_ROOT}/${SRC_BIN_VERSION}/win_ia32/engage-shared.dll lib/win32.ia32
    cp ${BIN_ROOT}/${SRC_BIN_VERSION}/win_ia32/engage-shared.lib lib/win32.ia32

    npm version ${SRC_BIN_VERSION}${VERSION_EXTENSION}
    npm publish

    cd ${CURRDIR}
    rm -rf ${BUILD_ROOT}
}

#determine_SRC_bin_version
echo "Publishing for version ${SRC_BIN_VERSION}${VERSION_EXTENSION} ..."

publish_it
