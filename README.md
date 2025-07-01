# EE450 Socket Programming Project - Txchain Blockchain System

## Student Information
- **Name:** Ana Sanson
- **USC ID:** 2336080099
- **USC Username:** asanson

## What I Have Done

### Completed Features
- **Phase 1**: TCP connection establishment between client/monitor and main server
- **Phase 2**: UDP communication between main server and backend servers (CHECK WALLET and TXCOINS operations)
- **Phase 3**: TXLIST operation for sorted transaction list generation
- **Phase 4** - Extra Credit: Advanced multi-layer encryption algorithm

### Extra Credit Implementation
I have implemented an advanced encryption algorithm that provides enhanced security through a multi-step process:

**Algorithm Overview:**
1. **Step 1 - ASCII Conversion with Length Prefix:** Each alphanumeric character is converted to its ASCII value with the number of digits prepended
2. **Step 2 - Double Number-to-ASCII Conversion:** The result is passed through ASCII conversion twice
3. **Step 3 - Pattern-Based Transformation:** Final transformation based on even/odd patterns and position

**Example:**
- Original: "Martin"
- Basic Encryption: "Pduwlq"
- Advanced Encryption: "F1Jlnpr1Vhhjlj5V5Zf3Jh5T5Xv5Hf5R59Ttfd5P5T59"

**Usage:** Run servers with command-line argument to enable advanced encryption:
```bash
./serverM advanced
```

## Code Files Description

### Main Components
1. **serverM.cpp** - Main server that coordinates all operations
   - Manages TCP connections with client (port 25099) and monitor (port 26099)
   - Communicates with backend servers via UDP (port 24099)
   - Performs balance calculations and transaction processing
   - Handles both basic and advanced encryption/decryption
   - Generates txchain.txt for TXLIST operation

2. **serverA.cpp** - Backend server A
   - Manages block1.txt file
   - Listens on UDP port 21099
   - Responds to wallet queries, serial number checks, and transaction additions
   - Stores encrypted transactions

3. **serverB.cpp** - Backend server B
   - Manages block2.txt file
   - Listens on UDP port 22099
   - Same functionality as serverA but for block2.txt

4. **serverC.cpp** - Backend server C
   - Manages block3.txt file
   - Listens on UDP port 23099
   - Same functionality as serverA but for block3.txt

5. **client.cpp** - Client application
   - Connects to main server via TCP (port 25099)
   - Sends CHECK WALLET and TXCOINS requests
   - Displays balance and transaction results

6. **monitor.cpp** - Monitor application
   - Connects to main server via TCP (port 26099)
   - Sends TXLIST requests
   - Receives confirmation of sorted transaction list generation

7. **advanced_encryption.cpp** - Standalone encryption/decryption utility (Extra Credit)
   - Command-line tool for testing the advanced encryption algorithm
   - Can encrypt or decrypt strings using the advanced multi-layer algorithm
   - Usage: `./advanced_encryption encrypt <string>` or `./advanced_encryption decrypt <string>`

8. **Makefile** - Compilation and build management
   - Supports make all, make clean, and individual component compilation
   - Includes compilation for advanced_encryption utility

## Message Formats

### Client to Main Server (TCP)
1. **CHECK WALLET:** `check_wallet <username>`
2. **TXCOINS:** `tx_coins <sender> <receiver> <amount>`

### Monitor to Main Server (TCP)
1. **TXLIST:** `txlist`

### Main Server to Backend Servers (UDP)
1. **Query for transactions:** `check_wallet <encrypted_username>`
2. **Check max serial number:** `check_serial_num`
3. **Add new transaction:** `add_transaction <serial_no> <encrypted_sender> <encrypted_receiver> <encrypted_amount>`
4. **Get all transactions:** `txlist`

### Backend Servers to Main Server (UDP)
1. **Transaction data:** `<serial_no> <encrypted_sender> <encrypted_receiver> <encrypted_amount>` (space-separated)
2. **No transactions found:** `none`
3. **Max serial number:** `<number>`
4. **Success code:** `0` (success) or `-1` (failure)

### Response Formats to Client
1. **Balance response:** `check_wallet <balance>`
2. **Transaction success:** `success <new_balance>`
3. **Insufficient funds:** `insufficient_funds <current_balance>`
4. **User not found:** `sender_no_exist`, `receiver_no_exist`, or `both_no_exist`

## Idiosyncrasies and Known Issues

### Assumptions
- All users start with 1000 txcoins as initial balance
- Transaction serial numbers are sequential starting from 1
- Block files must end with newline character (\n)
- Maximum buffer size is 100024 bytes for UDP communications
- Random server selection for new transaction storage

### Failure Conditions
- Program fails if block files (block1.txt, block2.txt, block3.txt) are not present in the same directory
- Port binding fails if ports are already in use (need to kill zombie processes)
- Socket creation fails if system resources are exhausted
- File operations fail if insufficient permissions

