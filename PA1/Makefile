# Use g++ as the compiler
#CXX = g++-10 # local debugging option
CXX = g++

# Set compiler flags
CXXFLAGS = -std=c++11

# Binary path
BIN_PATH = ./bin

# Target object lists
SERVER_OBJ = server.o echoserver.o iosocket.o
CLIENT_OBJ = client.o echoclient.o iosocket.o

# Generate all targets
all: server client $(SERVER_OBJ) $(CLIENT_OBJ) clean

# Generate target: server
server: $(SERVER_OBJ)
	$(CXX) $(CXX_FLAGS) -o $(BIN_PATH)/server $(SERVER_OBJ)

# Generate target: client
client: $(CLIENT_OBJ)
	$(CXX) $(CXX_FLAGS) -o $(BIN_PATH)/client $(CLIENT_OBJ)

# Generate object files
$(CLIENT_OBJ): %.o: %.cpp
$(SERVER_OBJ): %.o: %.cpp

# Clean intermediate files
clean:
	rm -rf $(SERVER_OBJ) $(CLIENT_OBJ)
