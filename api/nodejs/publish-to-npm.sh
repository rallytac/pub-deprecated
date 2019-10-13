#!/bin/bash

echo "================================================================================"
echo "Engage NPM Publisher"
echo "Copyright (c) 2019 Rally Tactical Systems, Inc."
echo "================================================================================"

THIS_ROOT=`pwd`
BUILD_ROOT=${THIS_ROOT}/.build
API_ROOT=${THIS_ROOT}/../
BIN_ROOT=${THIS_ROOT}/../../bin
LATEST_BIN_VERSION=""

function determine_latest_bin_version()
{
    FILES=(`ls -r ${BIN_ROOT}`)
    LATEST_BIN_VERSION=${FILES[0]}
}

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
    cp ${BIN_ROOT}/${LATEST_BIN_VERSION}/darwin/libengage-static.a lib/darwin.x64

    mkdir -p lib/linux.x64
    cp ${BIN_ROOT}/${LATEST_BIN_VERSION}/linux.x64/libengage-static.a lib/linux.x64

    #mkdir -p lib/linux.ia32
    #cp ${BIN_ROOT}/${LATEST_BIN_VERSION}/linux.ia32/libengage-static.a lib/linux.ia32

    #mkdir -p lib/win.x64
    #mkdir -p lib/win.ia32

    npm version ${LATEST_BIN_VERSION}
    #npm publish

    cd ${CURRDIR}
}

determine_latest_bin_version
echo "Publishing for version ${LATEST_BIN_VERSION} ..."

publish_it
