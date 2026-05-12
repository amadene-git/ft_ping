#!/bin/bash

FT_STDOUT=ft_stdout.log
FT_STDERR=ft_stderr.log
FT_RETURN=-1

EXPECT_STDOUT=expect_out.log
EXPECT_STDERR=expect_err.log
EXPECT_RETURN=-1
LAST_CMD=""

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

TEST_FAILED=0
TEST_PASSED=0

function execCommand() {
    cmd="timeout -s INT 2 ping $1 $2 $3";
    ft_cmd="valgrind --leak-check=full --log-file=valgrind.log timeout -s INT 2 ./ft_ping $1 $2 $3";
    LAST_CMD=$cmd

    FT_RETURN=$($(echo $ft_cmd) 1> $FT_STDOUT 2> $FT_STDERR);
    EXPECT_RETURN=$($(echo $cmd) 1> $EXPECT_STDOUT 2> $EXPECT_STDERR);

}

function compareOut() {
    grep -q "ERROR SUMMARY: 0 errors" valgrind.log
    if [[ $? -ne 0 ]]
    then
        printf "${RED} Testing '$LAST_CMD' valgrind KO $NC\n"
        (( TEST_FAILED++ ))
        return 1;
    fi


    if [[ $EXPECT_RETURN -ne $FT_RETURN ]]
    then
        printf "${RED} Testing '$LAST_CMD' failed, return '$FT_RETURN' expected '$EXPECT_RETURN' $NC\n"
        (( TEST_FAILED++ ))
        return 1;
    fi

    sed -i 's/ft_ping/ping/g' $FT_STDOUT
    sed -i 's/FT_PING/PING/g' $FT_STDOUT
    sed -i 's/time=[0-9]*.[0-9]* ms/time=0 ms/g' $FT_STDOUT $EXPECT_STDOUT
    sed -i '/round-trip/d' $FT_STDOUT $EXPECT_STDOUT
    sed -i 's/0x[0-9A-Fa-f]* = [0-9]*/0x0 = 0/g' $FT_STDOUT $EXPECT_STDOUT 
    diff $FT_STDOUT $EXPECT_STDOUT 
    
    if [[ $? -ne 0 ]]
    then
        printf "${RED} Testing '$LAST_CMD' diff stdout failed $NC\n"
        (( TEST_FAILED++ ))
        return 1;
    fi

    sed -i 's/ft_ping/ping/g' $FT_STDERR
    sed -i "s/or 'ping --usage' //g" $EXPECT_STDERR

    diff $FT_STDERR $EXPECT_STDERR
    if [[ $? -ne 0 ]]
    then
        printf "${RED} Testing '$LAST_CMD' diff stderr failed $NC\n"
        (( TEST_FAILED++ ))
        return 1;
    fi

    (( TEST_PASSED++ ))
}

function cleanOut() {
    rm -rf $FT_STDOUT $FT_STDERR $EXPECT_STDOUT $EXPECT_STDERR valgrind.log;
    FT_RETURN=NaN
    EXPECT_RETURN=NaN
}

function testCommand() {
    execCommand $1
    compareOut
    cleanOut
    echo "test passed $TEST_PASSED '$LAST_CMD'"
    echo 🥁
}

function printResult() {
    if [[ $TEST_FAILED -ne 0 ]]
    then
        printf "${RED}$TEST_FAILED/$(($TEST_FAILED + $TEST_PASSED)) test(s) failed\n"
        echo "😱 😱 😱"
    else
        printf "${GREEN}$TEST_PASSED/$(($TEST_FAILED + $TEST_PASSED)) test(s) passed\n"
        printf "All tests passed ! $NC\n"
        echo 🥳
    fi
}

function testTooManyHosts() {

    ./ft_ping google.com duckduckgo.com 1> $FT_STDOUT 2> $FT_STDERR;
    FT_RETURN=$?

    diff $FT_STDOUT /dev/null
    if [[ $? -ne 0 ]]
    then
        printf "${RED} Testing '$LAST_CMD' diff stdout failed $NC\n"
        (( TEST_FAILED++ ))
        return 1;
    fi


    echo "ft_ping: too many host 'duckduckgo.com'" >> $EXPECT_STDERR
    echo "Try 'ping --help' for more information." >> $EXPECT_STDERR
    diff $FT_STDERR $EXPECT_STDERR
    if [[ $? -ne 0 ]]
    then
        printf "${RED} Testing '$LAST_CMD' diff stdout failed $NC\n"
        (( TEST_FAILED++ ))
        return 1;
    fi

    if [[ $FT_RETURN -ne 64 ]]
    then
        printf "${RED} Testing '$LAST_CMD'  failed $NC\n"
        (( TEST_FAILED++ ))
        return 1;
    fi

    (( TEST_PASSED++ ))

    echo "test passed $TEST_PASSED './ft_ping google.com duckduckgo.com'"

}


make

# # host valid
# testCommand "127.0.0.1"
# testCommand "google.com"
# testCommand "-- google.com"

# # No host
# testCommand ""
# # bad host
# testCommand "abc"
# testCommand "192.0.0.1"
# testCommand "256.0.0.1"
# testCommand "-"
# testCommand "--"

# # invalid option
# testCommand "--snsdolancs"
# testCommand "--snsdolancs google.com"
# testCommand "-z"
# testCommand "-- -- google.com"

# testTooManyHosts
# cleanOut

# # host unreachable
# testCommand "10.0.0.1"


# # verbose
# testCommand "-v 127.0.0.1"

# memory leak
testCommand "debian.org"



# testCommand "www.kernel.org"
# testCommand "www.kernel.com"
# testCommand "kernel.com"
# testCommand "kernel.org"



printResult



