#!/bin/bash
set -e -x

# CLI arguments
PY_VERSIONS=$1
BUILD_REQUIREMENTS=$2
SYSTEM_PACKAGES=$3
PACKAGE_PATH=$4
PIP_WHEEL_ARGS=$5

if [ ! -z "$SYSTEM_PACKAGES" ]; then
    yum install -y ${SYSTEM_PACKAGES}  || { echo "Installing yum package(s) failed."; exit 1; }
fi

scl enable devtoolset-9 bash
export CMAKE_EXE=cmake3
export METRIC_SOURCE_PATH=`python2 -c "import os,sys; print os.path.realpath(os.path.join(sys.argv[1], os.pardir))" ${PACKAGE_PATH}`

# Compile wheels
arrPY_VERSIONS=(${PY_VERSIONS// / })
for PY_VER in "${arrPY_VERSIONS[@]}"; do
    # Update pip
    /opt/python/"${PY_VER}"/bin/pip install --upgrade --no-cache-dir pip

    # Check if requirements were passed
    if [ ! -z "$BUILD_REQUIREMENTS" ]; then
        /opt/python/"${PY_VER}"/bin/pip install --no-cache-dir ${BUILD_REQUIREMENTS} || { echo "Installing requirements failed."; exit 1; }
    fi

    # Build wheels
    /opt/python/"${PY_VER}"/bin/pip wheel /github/workspace/"${PACKAGE_PATH}" -w /github/workspace/wheelhouse/ ${PIP_WHEEL_ARGS} || { echo "Building wheels failed."; exit 1; }
done

# Bundle external shared libraries into the wheels
for whl in /github/workspace/wheelhouse/*-linux*.whl; do
    auditwheel repair "$whl" --plat "${PLAT}" -w /github/workspace/wheelhouse/ || { echo "Repairing wheels failed."; auditwheel show "$whl"; exit 1; }
done

echo "Succesfully build wheels:"
ls /github/workspace/wheelhouse
