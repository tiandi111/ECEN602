# Use g++ as the compiler
CXX = g++

# Set compiler flags
CXXFLAGS = -std=c++11

# Target object lists
SERVER_OBJ = server.o sbcpserver.o protocol.o io.o
CLIENT_OBJ = client.o sbcpclient.o protocol.o io.o

# Generate all targets
all: server client $(SERVER_OBJ) $(CLIENT_OBJ) clean

# Generate target: server
server: $(SERVER_OBJ)
	$(CXX) $(CXX_FLAGS) -o server $(SERVER_OBJ)

# Generate target: client
client: $(CLIENT_OBJ)
	$(CXX) $(CXX_FLAGS) -o client $(CLIENT_OBJ)

# Generate object files
$(CLIENT_OBJ): %.o: %.cpp
$(SERVER_OBJ): %.o: %.cpp

# Clean intermediate files
clean:
	rm -rf $(SERVER_OBJ) $(CLIENT_OBJ)
