#!/bin/bash

mkdir bin
cd core
make r 2>&1
cp tracecheck_release ../bin/tracecheck_LRAT

