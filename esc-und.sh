#!/bin/sh
# escape unescaped underscores in chunk names (@use/@defn)
# otherwise they will be interpreted by TeX, with unpleasant results
cmd='s/([^\\])_/\1\\_/g'
sed -E -e '/^@use /'$cmd \
       -e '/^@defn /'$cmd
