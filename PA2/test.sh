#!/bin/bash

## Parameters
IP="127.0.0.1"
PORT="6666"
OUTPUT_DIR="testoutput"
FIFO0="$OUTPUT_DIR/fifo0"
FIFO1="$OUTPUT_DIR/fifo1"
FIFO2="$OUTPUT_DIR/fifo2"
FIFO3="$OUTPUT_DIR/fifo3"
START_WAITING_TIME=1.7s
MSG_WAITING_TIME=0.5s

rm -r $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

# Input FIFOs for clients
#mkfifo $FIFO0
mkfifo $FIFO1
mkfifo $FIFO2
mkfifo $FIFO3

echo "Start testing:"

# ==== Test case 1 ====
# Normal operations with 3 clients
# Scenario:
#   Server starts
#   Client 1 starts and send JOIN with username "user1"
#   Client 2 starts and send JOIN with username "user2"
#   Client 3 starts and send JOIN with username "user3"
#   Client 1 send message "Hi, I'm user1"
#   Client 2 send message "Hello, I'm user2"
#   Client 3 send message "Howdy, I'm user3"
#   Client 1 exits
#   Client 2 exits
#   Client 3 exits
#   Server exits

TESTCASE="testcase1"
MAXCLIENTS=3
USERNAME1="user1"
USERNAME2="user2"
USERNAME3="user3"

echo "================================ Testcase 1 starts ====================================================="
echo "Testcase 1: Normal operations with 3 clients"

### Start server, the output will be stored in $OUTPUT_DIR/${TESTCASE}_server_output
echo "Start server..."
./server $IP $PORT $MAXCLIENTS > $OUTPUT_DIR/${TESTCASE}_server_output 2>&1 &
SERVERPID=$!
echo "server pid = $SERVERPID"


### Start clients. The input will redirect to fifo and the output will be stored in $OUTPUT_DIR/${TESTCASE}_clientx_output
echo "Start client1..."
./client $USERNAME1 $IP $PORT < $FIFO1 > $OUTPUT_DIR/${TESTCASE}_client1_output 2>&1 &
CLIENT1PID=$!
echo "client1 pid = $CLIENT1PID"
sleep $START_WAITING_TIME

echo "Start client2..."
./client $USERNAME2 $IP $PORT < $FIFO2 > $OUTPUT_DIR/${TESTCASE}_client2_output 2>&1 &
CLIENT2PID=$!
echo "client2 pid = $CLIENT2PID"
sleep $START_WAITING_TIME

echo "Start client3..."
./client $USERNAME3 $IP $PORT < $FIFO3 > $OUTPUT_DIR/${TESTCASE}_client3_output 2>&1 &
CLIENT3PID=$!
echo "client3 pid = $CLIENT3PID"
sleep $START_WAITING_TIME


### Send messages
echo "Hi, I'm user1" > $FIFO1
sleep $MSG_WAITING_TIME
echo "Hello, I'm user2" > $FIFO2
sleep $MSG_WAITING_TIME
echo "Howdy, I'm user3" > $FIFO3
sleep $MSG_WAITING_TIME

### Kill the clients and the server
echo "Kill client1, pid = $CLIENT1PID"
kill -9 $CLIENT1PID
echo "Client1 exits"
sleep $MSG_WAITING_TIME

echo "Kill client2, pid = $CLIENT2PID"
kill -9 $CLIENT2PID
echo "Client2 exits"
sleep $MSG_WAITING_TIME

echo "Kill client3, pid = $CLIENT3PID"
kill $CLIENT3PID
echo "Client3 exits"
sleep $MSG_WAITING_TIME

echo "Kill server, pid = $SERVERPID"
kill $SERVERPID
echo "Server exits"
sleep $MSG_WAITING_TIME

echo "================================ Testcase 1 ends ====================================================="
sleep 2s

# ==== Test case 2 ====
# Server rejects a client with a duplicate username
# Scenario:
#   Server starts
#   Client 1 starts and send JOIN with username "user1"
#   Client 2 starts and send JOIN with username "user2"
#   Client 3 starts and send JOIN with username "user2"
#   Client 1 exits
#   Client 2 exits
#   Server exits

