# Compiler info
CXX = g++  # Use g++ for C++ code
CXXFLAGS = -Wall -g -std=c++11  # Enable all warnings, debugging, and use C++11 standard
LDFLAGS = -lstdc++   # Link against the C++ standard library
# ARCH_FLAGS = -arch arm64  # For Apple M1/M2 chips (only needed on macOS)


# Executables
EXE_SERVER_M = serverM
EXE_SERVER_A = serverA
EXE_SERVER_B = serverB
EXE_SERVER_C = serverC
EXE_CLIENT = client
EXE_MONITOR = monitor
EXE_ADVANCED_ENCRYPTION = advanced_encryption

# Source Files
SRC_SERVER_M = serverM.cpp
SRC_SERVER_A = serverA.cpp
SRC_SERVER_B = serverB.cpp
SRC_SERVER_C = serverC.cpp
SRC_CLIENT = client.cpp
SRC_MONITOR = monitor.cpp
SRC_ADVANCED_ENCRYPTION = advanced_encryption.cpp

# Object Files
OBJ_SERVER_M = $(SRC_SERVER_M:.cpp=.o)
OBJ_SERVER_A = $(SRC_SERVER_A:.cpp=.o)
OBJ_SERVER_B = $(SRC_SERVER_B:.cpp=.o)
OBJ_SERVER_C = $(SRC_SERVER_C:.cpp=.o)
OBJ_CLIENT = $(SRC_CLIENT:.cpp=.o)
OBJ_MONITOR = $(SRC_MONITOR:.cpp=.o)
OBJ_ADVANCED_ENCRYPTION = $(SRC_ADVANCED_ENCRYPTION:.cpp=.o)


# Default target
all: $(EXE_SERVER_M) $(EXE_SERVER_A) $(EXE_SERVER_B) $(EXE_SERVER_C) $(EXE_CLIENT) $(EXE_MONITOR)

# Extra
extra: $(EXE_ADVANCED_ENCRYPTION)


# Main Server
$(EXE_SERVER_M): $(OBJ_SERVER_M)
	$(CXX) $(OBJ_SERVER_M) -o $(EXE_SERVER_M) $(CXXFLAGS)

# Backend Server A
$(EXE_SERVER_A): $(OBJ_SERVER_A)
	$(CXX) $(OBJ_SERVER_A) -o $(EXE_SERVER_A) $(CXXFLAGS)

# Backend Server B
$(EXE_SERVER_B): $(OBJ_SERVER_B)
	$(CXX) $(OBJ_SERVER_B) -o $(EXE_SERVER_B) $(CXXFLAGS)

# Backend Server C
$(EXE_SERVER_C): $(OBJ_SERVER_C)
	$(CXX) $(OBJ_SERVER_C) -o $(EXE_SERVER_C) $(CXXFLAGS)

# Client
$(EXE_CLIENT): $(OBJ_CLIENT)
	$(CXX) $(OBJ_CLIENT) -o $(EXE_CLIENT) $(CXXFLAGS)

# Monitor
$(EXE_MONITOR): $(OBJ_MONITOR)
	$(CXX) $(OBJ_MONITOR) -o $(EXE_MONITOR) $(CXXFLAGS)

# Advanced encryption
$(EXE_ADVANCED_ENCRYPTION): $(OBJ_ADVANCED_ENCRYPTION)
	$(CXX) $(OBJ_ADVANCED_ENCRYPTION) -o $(EXE_ADVANCED_ENCRYPTION) $(CXXFLAGS)

# Clean up object files and Executables
clean:
	rm -f $(OBJ_SERVER_M) $(OBJ_SERVER_A) $(OBJ_SERVER_B) $(OBJ_SERVER_C) $(OBJ_CLIENT) $(OBJ_MONITOR) $(EXE_SERVER_M) $(EXE_SERVER_A) $(EXE_SERVER_B) $(EXE_SERVER_C) $(EXE_CLIENT) $(EXE_MONITOR) $(EXE_ADVANCED_ENCRYPTION) $(OBJ_ADVANCED_ENCRYPTION)

# Rebuild the project
rebuild: clean all

