#!/bin/sh
here=`dirname "$0"`
#LD_LIBRARY_PATH=$here/lib:$LD_LIBRARY_PATH
#export LD_LIBRARY_PATH
#exec "$0".bin "$@"
exec "$here"/lib/ld-linux*.so* --library-path "$here"/lib "$0".bin "$@"