TESTCASE="testcase2"
MAXCLIENTS=3
USERNAME1="user1"
USERNAME2="user2"
USERNAME3="user2"

echo "================================ Testcase 2 starts ====================================================="
echo "Testcase 2: Server rejects a client with a duplicate username"

### Start server, the output will be stored in $OUTPUT_DIR/${TESTCASE}_server_output
echo "Start server..."
./server $IP $PORT $MAXCLIENTS > $OUTPUT_DIR/${TESTCASE}_server_output 2>&1 &
SERVERPID=$!
echo "server pid = $SERVERPID"


### Start clients. The input will redirect to fifo and the output will be stored in $OUTPUT_DIR/${TESTCASE}_clientx_output
echo "Start client1..."
./client $USERNAME1 $IP $PORT < $FIFO1 > $OUTPUT_DIR/${TESTCASE}_client1_output 2>&1 &
CLIENT1PID=$!
echo "client1 pid = $CLIENT1PID"
sleep $START_WAITING_TIME

echo "Start client2..."
./client $USERNAME2 $IP $PORT < $FIFO2 > $OUTPUT_DIR/${TESTCASE}_client2_output 2>&1 &
CLIENT2PID=$!
echo "client2 pid = $CLIENT2PID"
sleep $START_WAITING_TIME

echo "Start client3..."
./client $USERNAME3 $IP $PORT < $FIFO3 > $OUTPUT_DIR/${TESTCASE}_client3_output 2>&1 &
CLIENT3PID=$!
echo "client3 pid = $CLIENT3PID"
sleep $START_WAITING_TIME

### Let client output result to the files
echo "I'm user1" > $FIFO1
sleep $MSG_WAITING_TIME
echo "I'm user2" > $FIFO2
sleep $MSG_WAITING_TIME
echo "I'm user3" > $FIFO3
sleep $MSG_WAITING_TIME

### Kill the clients and the server
echo "Kill client1, pid = $CLIENT1PID"
kill $CLIENT1PID
echo "Client1 exits"
sleep $MSG_WAITING_TIME

echo "Kill client2, pid = $CLIENT2PID"
kill $CLIENT2PID
echo "Client2 exits"
sleep $MSG_WAITING_TIME

echo "Kill server, pid = $SERVERPID"
kill $SERVERPID
echo "Server exits"
sleep $MSG_WAITING_TIME

echo "================================ Testcase 2 ends ====================================================="


# ==== Test case 3 ====
# Server allows a previously used username to be reused
# Scenario:
#   Server starts
#   Client 1 starts and send JOIN with username "user1"
#   Client 2 starts and send JOIN with username "user2"
#   Client 2 send message "Hi, I'm user2 from client 2"
#   Client 2 exits
#   Client 3 starts and send JOIN with username "user2"
#   Client 3 send message "The user name 'user2' is reused by client 3."
#   Client 3 exits
#   Client 1 exits
#   Server exits

TESTCASE="testcase3"
MAXCLIENTS=3
USERNAME1="user1"
USERNAME2="user2"
USERNAME3="user2"

echo "================================ Testcase 3 starts ====================================================="
echo "Testcase 3: Server allows a previously used username to be reused"

### Start server, the output will be stored in $OUTPUT_DIR/${TESTCASE}_server_output
echo "Start server..."
./server $IP $PORT $MAXCLIENTS > $OUTPUT_DIR/${TESTCASE}_server_output 2>&1 &
SERVERPID=$!
echo "server pid = $SERVERPID"


### Start clients. The input will redirect to fifo and the output will be stored in $OUTPUT_DIR/${TESTCASE}_clientx_output
echo "Start client1..."
./client $USERNAME1 $IP $PORT < $FIFO1 > $OUTPUT_DIR/${TESTCASE}_client1_output 2>&1 &
CLIENT1PID=$!
echo "client1 pid = $CLIENT1PID"
sleep $START_WAITING_TIME

## Let client output result to the files
echo "Hi, I'm user1." > $FIFO1
sleep $MSG_WAITING_TIME

echo "Start client2..."
./client $USERNAME2 $IP $PORT < $FIFO2 > $OUTPUT_DIR/${TESTCASE}_client2_output 2>&1 &
CLIENT2PID=$!
echo "client2 pid = $CLIENT2PID"
sleep $START_WAITING_TIME

