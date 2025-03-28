#!/bin/bash

set -e

cd /opt/fprime/ledtest

fprime-util purge -f

fprime-util generate -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain/gr740-rtems5.cmake -DCMAKE_VERBOSE_MAKEFILE=ON

cd Components/Led

fprime-util build

cd ../../

cd LedBlinker

fprime-util build

cd ..

fprime-util build

wait

cp /opt/fprime/ledtest/build-artifacts/gr740-rtems5/LedBlinker/bin/LedBlinker.exe /app/exobiosphere/