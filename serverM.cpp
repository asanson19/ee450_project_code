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
#include <errno.h>
#include <netdb.h>
#include <sys/wait.h>
#include <algorithm>  // For sorting
#include <cstdlib>    // For atoi
#include <fstream>

//UDP Port
#define PORT_UDP 24099 //Port# is 24000 + last 3 digits of student_id (2336080_099_) -> 24099
//TCP Port with client
#define PORT_CLIENT 25099 //Port# is 25000 + last 3 digits of student_id (2336080_099_) -> 25099
//TCP Port with monitor
#define PORT_MONITOR 26099 //Port# is 26000 + last 3 digits of student_id (2336080_099_) -> 26099

#define MAXLINE 100024

int udp_sockfd, tcp_cl_socket_fd, tcp_mt_socket_fd;
struct sockaddr_in tcp_cl_address, tcp_mt_address, udp_server_addr, udp_client_addr;
bool use_advanced_encryption = false;

// returns status of binding
int create_and_bind_udp(int& udp_sockfd, struct sockaddr_in & udp_server_addr, struct sockaddr_in& udp_client_addr) {
    // Boot Up Server-M as UDP Client. -> Reference: "Sample UDP Client Code" from geeksforgeeks provided in Brightspace. 

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
    if (buff == nullptr) {
        perror("Memory allocation failed");
        return NULL;  // Return NULL instead of ""
    }
    // read the message from client and copy it in buffer 
    bzero(buff, 1024);
    int n = read(connfd, buff, 1024); 
    if(n == 0){
        return nullptr;
    }
    buff[n] = '\0';
    return buff;
}

int send_response_to_client(int connfd, char* operation, char* message){
    // Create a new buffer 
    size_t total_len = strlen(operation) + strlen(message) + 1;
    char *buf = (char *)malloc(total_len * sizeof(char));    
    if (buf == NULL) {
        // handle malloc failure
        return -1;
    }
    /* build the whole line safely */
    int n = snprintf(buf, total_len, "%s%s", operation, message);
    if (n < 0 || n >= total_len) {
        // handle snprintf error
        free(buf);
        return -1;
    }
    write(connfd, buf, n); 
    free(buf);
    return 0;
}


/*References for Encryption and Decryption: 
    1. ASCII. American Standard Code for Information… | by codezone | Medium */

char* number_to_ascii(char* numerated_message){
    char* encrypted = (char *)  malloc(strlen(numerated_message)*6*sizeof(char)); 
    encrypted[0] = '\0';
    int i = 0;
    while (numerated_message[i]){
        char current_letter = numerated_message[i];
        int ascii_val = int(current_letter);
        if(ascii_val <48 || ascii_val >122 || (ascii_val > 57 && ascii_val < 65) || (ascii_val > 90 && ascii_val < 97) ){
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(encrypted, current_letter_str);
            i++;
            continue;
        }
        char ascii_val_string[100]; 
        snprintf(ascii_val_string, 100, "%d", ascii_val);
        strcat(encrypted, ascii_val_string);
        i++;
    }
    return encrypted;
}

char* ascii_to_num(char* ascii_message){
    char* decrypted = (char *)  malloc(strlen(ascii_message)*6*sizeof(char)); 
    decrypted[0] = '\0';
    int i = 0;
    while (ascii_message[i]){
        char current_letter = ascii_message[i];
        int ascii_val = int(current_letter);
        if(ascii_val <48 || ascii_val >122 || (ascii_val > 57 && ascii_val < 65) || (ascii_val > 90 && ascii_val < 97) ){
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(decrypted, current_letter_str);
            i++;
            continue;
        }

        // the 2 chars together store ascii value of the number
        int current_char = ascii_message[i] - '0';
        int next_char = ascii_message[i+1] - '0';
        int cur_ascii = (current_char)*10 + (next_char);

        // converting ascii value to the number as a character
        char char_to_add = (char) cur_ascii;
        char cur[2];
        cur[0] = char_to_add;
        cur[1] = '\0';
        strcat(decrypted, cur);
        i+=2;
    }
    return decrypted;
}

