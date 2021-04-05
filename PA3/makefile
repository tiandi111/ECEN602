# CXX = g++-10
CXX = g++

CXXFLAGS = -std=c++11 -Wno-unused-value

BIN_PATH = ./bin

SERVER_OBJ = server.o tftpserver.o tftpsession.o utils.o io.o
TEST_OBJ = test/unittest.o test/mocklib.o tftpsession.o io.o

all: server test $(SERVER_OBJ) $(TEST_OBJ) clean

server: $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN_PATH)/server $(SERVER_OBJ)

test: $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN_PATH)/test $(TEST_OBJ)

$(CLIENT_OBJ): %.o: %.cpp
$(SERVER_OBJ): %.o: %.cpp

clean:
	rm -rf $(SERVER_OBJ) $(TEST_OBJ)
