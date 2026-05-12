#!/bin/bash

FT_STDOUT=ft_stdout.log
FT_STDERR=ft_stderr.log
FT_RETURN=-1

EXPECT_STDOUT=expect_out.log
EXPECT_STDERR=expect_err.log
EXPECT_RETURN=-1
LAST_CMD=""
VALGRIND_LOG=valgrind.log

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

TEST_FAILED=0
TEST_PASSED=0

function execCommand() {
    echo "run test $(($TEST_FAILED + $TEST_PASSED)) '$LAST_CMD' 🥁"

    cmd="timeout -s INT 2 ping $1 $2 $3";
    ft_cmd="valgrind --leak-check=full --log-file=$VALGRIND_LOG timeout -s INT 2 ./ft_ping $1 $2 $3";
    LAST_CMD=$cmd

    FT_RETURN=$($(echo $ft_cmd) 1> $FT_STDOUT 2> $FT_STDERR);
    EXPECT_RETURN=$($(echo $cmd) 1> $EXPECT_STDOUT 2> $EXPECT_STDERR);

}

function compareOut() {

    if [ -f $VALGRIND_LOG ]
    then
        grep -q "ERROR SUMMARY: 0 errors" $VALGRIND_LOG > /dev/null
        if [[ $? -ne 0 ]]
        then
            printf "${RED} Testing '$LAST_CMD' valgrind KO $NC\n"
            (( TEST_FAILED++ ))
            return 1;
        fi
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


    printf "${GREEN} Test passed $(($TEST_FAILED + $TEST_PASSED)) '$LAST_CMD' $NC\n"


    (( TEST_PASSED++ ))


}

function cleanOut() {
    rm -rf $FT_STDOUT $FT_STDERR $EXPECT_STDOUT $EXPECT_STDERR $VALGRIND_LOG;
    FT_RETURN=NaN
    EXPECT_RETURN=NaN
}

function testCommand() {
    execCommand $1
    compareOut
    cleanOut
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
    LAST_CMD="ping google.com duckduckgo.com"
    echo "run test $(($TEST_FAILED + $TEST_PASSED)) '$LAST_CMD' 🥁"

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

    echo "test passed $(($TEST_FAILED + $TEST_PASSED)) '$LAST_CMD'"

}

function testIcmpPacketTypeCode() {
    ICMP_PACKET_TYPE=$1
    ICMP_PACKET_CODE=$2
    LAST_CMD="testIcmpPacketTypeCode type $1 code $2"
    echo
    echo "run test $(($TEST_FAILED + $TEST_PASSED)) '$LAST_CMD' 🥁"


cat << EOF > forge.py
#!/usr/bin/env python3
from scapy.all import IP, ICMP, send
import sys
import time

ft_pid, target, seq = int(sys.argv[1]), sys.argv[2], int(sys.argv[3])
orig = IP(src="127.0.0.1", dst=target, proto=1) / ICMP(type=8, code=0, id=ft_pid & 0xffff, seq=seq)
send(IP(dst="127.0.0.1") / ICMP(type=$ICMP_PACKET_TYPE, code=$ICMP_PACKET_CODE) / bytes(orig)[:28])

EOF


    ./ft_ping 127.0.0.1 1> $FT_STDOUT 2> $FT_STDERR & FT_PID=$!
    python3 forge.py $FT_PID 127.0.0.1 0 1> /dev/null;
    sleep 2;
    kill -s INT $FT_PID
    
    ping -n 127.0.0.1 1> $EXPECT_STDOUT 2> $EXPECT_STDERR & PING_PID=$!
    python3 forge.py $PING_PID 127.0.0.1 0 1> /dev/null;
    sleep 2;
    kill -s INT $PING_PID
    
    rm -rf forge.py

    compareOut
    cleanOut
}

make

# host valid
testCommand "127.0.0.1"
testCommand "google.com"
testCommand "-- google.com"

# No host
testCommand ""
# bad host
testCommand "abc"
testCommand "192.0.0.1"
testCommand "256.0.0.1"
testCommand "-"
testCommand "--"

# invalid option
testCommand "--snsdolancs"
testCommand "--snsdolancs google.com"
testCommand "-z"
testCommand "-- -- google.com"


# test too many host in arg
testTooManyHosts
cleanOut

# host unreachable
testCommand "10.0.0.1"


# verbose
testCommand "-v 127.0.0.1"

# memory leak
testCommand "debian.org"


# test icmp type code
testIcmpPacketTypeCode 3 0     # Net Unreachable
testIcmpPacketTypeCode 3 1     # Host Unreachable
testIcmpPacketTypeCode 3 2     # Protocol Unreachable
testIcmpPacketTypeCode 3 3     # Port Unreachable
testIcmpPacketTypeCode 3 4     # Fragmentation needed
testIcmpPacketTypeCode 3 9     # Net Prohibited
testIcmpPacketTypeCode 3 10    # Host Prohibited
testIcmpPacketTypeCode 3 13    # Packet filtered

testIcmpPacketTypeCode 11 0    # TTL exceeded
testIcmpPacketTypeCode 11 1    # Frag reassembly time

testIcmpPacketTypeCode 5 0     # Redirect Network
testIcmpPacketTypeCode 5 1     # Redirect Host
testIcmpPacketTypeCode 5 2     # Redirect TOS & Network
testIcmpPacketTypeCode 5 3     # Redirect TOS & Host


# invalid type code
testIcmpPacketTypeCode 255 0    # cas 3 : fallback "Bad ICMP type: 255"
testIcmpPacketTypeCode 3 99     # cas 4 : "Dest Unreachable, Unknown Code: 99"
testIcmpPacketTypeCode 5 16     # Redirect TOS & Host






# valgrind errors + dns return conversion
# testCommand "www.kernel.org"
# testCommand "www.kernel.com"
# testCommand "kernel.com"
# testCommand "kernel.org"

# unreachable addr
# ping 192.0.2.1     # TEST-NET-1 (RFC 5737)
# ping 240.0.0.1     # Class E réservée



printResult