char* encrypt_message_advanced(char* message){

    char* current_encrypted = (char *)  malloc(strlen(message)*20*sizeof(char)); 
    current_encrypted[0] = '\0';
    int i = 0;

    // Step 1: iterate through message, change to len+ascii of each letter -> new_message
    while (message[i]){
        // Step 1: Determine ASCII value of each letter
        char current_letter = message[i];
        int ascii_val = int(current_letter);

        // check if alphanumeric
        if(ascii_val <48 || ascii_val >122 || (ascii_val > 57 && ascii_val < 65) || (ascii_val > 90 && ascii_val < 97) ){
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(current_encrypted, current_letter_str);
            i++;
            continue;
        }

        // Step 2: Append number of digits to the front
        char ascii_val_string[100]; 
        snprintf(ascii_val_string, 100, "%d", ascii_val);
        
        int len = strlen(ascii_val_string); // Determine if append 2 or 3
        char append_ascii_val_string[100];
        snprintf(append_ascii_val_string, 100, "%d%s",len,ascii_val_string); // Appends the number to the begginging of the string
        strcat(current_encrypted, append_ascii_val_string);
        i++;
    }

    // printf("step 1: %s \n",current_encrypted);
    // Step 2: new_message -> convert each number's char to ascii
    char* encrypted_twice = number_to_ascii(current_encrypted);
    free(current_encrypted);
    // printf("step 2: %s \n",encrypted_twice);

    // Step 3 (repeat): new_message -> convert each number's char to ascii
    char* encrypted_thrice = number_to_ascii(encrypted_twice);
    // printf("step 3: %s \n",encrypted_thrice);
    free(encrypted_twice);
    
    // Final Step -> convert to lowercase, uppercase or numbers based on even odd
    int cur_num = 0;
    bool cur_even = true;
    int j = 0;
    int pos = 0;
    char* final_encrypted = (char *) malloc(strlen(message)*20*sizeof(char)); 
    final_encrypted[0] = '\0';
    while(encrypted_thrice[j]){
        char current_letter = encrypted_thrice[j];
        int ascii_val = int(current_letter);

        if(ascii_val <48 || ascii_val >122 || (ascii_val > 57 && ascii_val < 65) || (ascii_val > 90 && ascii_val < 97) ){
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(final_encrypted, current_letter_str);
            j++;
            pos = 0;
            continue;
        }
        int current_char = encrypted_thrice[j] - '0';
        int next_char = encrypted_thrice[j+1] - '0';
        cur_num = (current_char)*10 + (next_char);
        char character_to_add = ' ';
        // Case1: number is even : keep number add 1, set cur_eve true
        if(cur_num % 2 == 0){
            cur_even = true;
            int final_ascii_val = cur_num + 1;
            character_to_add = (char)final_ascii_val;
        }
        // Case2: number is odd, but cur_even: Capital number, set cur_eve false
        else if(cur_num % 2 == 1 && cur_even){
            cur_even = false;
            int final_ascii_val = cur_num + 17 + (pos * 2);
            character_to_add = (char)final_ascii_val;
        }
        // Case3: number is odd, cur_even is false: lowercase number, set cur_eve false
        else if(cur_num % 2 == 1 && !cur_even){
            int final_ascii_val = cur_num + 49 + (pos * 2);
            character_to_add = (char)final_ascii_val;
        }

        if(pos == 8){
            pos = 0;
        }else{
            pos++;
        }
        j+=2;

        char char_to_concat[2];
        char_to_concat[0] = character_to_add;
        char_to_concat[1] = '\0';
        strcat(final_encrypted, char_to_concat);
    }

    // printf("step 4: %s \n",final_encrypted);
    free(encrypted_thrice);
    return final_encrypted;
}

