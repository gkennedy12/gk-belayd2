#!/bin/bash
#### LICENSE TBD
#
# Note: based on libcgroup's ftests-wrapper.sh
#

AUTOMAKE_SKIPPED=77
AUTOMAKE_HARD_ERROR=99

START_DIR=$PWD
echo START_DIR: $START_DIR
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
echo SCRIPT_DIR: $SCRIPT_DIR

if [ "$START_DIR" != "$SCRIPT_DIR" ]; then
	cp "$SCRIPT_DIR"/*.py "$START_DIR"
	cp "$SCRIPT_DIR"/*.json "$START_DIR"
	cp "$SCRIPT_DIR"/*.token "$START_DIR"
	ls -lrt
fi


set -x
./ftests.py -l 10 -L "$START_DIR/ftests.py.log"
RET=$?
set +x
echo RET: $RET

if [ -z "$srcdir" ]; then
	# $srcdir is set by automake but will likely be empty when run by hand and
	# that's fine
	srcdir=""
else
	srcdir=$srcdir"/"
fi

if [ "$START_DIR" != "$SCRIPT_DIR" ]; then
	rm -f "$START_DIR"/*.py
	rm -f "$START_DIR"/*.json
	rm -f "$START_DIR"/*.token
	rm -fr "$START_DIR"/__pycache__
	rm -f ftests.py.log
fi

exit $RET
