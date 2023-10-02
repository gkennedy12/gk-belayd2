#!/bin/bash
#### LICENSE TBD
#
# Note: based on libcgroup's ftests-wrapper.sh
#

AUTOMAKE_SKIPPED=77
AUTOMAKE_HARD_ERROR=99

START_DIR=$PWD
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ "$START_DIR" != "$SCRIPT_DIR" ]; then
	cp "$SCRIPT_DIR"/*.py "$START_DIR"
fi


./ftests.py -l 10 -L "$START_DIR/ftests.py.log"
RET=$?

if [ -z "$srcdir" ]; then
	# $srcdir is set by automake but will likely be empty when run by hand and
	# that's fine
	srcdir=""
else
	srcdir=$srcdir"/"
fi

if [ "$START_DIR" != "$SCRIPT_DIR" ]; then
	rm -f "$START_DIR"/*.py
	rm -fr "$START_DIR"/__pycache__
	rm -f ftests.py.log
fi

exit $RET
