# CXX = g++-10
CXX = g++

CXXFLAGS = -std=c++11

BIN_PATH = ./bin

SERVER_OBJ = server.o sbcpserver.o protocol.o io.o
CLIENT_OBJ = client.o sbcpclient.o protocol.o io.o
TEST_OBJ = test/unittest.o protocol.o io.o

all: server client test $(SERVER_OBJ) $(CLIENT_OBJ) $(TEST_OBJ) clean

server: $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN_PATH)/server $(SERVER_OBJ)

client: $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN_PATH)/client $(CLIENT_OBJ)

test: $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN_PATH)/test $(TEST_OBJ)

$(CLIENT_OBJ): %.o: %.cpp
$(SERVER_OBJ): %.o: %.cpp

clean:
	rm -rf $(SERVER_OBJ) $(CLIENT_OBJ) $(TEST_OBJ)
