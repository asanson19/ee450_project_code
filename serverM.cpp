
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>  // For strtok
#include <string>
#include <random>


//UDP Port
#define PORT_UDP 24099 //Port# is 24000 + last 3 digits of student_id (2336080_099_) -> 24099
//TCP Port with client
#define PORT_CLIENT 25099 //Port# is 25000 + last 3 digits of student_id (2336080_099_) -> 25099
//TCP Port with monitor
#define PORT_MONITOR 26099 //Port# is 26000 + last 3 digits of student_id (2336080_099_) -> 26099

#define MAXLINE 1024

int udp_sockfd, tcp_cl_socket_fd, tcp_mt_socket_fd;
struct sockaddr_in tcp_cl_address, tcp_mt_address, udp_server_addr, udp_client_addr;

// returns status of binding
int create_and_bind_udp(int& udp_sockfd, struct sockaddr_in & udp_server_addr, struct sockaddr_in& udp_client_addr) {
    // Boot Up Server-M as UDP Client. -> Reference: "Sample UDP Client Code" from geeksforgeeks provided in Brightspace. 
    // char buffer[MAXLINE];
    // char *hello = "Hello from udp client (Server-M)";

    // Creating socket file descriptor
    if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        return -1;
    }

    memset(&udp_server_addr, 0, sizeof(udp_server_addr));
    memset(&udp_client_addr, 0, sizeof(udp_client_addr));

    // Binding the client to a specific local address and port
    udp_client_addr.sin_family = AF_INET;
    udp_client_addr.sin_port = htons(PORT_UDP);  // Client's port to listen on
    udp_client_addr.sin_addr.s_addr = INADDR_ANY;  // Or use a specific local IP (e.g., inet_addr("192.168.1.2"))

    if (bind(udp_sockfd, (const struct sockaddr *)&udp_client_addr, sizeof(udp_client_addr)) < 0) {
        perror("bind failed udp");
    }
    
    return 0;
}

