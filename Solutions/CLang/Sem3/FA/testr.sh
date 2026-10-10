#!/usr/bin/env bash

set +e

tests=(
"./second.out ln2 0"
"./second.out ln2 -1"
"./second.out sq2 -1"
"./second.out gamma 1"
"./second.out ln2 abc"
"./second.out unknown 5"
"./second.out ln2 1"
"./second.out ln2 20"
"./second.out eul 10"
"./second.out pi 10"
"./second.out sq2 20"
"./second.out gamma 2"
"./second.out gamma 0"
)

for cmd in "${tests[@]}"; do
    echo "================================"
    echo "TEST: $cmd"

    timeout 3s bash -c "$cmd" \
        >test_stdout.txt 2>test_stderr.txt

    rc=$?

    echo "EXIT CODE: $rc"
    echo "STDOUT:"
    cat test_stdout.txt
    echo "STDERR:"
    cat test_stderr.txt
done

rm -f test_stdout.txt test_stderr.txt
