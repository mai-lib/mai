#!/bin/bash

# noweb doesn't have a version, annoyingly
NOWEB=$(zgrep -o '^version.*[^.]' $(man -w noweb))
echo "\begin{verbatim}"
cat /etc/issue.net; echo
echo "noweb ${NOWEB}"; echo

# binaries and libc
#LIBC=$(find /lib -name 'libc.so.*')
#$LIBC
BINS=(pdflatex gcc make bison m4 bash emacs)
for bin in "${BINS[@]}"; do
    $bin --version | head -n1
    echo
done

echo "\end{verbatim}"
