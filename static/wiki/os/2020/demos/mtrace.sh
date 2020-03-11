#!/bin/bash

trace() {
  # ltrace outputs to stderr, so discard stdout
  # -f: trace threads/sub-processes
  # -e 'malloc+free-@libc.so*': only trace malloc/free
  ltrace -f -e 'malloc+free-@libc.so*' $@ > /dev/null
}

trace $@ 2>&1 # move stderr to stdout
