#!/bin/sh
bin=`readlink -f "$0"`
here=`dirname "$bin"`
#LD_LIBRARY_PATH=$here/lib:$LD_LIBRARY_PATH
#export LD_LIBRARY_PATH
#exec "$bin".bin "$@"
exec "$here"/lib/ld-linux*.so* --library-path "$here"/lib "$bin".bin "$@"