## Client 2 send message "Hi, I'm user2 from client 2"
echo "Hi, I'm user2 from client 2" > $FIFO2
sleep $MSG_WAITING_TIME

## Client 2 exits
echo "Kill client2, pid = $CLIENT2PID"
kill $CLIENT2PID
echo "Client2 exits"
sleep $MSG_WAITING_TIME


## Client 3 starts and send JOIN with username "user2"
echo "Start client3..."
./client $USERNAME3 $IP $PORT < $FIFO3 > $OUTPUT_DIR/${TESTCASE}_client3_output 2>&1 &
CLIENT3PID=$!
echo "client3 pid = $CLIENT3PID"
sleep $START_WAITING_TIME

## Client 3 send message "The user name 'user2' is reused by client 3."
echo "The user name 'user2' is reused by client 3." > $FIFO3
sleep $MSG_WAITING_TIME

## Client 3 exits
echo "Kill client3, pid = $CLIENT3PID"
kill $CLIENT3PID
echo "Client3 exits"
sleep $MSG_WAITING_TIME

### Kill the clients and the server
echo "Kill client1, pid = $CLIENT1PID"
kill $CLIENT1PID
echo "Client1 exits"
sleep $MSG_WAITING_TIME

echo "Kill server, pid = $SERVERPID"
kill $SERVERPID
echo "Server exits"
sleep $MSG_WAITING_TIME

echo "================================ Testcase 3 ends ====================================================="







# ==== Test case 4 ====
# Server rejects the client because it exceeds the maximum number of clients allowed
# Maximum number of clients allowed: 2
# Scenario:
#   Server starts
#   Client 1 starts and send JOIN with username "user1"
#   Client 2 starts and send JOIN with username "user2"
#   Client 1 send message "Hi, I'm user1"
#   Client 2 send message "Hello, I'm user2"
#   Client 3 starts and send JOIN with username "user3" (failed because of the full chat room)
#   Client 1 exits
#   Client 2 exits
#   Server exits

TESTCASE="testcase4"
MAXCLIENTS=2
USERNAME1="user1"
USERNAME2="user2"
USERNAME3="user3"

echo "================================ Testcase 4 starts ====================================================="
echo "Testcase 4: Server allows a previously used username to be reused"

### Start server, the output will be stored in $OUTPUT_DIR/${TESTCASE}_server_output
echo "Start server..."
./server $IP $PORT $MAXCLIENTS > $OUTPUT_DIR/${TESTCASE}_server_output 2>&1 &
SERVERPID=$!
echo "server pid = $SERVERPID"


### Start clients. The input will redirect to fifo and the output will be stored in $OUTPUT_DIR/${TESTCASE}_clientx_output
echo "Start client1..."
./client $USERNAME1 $IP $PORT < $FIFO1 > $OUTPUT_DIR/${TESTCASE}_client1_output 2>&1 &
CLIENT1PID=$!
echo "client1 pid = $CLIENT1PID"
sleep $START_WAITING_TIME

## Client 1 send message "Hi, I'm user1"
echo "Hi, I'm user1" > $FIFO1
sleep $MSG_WAITING_TIME

echo "Start client2..."
./client $USERNAME2 $IP $PORT < $FIFO2 > $OUTPUT_DIR/${TESTCASE}_client2_output 2>&1 &
CLIENT2PID=$!
echo "client2 pid = $CLIENT2PID"
sleep $START_WAITING_TIME

## Client 2 send message "Hello, I'm user2"
echo "Hello, I'm user2" > $FIFO2
sleep $MSG_WAITING_TIME

## Client 3 starts and send JOIN with username "user3"
echo "Start client3..."
./client $USERNAME3 $IP $PORT < $FIFO3 > $OUTPUT_DIR/${TESTCASE}_client3_output 2>&1 &
CLIENT3PID=$!
echo "client3 pid = $CLIENT3PID"
sleep $START_WAITING_TIME

## Client 3 output
echo "" > $FIFO3
sleep $MSG_WAITING_TIME

## Client 1 exits
echo "Kill client1, pid = $CLIENT1PID"
kill $CLIENT1PID
echo "Client1 exits"
sleep $MSG_WAITING_TIME

