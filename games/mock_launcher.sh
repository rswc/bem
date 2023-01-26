#!/bin/bash

if [ $# -ne 5 ]; then
    echo "Error: script expects 5 parameters, but got $#." >&2
    exit 1
fi

if [ ! -f "$1" ] || [ ! -f "$2" ]; then
    echo "Error: first two arguments must be existing files." >&2
    exit 1
fi

if [ $(( RANDOM % 100 )) -le "1" ]; then
    echo "Error: Program terminated with 1% prob. Just because :-)" >&2
    exit 1
fi

# Generate a random number between 5 and 20
n_rounds=$(( ( RANDOM % 16 )  + 5 ))
echo "Playing game $3 with board size of $5 and $n_rounds rounds." >&2

reason=""
winner=""
seconds=$(bc <<< "scale=3;$5 / 1000")

for i in $(seq 1 $n_rounds); do

    echo "Playing round: $i, sleeping $seconds [s]." >&2
    sleep $seconds 
    
    if [ $(( RANDOM % 100 )) -le "5" ]; then
        reason="TIMEOUT"
        break
    fi
done

if [ $(expr $n_rounds % 2) != "0" ]; then
    winner="\"PLAYER1\""
else 
    winner="\"PLAYER2\""
fi

echo "$1;$2;$winner;$reason;"