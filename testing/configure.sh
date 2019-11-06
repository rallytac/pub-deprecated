#!/bin/bash

function show_usage()
{
    echo "usage: configure <platform> <binary_version>"    
}

PLATFORM=${1}
VERSION=${2}
GITHUB_BASE=https://github.com/rallytac/pub/raw/master

if [[ "${PLATFORM}" == "" ]]; then
    show_usage
    exit 1
fi

if [[ "${VERSION}" == "" ]]; then
    show_usage
    exit 1
fi

# Configurations
wget ${GITHUB_BASE}/configurations/sample_engine_policy.json
cat sample_engine_policy.json | sed 's/@..\/certificates\//@.\//g' > sample_engine_policy.json
wget ${GITHUB_BASE}/configurations/sample_mission_template.json
cat sample_mission_template.json | sed 's/@..\/certificates\//@.\//g' > sample_mission_template.json

# Certificates
wget ${GITHUB_BASE}/certificates/rtsCA.pem
wget ${GITHUB_BASE}/certificates/rtsFactoryDefaultEngage.pem
wget ${GITHUB_BASE}/certificates/rtsFactoryDefaultEngage.key

# Binaries
wget ${GITHUB_BASE}/bin/${VERSION}/${PLATFORM}/engage-cmd
chmod +x engage-cmd
wget ${GITHUB_BASE}/bin/${VERSION}/${PLATFORM}/libengage-shared.so
chmod +x libengage-shared.so

echo "Dont forget to run 'export LD_LIBRARY_PATH=./' in your terminal!"