## Client 2 exits
echo "Kill client2, pid = $CLIENT2PID"
kill $CLIENT2PID
echo "Client2 exits"
sleep $MSG_WAITING_TIME

### Kill the server
echo "Kill server, pid = $SERVERPID"
kill $SERVERPID
echo "Server exits"
sleep $MSG_WAITING_TIME

echo "================================ Testcase 4 ends ====================================================="




# ==== Test case 5 ====
# ONLINE/OFFLINE/ACK/NAK test
# Maximum number of clients allowed: 3
# Scenario:
#   Server starts
#   Client 1 starts and send JOIN with username "user1"
#   Client 2 starts and send JOIN with username "user2"
#   Client 3 starts and send JOIN with username "user3"
#   Client 3 exits
#   Client 4 starts and send JOIN with username "user2"
#   Client 1 exits
#   Client 2 exits
#   Server exits

TESTCASE="testcase5"
MAXCLIENTS=3
USERNAME1="user1"
USERNAME2="user2"
USERNAME3="user3"

echo "================================ Testcase 5 starts ====================================================="
echo "Testcase 5: ONLINE/OFFLINE/ACK/NAK test"

### Start server, the output will be stored in $OUTPUT_DIR/${TESTCASE}_server_output
echo "Start server..."
./server $IP $PORT $MAXCLIENTS > $OUTPUT_DIR/${TESTCASE}_server_output 2>&1 &
SERVERPID=$!
echo "server pid = $SERVERPID"


### Start clients. The input will redirect to fifo and the output will be stored in $OUTPUT_DIR/${TESTCASE}_clientx_output
echo "Start client1..."
./client $USERNAME1 $IP $PORT < $FIFO1 > $OUTPUT_DIR/${TESTCASE}_client1_output 2>&1 &
CLIENT1PID=$!
echo "client1 pid = $CLIENT1PID"
sleep $START_WAITING_TIME

## Client 1 output
echo "Hi, I'm user1" > $FIFO1
sleep $MSG_WAITING_TIME

echo "Start client2..."
./client $USERNAME2 $IP $PORT < $FIFO2 > $OUTPUT_DIR/${TESTCASE}_client2_output 2>&1 &
CLIENT2PID=$!
echo "client2 pid = $CLIENT2PID"
sleep $START_WAITING_TIME

## Client 2 output
echo "Hello, I'm user2" > $FIFO2
sleep $MSG_WAITING_TIME

## Client 3 starts and send JOIN with username "user3"
echo "Start client3..."
./client $USERNAME3 $IP $PORT < $FIFO3 > $OUTPUT_DIR/${TESTCASE}_client3_output 2>&1 &
CLIENT3PID=$!
echo "client3 pid = $CLIENT3PID"
sleep $START_WAITING_TIME

## Client 3 output
echo "Howdy, I'm user3" > $FIFO3
sleep $MSG_WAITING_TIME

## Client 3 exits
echo "Kill client3, pid = $CLIENT3PID"
kill $CLIENT3PID
echo "Client3 exits"
sleep $MSG_WAITING_TIME

## Client 4 starts and send JOIN with username "user2"
echo "Start client4..."
./client $USERNAME2 $IP $PORT < $FIFO3 > $OUTPUT_DIR/${TESTCASE}_client4_output 2>&1 &
CLIENT4PID=$!
echo "client4 pid = $CLIENT4PID"
sleep $START_WAITING_TIME

## Client 4 output
echo "" > $FIFO3
sleep $MSG_WAITING_TIME

## Client 4 exits
echo "Kill client4, pid = $CLIENT4PID"
kill $CLIENT4PID
echo "Client4 exits"
sleep $MSG_WAITING_TIME

## Client 1 exits
echo "Kill client1, pid = $CLIENT1PID"
kill $CLIENT1PID
echo "Client1 exits"
sleep $MSG_WAITING_TIME

## Client 2 exits
echo "Kill client2, pid = $CLIENT2PID"
kill $CLIENT2PID
echo "Client2 exits"
sleep $MSG_WAITING_TIME

### Kill the server
echo "Kill server, pid = $SERVERPID"
kill $SERVERPID
echo "Server exits"
sleep $MSG_WAITING_TIME

echo "================================ Testcase 5 ends ====================================================="