char* decrypt_message_advanced(char* encrypted){
    char* decrypted = (char *) malloc(strlen(encrypted)*20*sizeof(char)); 
    decrypted[0] = '\0';
    // int cur_num = 0;
    // bool cur_even = true;
    int j = 0;
    int pos = 0;
    char* cur_decrypted = (char *) malloc(strlen(encrypted)*20*sizeof(char)); 
    cur_decrypted[0] = '\0';
    while(encrypted[j]){
        char current_letter = encrypted[j];
        int ascii_val = int(current_letter);

        char decoded[100]; 
        // Case1: character is a number 
        if(ascii_val > 47 && ascii_val <58){
            int decoded_num = ascii_val -1;
            snprintf(decoded, 100, "%d", decoded_num);
        }
        // Case2: Capital letter
        else if(ascii_val > 64 && ascii_val <91){
            int decoded_num = ascii_val - 17 - (pos * 2);
            snprintf(decoded, 100, "%d", decoded_num);
        }
        // Case3: Lowercase letter
        else if(ascii_val > 96 && ascii_val <123){
            int decoded_num = ascii_val - 49 - (pos * 2);
            snprintf(decoded, 100, "%d", decoded_num);
        }
        else {
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(cur_decrypted, current_letter_str);
            j++;
            pos = 0;
            continue;
        }

        strcat(cur_decrypted, decoded);        
        if(pos == 8){
            pos = 0;
        }else{
            pos++;
        }
        j++;
    }

    // printf("decode 1: %s\n", cur_decrypted);
    char* decrypted_twice = ascii_to_num(cur_decrypted);
    free(cur_decrypted);

    // printf("decode 2: %s\n", decrypted_twice);
    char* decrypted_thrice = ascii_to_num(decrypted_twice);
    free(decrypted_twice);
    // printf("decode 3: %s\n", decrypted_thrice);

    int i = 0;
    while(decrypted_thrice[i]){
        char current_letter = decrypted_thrice[i];
        int ascii_val = int(current_letter);
        if(ascii_val <48 || ascii_val >122 || (ascii_val > 57 && ascii_val < 65) || (ascii_val > 90 && ascii_val < 97) ){
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(decrypted, current_letter_str);
            i++;
            continue;
        }

        // ascii value of '2', next 2 digits are ascii
        if(ascii_val == 50){
            // convert it to ascii
            int tens_num = decrypted_thrice[i+1] - '0';
            int ones_num = decrypted_thrice[i+2] - '0';
            int final_asci_decode = (tens_num)*10 + (ones_num);
            char current_letter = (char)final_asci_decode;
            // add it to decrypted string
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(decrypted, current_letter_str);
            i+=3;
        }else if(ascii_val == 51){
            // convert it to ascii
            int hund_num = decrypted_thrice[i+1] - '0';
            int tens_num = decrypted_thrice[i+2] - '0';
            int ones_num = decrypted_thrice[i+3] - '0';

            int final_asci_decode = hund_num*100+ (tens_num)*10 + (ones_num);
            char current_letter = (char)final_asci_decode;
            // add it to decrypted string
            char current_letter_str[2];
            current_letter_str[0] = current_letter;
            current_letter_str[1] = '\0';
            strcat(decrypted, current_letter_str);
            i+=4;
        }else{
            i++;
        }
    }

    return decrypted;
}


