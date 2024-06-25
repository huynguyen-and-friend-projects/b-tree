#!/bin/sh

cd $(dirname $0)

if [ ! -f ./build/test/test-exe ]; then
    echo << EOF
    Error, please build the test first
EOF
    exit 1
fi

cd './build/test/'

ctest
