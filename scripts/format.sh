#!/bin/bash

for name in `find -E src include -regex '.*\.hpp|.*\.cpp'`
do
	clang-format --verbose -i "$name"
done