char* encrypt_message(char* message){
    if(use_advanced_encryption){
        return encrypt_message_advanced(message);
    }
    char* encrypted = (char*)malloc(strlen(message) * 2 * sizeof(char));
    if (encrypted == nullptr) {
        perror("Memory allocation failed");
        return message;  // Exit or handle appropriately
    }
    encrypted[0] = '\0';
    int i = 0;
    while (message[i]){
        char current_letter = message[i];
        int ascii_val = int(current_letter);
        int new_ascii_val = ascii_val; //For non-letter or non-number cases the ascii value remains unchanged. 
        // FIX: Use a stack buffer instead of malloc for single character
        char encrypted_letter[2] = {0};  // 2 bytes: 1 for char, 1 for null terminator
        
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
    if(use_advanced_encryption){
        return decrypt_message_advanced(encrypted);
    }
    char* decrypted = (char*)malloc(strlen(encrypted) * 2 * sizeof(char));
    if (decrypted == nullptr) {
        perror("Memory allocation failed");
        return encrypted;  // Exit or handle appropriately
    }
    decrypted[0] = '\0';

    int i = 0;
    while (encrypted[i]){
        char current_letter = encrypted[i];
        int ascii_val = int(current_letter);
        int new_ascii_val = ascii_val; //For non-letter or non-number cases the ascii value remains unchanged. 
        // FIX: Use a stack buffer instead of malloc for single character
        char decrypted_letter[2] = {0};  // 2 bytes: 1 for char, 1 for null terminator
        
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
        char operation[MAXLINE] = "check_wallet ";
        strcat(operation, encrypted_username);

        // Send the UDP request to the server
        sendto(udp_sockfd, (const char *)operation, strlen(operation), 
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

    // Free the encrypted username
    free(encrypted_username);

    // If no transaction data was found, return -1
    if (!found) {
        return -1;
    }

    // Start with the initial balance of 1000 txcoins
    int balance = 1000;
    char transactions_copy[10000];
    strcpy(transactions_copy, transactions);  // FIX: Make a copy to preserve original
    char* serial_num = strtok(transactions_copy, " ");
    while(serial_num != NULL){
        char* sender_enc = strtok(NULL, " ");
        char* receiver_enc = strtok(NULL, " ");
        char* amount_enc = strtok(NULL, " ");
        
        // FIX: Check for NULL before decrypting
        if (sender_enc != NULL && receiver_enc != NULL && amount_enc != NULL) {
            char* sender = decrypt_message(sender_enc);
            char* receiver = decrypt_message(receiver_enc);
            char* amount_str = decrypt_message(amount_enc);
            
            int amount = atoi(amount_str);  // Convert the amount to integer
            if (strcmp(sender, username) == 0) {
                // If the user is the sender, subtract the amount from the balance
                balance -= amount;
            }
            if (strcmp(receiver, username) == 0) {
                // If the user is the receiver, add the amount to the balance
                balance += amount;
            }
            
            // Free decrypted strings
            free(sender);
            free(receiver);
            free(amount_str);
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
        sendto(udp_sockfd, (const char *)operation, strlen(operation), 
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
    char* message = (char*)malloc(MAXLINE * sizeof(char));  // Allocate enough memory
    if (message == nullptr) {
        perror("Memory allocation failed");
        return -1;  //  if malloc fails
    }
    // Initialize the message to an empty string
    message[0] = '\0';  

    // Concatenate sender and receiver
    strcat(message, sender);
    strcat(message, " ");
    strcat(message, receiver);
    strcat(message, " ");

    // Allocate memory and format the amount
    char str_amount[MAXLINE]; 
    snprintf(str_amount, sizeof(str_amount), "%d%c", amount, '\0');  // Format the amount as a string
    strcat(message, str_amount);  

    // Encrypt the message here
    // printf("message: %s \n\n", message);

    char* encrypted_message = encrypt_message(message);  // call encryption function here

    // printf("Encrypted message: %s \n\n", encrypted_message);

    // Add the unencrypted serial number to the final message
    char final_message[MAXLINE];  
    snprintf(final_message, sizeof(final_message), "%d", serial_num); 
    strcat(final_message, " "); 
    strcat(final_message, encrypted_message); 

    // Free dynamically allocated memory
    free(message);
    free(encrypted_message);

    // Server details
    int port_cur = 20000 + random_server*1000 + 99;
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_port = htons(port_cur);
    udp_server_addr.sin_addr.s_addr = INADDR_ANY;

    // Construct message
    char operation[MAXLINE] = "add_transaction ";  
    strcat(operation, final_message);

    // Send the UDP request to the server
    sendto(udp_sockfd, (const char *)operation, strlen(operation), 
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
        char status_code[1024] = "sender_no_exist ";
        char message[100];
        snprintf(message, strlen(sender) + 1, "%s%c", sender,'\0');
        send_response_to_client(connfd, status_code, message);
    }
    // receiver doesnt exist
    else if(receiver_balance == -1){
        // send to client that no balance
        char status_code[1024] = "receiver_no_exist ";
        char message[100];
        snprintf(message, strlen(receiver)+1, "%s%c", receiver,'\0');
        send_response_to_client(connfd, status_code, message);
    }
    else if(sender_balance < amount){
        // send to client that no balance
        char status_code[1024] = "insufficient_funds ";
        char message[100];
        snprintf(message, 100, "%d%c", sender_balance,'\0');
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
        
        snprintf(message, 100, "%d%c", sender_balance_new, '\0');
        send_response_to_client(connfd, status_code, message);
    }

}

struct Transaction {
    int serial_num;     // Serial number of the transaction
    char* sender; // Sender of the transaction
    char* receiver; // Receiver of the transaction
    int amount;         // Amount being transferred
    
    // Constructor to initialize a transaction
    Transaction(int serial,  char* sender,  char* receiver, int amount)
        : serial_num(serial), sender(sender), receiver(receiver), amount(amount) {}
};

// Function to compare transactions based on their serial number
bool compare_transactions(const Transaction& t1, const Transaction& t2) {
    return t1.serial_num < t2.serial_num;
}

int get_all_transactions() {
    char transactions[MAXLINE*15] = "";
    int i = 1;
    while(true){
        // Server details
        int port_cur = 20000 + i*1000 + 99;
        udp_server_addr.sin_family = AF_INET;
        udp_server_addr.sin_port = htons(port_cur);
        udp_server_addr.sin_addr.s_addr = INADDR_ANY;

        // Construct message
        char operation[150] = "txlist ";

        // Send the UDP request to the server
        sendto(udp_sockfd, (const char *)operation, strlen(operation), 
             0, (const struct sockaddr *) &udp_server_addr,  
                 sizeof(udp_server_addr)); 

        // Get Response from server
        char buffer[MAXLINE*10]; 
        socklen_t len; 
        int n = recvfrom(udp_sockfd, (char *)buffer, MAXLINE,  
                MSG_WAITALL, (struct sockaddr *) &udp_server_addr, 
                &len); 
        buffer[n] = '\0'; 

        //  Store valid response in the transactions buffer
        if(strcmp(buffer,"none") != 0){
            strcat(transactions, buffer);
            strcat(transactions, " ");
        }

        // printf("Server %c: %s\n\n", (char)(i+'A'-1), buffer);

        i++;
        if(i==4){  // Stop after sending requests to all 3 servers
            break;
        }
    }

    // printf("Transactions: %s\n\n", transactions);

    std::vector<Transaction> transaction_list;

    FILE* file = fopen("txchain.txt", "w");
    if (file == nullptr) {
        perror("Error opening file");
        return -1;
    }

    // char* decrypted = decrypt_message(transactions);

    // printf("decrypted Transactions: %s\n\n", decrypted);

    char* token = strtok(transactions, "\n");

    while (token != nullptr) {
        // Parse each transaction line
        // Format: serial_num sender receiver amount
        char serial[100];
        char* sender = (char*)malloc(MAXLINE);
        char* receiver = (char*)malloc(MAXLINE); 
        char* amount = (char*)malloc(MAXLINE); 
        
        if (sscanf(token, "%99s %99s %99s %99s", serial, sender, receiver, amount) == 4) {
            char* decrypted_sender = decrypt_message(sender);
            char* decrypted_receiver = decrypt_message(receiver);
            char* decrypted_amount = decrypt_message(amount);
            if (decrypted_sender != nullptr) {  // Check for NULL return
                transaction_list.emplace_back(atoi(serial), decrypted_sender, decrypted_receiver, atoi(decrypted_amount)); 
            }

            else {
                free(sender);
                free(receiver);
                free(amount);
            }
        }
        else {
            // Failed to parse, free allocated memory
            free(sender);
            free(receiver);
            free(amount);
        }
            
        token = strtok(nullptr, "\n");
    }

    // Sort transactions by serial number
    std::sort(transaction_list.begin(), transaction_list.end(), compare_transactions);

    // Write sorted transactions to file
    for (const auto& trans : transaction_list) {
        fprintf(file, "%d %s %s %d\n", trans.serial_num, trans.sender, trans.receiver, trans.amount);
    }

    fclose(file);

    return 0;
}

int main(int argc, char* argv[]){
    /* PHASE 1: 
        a). Boot up Server-M. 
        b). Connect client to the main server (Server-M). -> Connection Type: TCP
        c). Connect monitor to the main server (Server-M). -> Connection Type: TCP
    */
    srand(time(nullptr));
    if (argc == 1){
        printf("No arguments, using character shifting protocols\n\n");
    }else if (argc == 2){
        printf("Using encryption protocol %s.\n\n", argv[1]);
        use_advanced_encryption = true;
    }

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
     /* PHASE 2: 
         Check wallet and Transfer coin operations
    */
    fd_set readfds;
    int max_sd = std::max(tcp_cl_socket_fd, tcp_mt_socket_fd);

    while(true){
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add client and monitor sockets to the set
        FD_SET(tcp_cl_socket_fd, &readfds);
        FD_SET(tcp_mt_socket_fd, &readfds);

        // Check for activity on the sockets
        int activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);

        if (activity < 0) {
            perror("select error");
            continue;
        }

        // Accept the data packet from client and verification 
        if (FD_ISSET(tcp_cl_socket_fd, &readfds)) {

            struct sockaddr_in cli;
            socklen_t len = sizeof(cli);
            int connfd = accept(tcp_cl_socket_fd, (sockaddr*)&cli, &len); 
            if (connfd >=0){
                char* message = get_client_request(connfd, tcp_cl_socket_fd, cli);
                if (message == NULL) {
                    close(connfd);
                    continue;
                }
                
                //Make a copy of the message before using strtok
                char message_copy[1024];
                strncpy(message_copy, message, sizeof(message_copy) - 1);
                message_copy[sizeof(message_copy) - 1] = '\0';
                
                // figure out operation type and display message
                char* operation = strtok(message_copy, " ");  // First token is the operation

                if (operation != NULL && strcmp(operation, "check_wallet") == 0) {
                    // Handle check_wallet operation
                    char* username = strtok(nullptr, " ");  // Get the next token (username)
                    if (username != NULL) {
                        printf("The main server received input=\"%s\" from the client using TCP over %d\n\n", username, PORT_CLIENT);
                        int balance = process_check_wallet(username);
                        char operation_return[1024] = "check_wallet ";
                        char message[100];
                        snprintf(message, 100, "%d", balance);
                        send_response_to_client(connfd, operation_return, message);
                        printf("The main server sent the current balance to the client.\n\n");
                    }
                } else if (operation != NULL) {
                    // Handle txcoins operation
                    char* sender = strtok(nullptr, " ");  // Get sender's username
                    char* receiver = strtok(nullptr, " ");  // Get receiver's username
                    char* amount_str = strtok(nullptr, " ");  // Get amount
                    
                    if (sender != NULL && receiver != NULL && amount_str != NULL) {
                        int amount = atoi(amount_str);  // Get amount to transfer
                        printf("The main server received from %s to transfer %d coins to %s using TCP over %d\n\n", sender, amount, receiver, PORT_CLIENT);
                        
                        process_transfer_coins(sender, receiver, amount, connfd);
                        printf("The main server sent the result of the transaction to the client. \n\n");
                    }
                }
                free(message);  // Free the allocated message
                close(connfd);
            }
        }

        /* PHASE 3: 
         Transaction list operation
        */
        if (FD_ISSET(tcp_mt_socket_fd, &readfds)) {

            // Accepting connection
            struct sockaddr_in cli_monitor;
            socklen_t len_monitor = sizeof(cli_monitor);
            int connfd_monitor = accept(tcp_mt_socket_fd, (sockaddr*)&cli_monitor, &len_monitor); 
            if (connfd_monitor < 0) continue;

            // Get the monitor
            char* message_monitor = get_client_request(connfd_monitor, tcp_mt_socket_fd, cli_monitor);
            if (message_monitor == NULL) {
                close(connfd_monitor);
                continue;
            }
            
            char* operation = strtok(message_monitor, " "); 

            if(operation != NULL && strcmp(operation, "txlist") == 0) {
                printf("The main server received a sorted list request from the monitor using TCP over port %d\n\n", PORT_MONITOR);
                int success = get_all_transactions();
                char message[4];  
                snprintf(message, sizeof(message), " %d", success); 
                send_response_to_client(connfd_monitor, operation, message);
            }else{
                printf("Invalid operation received from monitor\n");
            }
            free(message_monitor);  // Free the allocated message
            close(connfd_monitor);
        }
    }
    
    close(tcp_cl_socket_fd);
    close(tcp_mt_socket_fd);
    close(udp_sockfd);
    return 0;
}