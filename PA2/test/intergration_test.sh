#!/bin/bash

#==========================
#   Test Preparation
#==========================
echo "start testing:"

cd "$(dirname "$0")"
echo "working directory: "$PWD""

svr_addr="0.0.0.0"
svr_port=8080
echo "server listen on: $svr_addr : $svr_port"

rm -rf output
mkdir -p output
out_dir="output"

failed_any=0

#==========================
# Test 1: Normal Operation
#==========================
# make fifo for clients, input will be feed to fifo later
#mkfifo $out_dir/c0_in
#mkfifo $out_dir/c1_in
#mkfifo $out_dir/c2_in
#
## start sever
#../cmake-build-debug/output/server $svr_port > $out_dir/svr_out & svr_pid=$!
#
## wait server for ready
#sleep 1s
#
## start 3 clients
#../cmake-build-debug/output/client 0 $svr_addr $svr_port > $out_dir/c0_out < $out_dir/c0_in &
#c0_pid=$!
#../cmake-build-debug/output/client 1 $svr_addr $svr_port > $out_dir/c1_out < $out_dir/c1_in &
#c1_pid=$!
#../cmake-build-debug/output/client 2 $svr_addr $svr_port > $out_dir/c2_out < $out_dir/c2_in &
#c2_pid=$!
#
## write data to client standard input
#echo "hello, I'm client 0" > $out_dir/c0_in; sleep 0.1
#echo "hello, I'm client 1" > $out_dir/c1_in; sleep 0.1
#echo "hello, I'm client 2" > $out_dir/c2_in
#
#sleep 3s
#
#kill -9 $c0_pid
#kill -9 $c1_pid
#kill -9 $c2_pid
#kill -9 $svr_pid

#============================
# Test 2: Duplicate username
#============================
# start sever
../cmake-build-debug/output/server $svr_port >/dev/null &
svr_pid=$!

sleep 1

../cmake-build-debug/output/client 0 $svr_addr $svr_port >/dev/null &
c0_pid=$!

../cmake-build-debug/output/client 0 $svr_addr $svr_port >/dev/null &
c1_pid=$!

if [ "$?" -ne 0 ] ; then
  echo '---' Test 2: Duplicate username PASS
else
  echo '---' Client with duplicate username should be prohibited
  echo '---' Test 2: Duplicate username FAILED
  failed_any=1
fi

kill -15 $svr_pid
kill -15 $svr_pid
kill -15 $svr_pid