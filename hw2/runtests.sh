#! /bin/bash

TEST_DIR=tests/
EXEC=hw2
OUTPUT=out
TEMP=temp
DIFF_FILE=diff_file

RED='\033[41;37m'
GREEN='\033[42m'
BLUE='\033[44m'
RESET='\033[0m'

# after exporting envfile,
# use absolute path for commands [date, cut, sed] since PATH might be overwritten
DATE=$(which date)
CUT=$(which cut)
SED=$(which sed)
TEST=$(which test)
MD5SUM=$(which md5sum)
XARGS=$(which xargs)
make all

# check if running sub test
function choose_single_test {
    if [ $# -eq 1 ]
    then
        grep -E "${TEST_DIR}${SUBDIR}/$1"
    else
        :
    fi
}

make clean && make all

ALL_PASS=1

# run tests
for SUBDIR in $(ls -1 $TEST_DIR | grep -E "$1" | sort)
do
    echo $SUBDIR
    for IN in $(ls -1 ${TEST_DIR}${SUBDIR}/*.in | choose_single_test "$2" | sort)
    do
        PASS=1
        NAME=$(echo ${IN} | cut -d '.' -f1)
        # inform which test will run
        echo -e "\n\n${BLUE} Running Test ${RESET} ${NAME}"

        # set up environment if applicable
        # make a copy of the original PATH variable first
        PATH_COPY=$PATH
        ENVFILE=$($SED 's/.in$/.env/g' <<< ${IN})
        export $(cat $ENVFILE | $XARGS) &> /dev/null || true

        # exec
        T1=$($DATE +%s%N | $CUT -b1-13 | $SED s/N/000/g)
        ./${EXEC} < ${IN} &> $OUTPUT
        # set PATH back to original
        export PATH=$PATH_COPY

        RET=$?
        T2=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        TT=$((T2-T1))

        # identify important files
        TITLE="DIFF"
        EXPECTED=$(sed 's/.in$/.expected/g' <<< ${IN})
        MD5=$(sed 's/.in/.md5/g' <<< ${IN})
        if $TEST -f "$MD5"; then
            EXPECTED=$(echo ${MD5})
            cat $OUTPUT | $MD5SUM > $TEMP
            mv $TEMP $OUTPUT
            TITLE="MD5"
        fi

        # determine if run failed
        if [ $RET -eq 0 ]; then
            echo -e "${GREEN} exec OK in ${TT}ms ${RESET}"
        else
            echo -e "${RED} exec FAIL in ${TT}ms ${RESET}"
            PASS=0
            ALL_PASS=0
        fi

        # determine if output matches
        diff -y $EXPECTED $OUTPUT &> "$DIFF_FILE"
        DIFF_RET=$?
        if [ $DIFF_RET -eq 0 ]; then
            echo -e "${GREEN} ${TITLE} OK ${RESET}"
        else
            echo -e "${RED} ${TITLE} FAILED ${RESET}"
            echo "diff -y \$EXPECTED \$OUTPUT"
            cat "$DIFF_FILE"
            PASS=0
            ALL_PASS=0
        fi

        # determine if test was passed
        if [ $PASS -eq 0 ]; then
            echo -e "${RED} TEST ${NAME} FAILED ${RESET}"
        else
            echo -e "${GREEN} TEST ${NAME} PASSED ${RESET}"
        fi
    done
done

rm -f "$DIFF_FILE" $TEMP $OUTPUT
make clean

if [ $ALL_PASS -eq 0 ]; then
    echo -e "${RED} Some test FAILED ${RESET}"
    exit 1
else
    echo -e "${GREEN} All tests PASSED ${RESET}"
    exit 0
fi
