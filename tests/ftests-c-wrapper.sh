#!/bin/bash
#### LICENSE TBD

BASEDIR=$(dirname $0)

RET=0

for FILE in *; do
	if [[ $FILE == *.c ]]; then
		EXE=${FILE%.c}

		echo "Running $EXE $BASEDIR"
		if [[ -x "./$EXE" ]]; then
			CMD="./$EXE"
		else
			CMD=${srcdir}/$EXE
		fi
		$CMD $BASEDIR
		TMP_RET=$?
		echo "$CMD returned $TMP_RET"

		if [[ TMP_RET -gt RET ]]; then
			RET=$TMP_RET
		fi
	fi
done

exit $RET