// returns status of binding
int create_and_bind_tcp(int& tcp_socket_fd, int port, struct sockaddr_in& tcp_address){
    // Set up client TCP connection -> Reference: "Sample Server Code" from geeksforgeeks provided in Brightspace. 
    
    // Creating socket
    if ((tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        return -1;
    }

    // Assign IP, PORT 
    tcp_address.sin_family = AF_INET;
    tcp_address.sin_addr.s_addr = INADDR_ANY;
    tcp_address.sin_port = htons(port);

    // Binding to port
    if (bind(tcp_socket_fd, (struct sockaddr *)&tcp_address,
    sizeof(tcp_address))<0){
        perror("bind failed tcp");
        exit(EXIT_FAILURE);
    }

    if ((listen(tcp_socket_fd, 5)) != 0) { 
        exit(0); 
    } 

    return 0;
}

char* get_client_request(int& connfd, int sockfd, struct sockaddr_in& cli){
    // Accept the data packet from client and verification 
    if (connfd < 0) { 
        printf("server accept failed...\n\n"); 
        exit(0); 
    } 

    char *buff = (char*)(malloc(1024 * sizeof(char))); 
    // read the message from client and copy it in buffer 
    bzero(buff, 1024);
    read(connfd, buff, 1024); 

    return buff;
}

int send_response_to_client(int connfd, char* operation, char* message){
    strcat(operation, message);
    write(connfd, operation, 100); 
    return 0;
}

char* encrypt_message(char* message){
    char* encrypted = (char*)malloc(1024 * sizeof(char));
    int i = 0;
    while (message[i]){
        printf("%c",message[i]);
        char current_letter = message[i];
        int ascii_val = int(current_letter);
        int new_ascii_val = ascii_val; //For non-letter or non-number cases the ascii value remains unchanged. 
        char* encrypted_letter = (char*)malloc(1 * sizeof(char));

        // General approach for all cases: encrypted ascii value -> (current - start + shift) mod total + start
        
        // Upper case letters
        if(ascii_val >= 65 && ascii_val <= 90){
            new_ascii_val = ((ascii_val - 65 + 3)%26) + 65;      
        }

        // Lower case letters
        else if(ascii_val >= 97 && ascii_val <= 122){
            new_ascii_val = ((ascii_val - 97 + 3)%26) + 97;      
        }

        // Numbers from 0-9
        else if(ascii_val >= 48 && ascii_val <= 57){
            new_ascii_val = ((ascii_val - 48 + 3)%10) + 48;      
        }

        encrypted_letter[0] = (char)(new_ascii_val); 
        strcat(encrypted, encrypted_letter);
        i++;
    }
    
    return encrypted;
    
}

char* decrypt_message(char* encrypted){
    char* decrypted = (char*)malloc(1024 * sizeof(char));
    int i = 0;
    while (encrypted[i]){
        char current_letter = encrypted[i];
        int ascii_val = int(current_letter);
        int new_ascii_val = ascii_val; //For non-letter or non-number cases the ascii value remains unchanged. 
        char* decrypted_letter = (char*)malloc(1 * sizeof(char));

        // General approach for all cases: decrypted ascii value -> (current - start - shift + total) mod total + start 
        
        // Upper case letters
        if(ascii_val >= 65 && ascii_val <= 90){
            new_ascii_val = ((ascii_val - 65 - 3 + 26)%26) + 65;      
        }

        // Lower case letters
        else if(ascii_val >= 97 && ascii_val <= 122){
            new_ascii_val = ((ascii_val - 97 - 3 + 26)%26) + 97;      
        }

        // Numbers from 0-9
        else if(ascii_val >= 48 && ascii_val <= 57){
            new_ascii_val = ((ascii_val - 48 - 3 + 10)%10) + 48;      
        }

        decrypted_letter[0] = (char)(new_ascii_val); 
        strcat(decrypted, decrypted_letter);
        i++;
    }
    return decrypted;
    
}


int process_check_wallet(char* username, bool suppress_messages=false, bool transfer=false){
    // encrypt username
    char* encrypted_username = encrypt_message(username);

    // Sending data through udp
    char transactions[10000] = "";
    bool found = false;
    int i = 1;
    while(true){
        // Server details
        int port_cur = 20000 + i*1000 + 99;
        udp_server_addr.sin_family = AF_INET;
        udp_server_addr.sin_port = htons(port_cur);
        udp_server_addr.sin_addr.s_addr = INADDR_ANY;

        // Construct message
        char operation[150] = "check_wallet ";
        strcat(operation, encrypted_username);

        // Send the UDP request to the server
        sendto(udp_sockfd, (const char *)operation, 150, 
             0, (const struct sockaddr *) &udp_server_addr,  
                 sizeof(udp_server_addr)); 

        char server_name = (char)64+i;
        if(!suppress_messages){
            printf("The main server sent a request to server %c\n\n", server_name); 
        }

        // Get Response from server
        char buffer[MAXLINE*10]; 
        socklen_t len; 
        int n = recvfrom(udp_sockfd, (char *)buffer, MAXLINE,  
                MSG_WAITALL, (struct sockaddr *) &udp_server_addr, 
                &len); 
        buffer[n] = '\0'; 

        if(!suppress_messages && !transfer){
            printf("The main server received transactions from Server %c using UDP over %d \n\n", server_name,port_cur);
        }
        if(!suppress_messages && transfer){
            printf("The main server received the feedback from Server %c using UDP over port %d \n\n", server_name,port_cur);
        }
        //  Store valid response in the transactions buffer
        if(strcmp(buffer,"none") != 0){
            strcat(transactions, buffer);
            strcat(transactions, " ");
            found = true;
        }
        i++;
        if(i==4){  // Stop after sending requests to all 3 servers
            break;
        }
    }

    // If no transaction data was found, return -1
    if (!found) {
        return -1;
    }

    // Start with the initial balance of 1000 txcoins
    int balance = 1000;
    char* serial_num = strtok(transactions, " ");
    while(serial_num != NULL){
        char* sender = decrypt_message(strtok(NULL, " "));
        char* receiver = decrypt_message(strtok(NULL, " "));
        char* amount_str = decrypt_message(strtok(NULL, " "));
        if (receiver != NULL && amount_str != NULL) {
            int amount = atoi(amount_str);  // Convert the amount to integer
            if (strcmp(sender, username) == 0) {
                // If the user is the sender, subtract the amount from the balance
                balance -= amount;
            }
            if (strcmp(receiver, username) == 0) {
                // If the user is the receiver, add the amount to the balance
                balance += amount;
            }
        }
        serial_num = strtok(NULL, " ");
    }
    return balance;
}

// Get the max serial number from backend server transactions
int get_max_serial_num(){
    int max_num = 0;
    int i = 1;
    while(true){
        // Server details
        int port_cur = 20000 + i*1000 + 99;
        udp_server_addr.sin_family = AF_INET;
        udp_server_addr.sin_port = htons(port_cur);
        udp_server_addr.sin_addr.s_addr = INADDR_ANY;

        // Construct message
        char operation[150] = "check_serial_num ";

        // Send the UDP request to the server
        sendto(udp_sockfd, (const char *)operation, 150, 
             0, (const struct sockaddr *) &udp_server_addr,  
                 sizeof(udp_server_addr)); 

        // Get Response from server
        char buffer[MAXLINE*10]; 
        socklen_t len; 
        int n = recvfrom(udp_sockfd, (char *)buffer, MAXLINE,  
                MSG_WAITALL, (struct sockaddr *) &udp_server_addr, 
                &len); 
        buffer[n] = '\0'; 
        
        // Check if the backend server's max serial num is max
        int new_max = atoi(buffer);
        if (new_max > max_num){
            max_num = new_max;
        }
        i++;
        if(i==4){  // Stop after sending requests to all 3 servers
            break;
        }
    }
    return max_num;
}



int send_new_transaction(int serial_num, char *sender, char *receiver, int amount, int random_server){
    char* message = (char*)malloc(1024 * sizeof(char));  // Allocate enough memory
    if (message == nullptr) {
        perror("Memory allocation failed");
        return -1;  //  if malloc fails
    }
    // Initialize the message to an empty string
    message[0] = '\0';  // Or use memset(message, 0, 1024); to clear the buffer

    // Concatenate sender and receiver
    strcat(message, sender);
    strcat(message, " ");
    strcat(message, receiver);
    strcat(message, " ");

    // Allocate memory and format the amount
    char str_amount[20]; 
    snprintf(str_amount, sizeof(str_amount), "%d", amount);  // Format the amount as a string
    strcat(message, str_amount);  

    // Encrypt the message here (pseudo code)
    char* encrypted_message = encrypt_message(message);  // call encryption function here

    // Add the unencrypted serial number to the final message
    char final_message[2000];  
    snprintf(final_message, sizeof(final_message), "%d", serial_num); 
    strcat(final_message, " "); 
    strcat(final_message, encrypted_message); 

    // Free dynamically allocated memory
    free(message);

    // Server details
    int port_cur = 20000 + random_server*1000 + 99;
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_port = htons(port_cur);
    udp_server_addr.sin_addr.s_addr = INADDR_ANY;

    // Construct message
    char operation[150] = "add_transaction ";
    strcat(operation, final_message);

    // Send the UDP request to the server
    sendto(udp_sockfd, (const char *)operation, 150, 
            0, (const struct sockaddr *) &udp_server_addr,  
                sizeof(udp_server_addr)); 

    // Get Response from server
    char buffer[MAXLINE*10]; 
    socklen_t len; 
    int n = recvfrom(udp_sockfd, (char *)buffer, MAXLINE,  
            MSG_WAITALL, (struct sockaddr *) &udp_server_addr, 
            &len); 
    buffer[n] = '\0'; 
    // Check if the backend server's max serial num is max
    int status_code = atoi(buffer);
    return status_code;
}


void process_transfer_coins(char* sender, char* receiver, int amount, int connfd){

    // Get balances and ensure the users exist
    int sender_balance = process_check_wallet(sender, false, true);
    int receiver_balance = process_check_wallet(receiver, true, true);


    if(sender_balance == -1 && receiver_balance == -1){
        // send to client that no balance
        char status_code[1024] = "both_no_exist ";
        send_response_to_client(connfd, status_code, " ");
    }
    // sender doesnt exist
    else if(sender_balance == -1){
        // send to client that no balance
        char status_code[1024] = "one_no_exist ";
        char message[100];
        snprintf(message, 100, "%s", sender);
        send_response_to_client(connfd, status_code, message);
    }
    // receiver doesnt exist
    else if(receiver_balance == -1){
        // send to client that no balance
        char status_code[1024] = "one_no_exist ";
        char message[100];
        snprintf(message, 100, "%s", receiver);
        send_response_to_client(connfd, status_code, message);
    }
    else if(sender_balance < amount){
        // send to client that no balance
        char status_code[1024] = "insufficient_funds ";
        char message[100];
        snprintf(message, 100, "%d", sender_balance);
        send_response_to_client(connfd, status_code, message);
    }else{
        // Conduct transaction 
        // Check all the servers for max serial number 
        int new_serial_num = get_max_serial_num() + 1;

        // Pick a server at random and send the transaction to the server picked
        int random_server = (rand() % 3) + 1;

        if(send_new_transaction(new_serial_num,sender, receiver, amount, random_server) < 0){
            printf("Error adding the new transaction\n\n");
        }

        // 3. check wallet for sender with suppress messages to check balance
        int sender_balance_new = process_check_wallet(sender, true, true);
        char status_code[1024] = "success ";
        char message[100];
        snprintf(message, 100, "%d", sender_balance_new);
        send_response_to_client(connfd, status_code, message);
    }

}



int main(){
    /* PHASE 1: 
        a). Boot up Server-M. 
        b). Connect client to the main server (Server-M). -> Connection Type: TCP
        c). Connect monitor to the main server (Server-M). -> Connection Type: TCP
    */


    if(create_and_bind_udp(udp_sockfd, udp_server_addr, udp_client_addr) < 0) {
        exit(EXIT_FAILURE);
    }

    if(create_and_bind_tcp(tcp_cl_socket_fd, PORT_CLIENT, tcp_cl_address) < 0){
        exit(EXIT_FAILURE);
    }

    if(create_and_bind_tcp(tcp_mt_socket_fd, PORT_MONITOR, tcp_mt_address) < 0){
        exit(EXIT_FAILURE);
    }

    printf("The main server is up and running.\n\n");
    while(true){
        // Accept the data packet from client and verification 
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int connfd = accept(tcp_cl_socket_fd, (sockaddr*)&cli, &len); 

        char* message = get_client_request(connfd, tcp_cl_socket_fd, cli);

        // figure out operation type and display message
        char* operation = strtok(message, " ");  // First token is the operation

        if (strcmp(operation, "check_wallet") == 0) {
            // Handle check_wallet operation
            char* username = strtok(nullptr, " ");  // Get the next token (username)
            printf("The main server received input=%s from the client using TCP over %d\n\n", username, PORT_CLIENT);
            
            int balance = process_check_wallet(username);
            char operation[1024] = "check_wallet ";
            char message[100];
            snprintf(message, 100, "%d", balance);
            send_response_to_client(connfd, operation, message);
            printf("The main server sent the current balance to the client.\n\n");
        } else {
            // Handle txcoins operation
            char* sender = strtok(nullptr, " ");  // Get sender's username
            char* receiver = strtok(nullptr, " ");  // Get receiver's username
            int amount = atoi(strtok(nullptr, " "));  // Get amount to transfer
            printf("The main server received from %s to transfer %d coins to %s using TCP over %d\n\n", sender, amount, receiver, PORT_CLIENT);
            
            process_transfer_coins(sender, receiver, amount, connfd);
            printf("The main server sent the result of the transaction to the client. \n\n");
        }
            
    }
    
    close(tcp_cl_socket_fd);
    close(tcp_mt_socket_fd);
    close(udp_sockfd);
    return 0;
}