### Special Behaviors
- The main server uses select() for handling multiple TCP connections
- Transactions are randomly distributed among the three backend servers
- The txchain.txt file is regenerated each time TXLIST is called
- Empty username searches return -1 balance to indicate user not found

## Reused Code
- **Beej's Guide to Network Programming:** Socket setup and connection establishment code
  - TCP server setup in serverM.cpp (create_and_bind_tcp function)
  - UDP socket creation in serverM.cpp (create_and_bind_udp function)
  - Client connection code in client.cpp and monitor.cpp
  - Citation: Beej's Guide to Network Programming (https://beej.us/guide/bgnet/)

- **GeeksforGeeks Socket Programming Examples:** Referenced for basic socket operations
  - Sample UDP Server Code used as reference in serverA.cpp
  - Sample TCP Client Code used as reference in client.cpp
  - Sample TCP Server Code used as reference in serverM.cpp

## Compilation and Execution

### Compilation
```bash
make all        # Compiles all files including advanced_encryption utility
make clean      # Removes all executables and object files
```

To compile the advanced encryption utility separately:
```bash
g++ -o advanced_encryption advanced_encryption.cpp
```

### Execution Order
1. Start servers in separate terminals (MUST be in this order):
   ```bash
   ./serverM              # For basic encryption
   ./serverM advanced     # For advanced encryption
   ./serverA
   ./serverB
   ./serverC
   ```

2. Run client operations:
   ```bash
   ./client <username>                           # Check wallet
   ./client <sender> <receiver> <amount>         # Transfer coins
   ```

3. Run monitor operation:
   ```bash
   ./monitor TXLIST
   ```

### Port Configuration
Based on USC ID ending in 099:
- Server A: 21099 (UDP)
- Server B: 22099 (UDP)
- Server C: 23099 (UDP)
- Main Server: 24099 (UDP), 25099 (TCP-Client), 26099 (TCP-Monitor)

## Extra Credit Instructions

### Enabling Advanced Encryption
1. Start the main server with the "advanced" argument:
   ```bash
   ./serverM advanced
   ```
2. All other servers and clients will automatically use the advanced encryption protocol

### Testing the Encryption Algorithm
A standalone utility `advanced_encryption` is provided to test the encryption/decryption:

```bash
# Compile the utility
make extra

# Encrypt a string
./advanced_encryption encrypt Martin
# Output: F1Jlnpr1Vhhjlj5V5Zf3Jh5T5Xv5Hf5R59Ttfd5P5T59

# Decrypt a string
./advanced_encryption decrypt F1Jlnpr1Vhhjlj5V5Zf3Jh5T5Xv5Hf5R59Ttfd5P5T59
# Output: Martin

# Encrypt multiple strings
./advanced_encryption encrypt Martin Chinmay 10
# Output: F1Jlnpr1Vhhjlj5V5Zf3Jh5T5Xv5Hf5R59Ttfd5P5T59 F1J5Nprp5J59L3Pn5Z59Jjnl5X5J59Lj5V59Fjj1Nrrtvb5Nl15V F1J3Nrr1V3H7
```

### Advanced Encryption Algorithm Details
The algorithm performs multiple transformations:

1. **ASCII Conversion with Length Encoding:**
   - Each character is converted to ASCII value
   - Length of ASCII representation is prepended
   - Example: 'M' (ASCII 77) → "277"

2. **Double ASCII Transformation:**
   - Each digit is converted to its ASCII representation
   - Process is repeated twice for additional security

3. **Pattern-Based Final Encoding:**
   - Even ASCII values: increment by 1
   - Odd ASCII values with even position: convert to uppercase letter
   - Odd ASCII values with odd position: convert to lowercase letter
   - Position resets every 9 characters

### Preparing Block Files for Extra Credit Testing
When testing the extra credit implementation, you can use the `advanced_encryption` utility to prepare encrypted usernames for the block files:

```bash
# Example: Encrypt usernames for block files
./advanced_encryption encrypt Racheal John Rishil Alice Oliver Rachit Chinmay

# Use the output to create test block files with advanced encryption
```

### Testing Advanced Encryption with Full System
Example test sequence:
```bash
# Terminal 1-4: Start servers with advanced encryption
./serverM advanced
./serverA
./serverB
./serverC

# Terminal 5: Test client operations
./client Martin
./client Martin Chinmay 100

# Terminal 6: Generate transaction list
./monitor TXLIST
```

## Testing
The system follows the required sequence: 2 client operations followed by 1 monitor operation.

Example test sequence:
```bash
./client Chinmay
./client Xiake
./monitor TXLIST
./client Chinmay Martin 20
./client Martin
./monitor TXLIST
```
