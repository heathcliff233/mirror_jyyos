#!/bin/bash

qemu-system-x86_64 -s -S -nographic -serial none -machine accel=tcg -smp 4 -drive format=raw,file=build/amtest-x86_64-qemu &
pid=$!
gdb \
  -ex "target remote localhost:1234" \
  -ex "set confirm off"

kill -9 $!
