#!/bin/bash

DEVICE=01:011

program()
{
    sudo mspdebug rf2500 -U $DEVICE "prog main.elf"
}

debug()
{
    setsid sh -c "mspdebug rf2500 -U $DEVICE \"gdb\" 2>&1 1>/dev/null &"

    msp430-gdb

    sudo killall mspdebug
}

P_FLAG=0
D_FLAG=0

for i in $@
do
    if [[ $i == -p ]]
    then
        P_FLAG=1
    fi

    if [[ $i == -d ]]
    then
        D_FLAG=1
    fi
done

if [[ $P_FLAG == 1 ]]
then
    program
fi

if [[ $D_FLAG == 1 ]]
then
    debug
fi
