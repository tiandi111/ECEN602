#!/bin/bash

# ==================================
#       Test Preparation
# ==================================
cd "$(dirname "$0")"

bin_path=../bin

addr=0.0.0.0

port=69

log_file=server_log

# start server
./$bin_path/server $port  & svr_pid=$!

# ==================================
#       Test Helper Functions
# ==================================

# Print Test Result
#   arg1: program return code
#   arg2: test case name
echo_test_res () {
  RED='\033[0;31m'
  GRN='\033[0;32m'
  NC='\033[0m'
  if [ "$1" -eq 0 ] ; then
    echo -e "${GRN}$2 (PASS)${NC}"
  else
    echo -e "${RED}$2 (FAILED)${NC}"
  fi
}

# ==================================
#   Test: Transfer binary files
# ==================================
trans_bin_file () {
  # generate temp file
  temp_file_path=$(mktemp)
  temp_file_name=$(basename $temp_file_path)
  # fill the temp file
  dd if=/dev/urandom of="$temp_file_path" bs="$1" count=1 &>/dev/null &&
  # sends tftp request
  echo "get $temp_file_path" | tftp $addr $port &>/dev/null
  # wait for complete
  sleep 1
  # compare results
  cmp "$temp_file_name" "$temp_file_path" &&
  # output return code
  echo "$?"
  # remove file
  rm "$temp_file_path" "$temp_file_name"
}

# --- Sub Test 1: 2048B ---
res=$(trans_bin_file "2048")
echo_test_res "$res" "--- Test 1: Transfer a binary file (2048B)"

# --- Sub Test 2: 2047B ---
res=$(trans_bin_file "2047")
echo_test_res "$res" "--- Test 2: Transfer a binary file (2047B)"

# --- Sub Test 3: 34MB ---
res=$(trans_bin_file "35651584")
echo_test_res "$res" "--- Test 3: Transfer a binary file (34MB)"

# ==================================
#   Test: Transfer text files
# ==================================
txt_file_path=./data/test_txt
txt_file_name=$(basename $txt_file_path)
printf "ascii\n get $txt_file_path" | tftp $addr $port &>/dev/null
sleep 1
cmp "$txt_file_path" "$txt_file_name"
res=$?
echo_test_res "$res" "--- Test 4: Transfer a text file"
rm "$txt_file_name"

# ==================================
#   Test: Receive Error Packet
# ==================================
non_exist_file=/tmp/xxxxx && rm non_exist_file &>/dev/null
output=$(echo "get $non_exist_file" | tftp $addr $port)
ret=$([[ $output == *"Error"* ]] && echo "0" || echo "1")
echo_test_res "$ret" "--- Test 5: Receive Error Packet"
rm $(basename $non_exist_file)

# ==================================
#   Test: Multiple Clients
# ==================================
temp_file_path=$(mktemp)
temp_file_name=$(basename "$temp_file_path")

dd if=/dev/urandom of="$temp_file_path" bs=4194304 count=1 &>/dev/null
echo "get $temp_file_path ${temp_file_name}_1" | tftp $addr $port &>/dev/null &
echo "get $temp_file_path ${temp_file_name}_2" | tftp $addr $port &>/dev/null &
echo "get $temp_file_path ${temp_file_name}_3" | tftp $addr $port &>/dev/null &

sleep 15s

cmp "$temp_file_path" "${temp_file_name}_1" &&
cmp "$temp_file_path" "${temp_file_name}_2" &&
cmp "$temp_file_path" "${temp_file_name}_3"
res=$?

echo_test_res "$ret" "--- Test 6: Multiple Clients"

rm "$temp_file_path" "${temp_file_name}"*

# ==================================
#   Test: Interrupt Client
# ==================================
temp_file_path=$(mktemp)
temp_file_name=$(basename "$temp_file_path")

dd if=/dev/urandom of="$temp_file_path" bs=31457280 count=1 &>/dev/null
echo "get $temp_file_path" | tftp $addr $port &>/dev/null & c_pid=$!

sleep 1
kill -9 $c_pid

sleep 10s

echo_test_res "0" "--- Test 7: Interrupt Client"

rm "$temp_file_path" "$temp_file_name"

# ----- Test Cleanup -----
kill -9 $svr_pid
