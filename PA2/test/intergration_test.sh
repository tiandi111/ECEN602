#!/bin/bash

#==========================
#   Test Preparation
#==========================
echo "Start Testing:"

cd "$(dirname "$0")"
echo "working directory: "$PWD""

bin_path=../bin
#bin_path=../cmake-build-debug/output

svr_addr=0.0.0.0
svr_port=8080

rm -rf output
mkdir -p output
out_dir=output
data_dir=data

failed_any=0

#=========================
# Helper Functions
#=========================

#==========================
# Test 1: Normal Operation
#==========================

svr_port=8080

# make fifo for clients stdin redirection
# use "sleep 7 >" to open fifos so that they won't block clients
mkfifo $out_dir/c0_in && sleep 10 > $out_dir/c0_in &
mkfifo $out_dir/c1_in && sleep 10 > $out_dir/c1_in &
mkfifo $out_dir/c2_in && sleep 10 > $out_dir/c2_in &

# start sever
timeout 10 $bin_path/server $svr_port > $out_dir/svr_out &
sleep 1

## start 3 clients, sleep for clients to set up completely
timeout 4s $bin_path/client 0 $svr_addr $svr_port >$out_dir/c0_out <$out_dir/c0_in &
sleep 1
timeout 4s $bin_path/client 1 $svr_addr $svr_port >$out_dir/c1_out <$out_dir/c1_in &
sleep 1
timeout 4s $bin_path/client 2 $svr_addr $svr_port >$out_dir/c2_out <$out_dir/c2_in &
sleep 1

## feed input to clients
echo "hello, I'm client 0" > $out_dir/c0_in; sleep 0.1
echo "hello, I'm client 1" > $out_dir/c1_in; sleep 0.1
echo "hello, I'm client 2" > $out_dir/c2_in; sleep 0.1

# compare outputs
if
cmp $out_dir/c0_out $data_dir/c0_out_expected &&
cmp $out_dir/c1_out $data_dir/c1_out_expected &&
cmp $out_dir/c2_out $data_dir/c2_out_expected;
then
  echo '---' Test 1: Normal Operation PASS
else
  echo '---' Unexpected client outputs
  echo '---' Test 1: Normal Operation FAILED
  failed_any=1
fi

#============================
# Test 2: Duplicate username
#============================
svr_port=8081
case1_exit_code=0
case2_exit_code=0

# start server
timeout 8s $bin_path/server $svr_port &
sleep 0.5

# joint the first client with name '0'
timeout 2s $bin_path/client 0 $svr_addr $svr_port >/dev/null & c0_pid=$!
sleep 0.5

# joint the second client with name '0', expect to be rejected
timeout 1s $bin_path/client 0 $svr_addr $svr_port >/dev/null
case1_exit_code=$?

# wait the first two client with name '0' to exit
wait $c0_pid

# joint the third client with name '0', expect to be accpeted
timeout 1s $bin_path/client 0 $svr_addr $svr_port >/dev/null
case2_exit_code=$?

if [ $case1_exit_code -ne 1 ]; then
  echo '---' Client with duplicate username should be prohibited
  echo '---' Test 2: Duplicate username FAILED
  failed_any=1
# exit with 124 means the process is terminated by timeout
elif [ $case2_exit_code -ne 124 ]; then
  echo '---' Client username should be recycled
  echo '---' Test 2: Duplicate username FAILED
  failed_any=1
else
  echo '---' Test 2: Duplicate username PASS
fi

wait

#==============================
# Test 3: Max Number of Clients
#==============================
svr_port=8082

# start server, set maximum number of client to be 1
timeout 3s $bin_path/server $svr_port 1 &
sleep 0.5

# join the first client
timeout 2s $bin_path/client 0 $svr_addr $svr_port >/dev/null &
sleep 0.5

# join the second client, expect to be rejected
timeout 1s $bin_path/client 1 $svr_addr $svr_port >/dev/null

if [ $? -eq 1 ]; then
  echo '---' Test 3: Max Number of Clients PASS
else
  echo '---' Clients should be rejected when excceed maximum number of clients
  echo '---' Test 3: Duplicate username FAILED
  failed_any=1
fi

wait

#==============================
if [ $failed_any -eq 0 ]; then
    echo '***' PASSED ALL TESTS
else
    echo '***' FAILED SOME TESTS
    exit 1
fi
