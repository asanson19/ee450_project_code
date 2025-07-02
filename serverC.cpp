// Has acces to block3.txt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>

#define PORT 23099 //Port# is 23000 + last 3 digits of student_id (2336080_099_) -> 23099
#define MAXLINE 1024
#define BLOCK_FILE "block3.txt" // Path to the transaction file


void check_wallet(int sockfd, struct sockaddr_in &cliaddr, socklen_t len, const char* username){
    printf("The Server C received a request from the Main Server.\n\n"); 
    // Open the block1.txt file to check transactions
    FILE* file = fopen(BLOCK_FILE, "r");
    if (file == nullptr) {
        perror("Error opening file");
        return;
    }

    char buffer[MAXLINE*10] = "";  // Initialize buffer with zeros
    char line[MAXLINE];  // To store each line from the file

    char* username_full = (char*)malloc((strlen(username) +2) * sizeof(char));
    username_full[0] = '\0';
    strcat(username_full, username);
    strcat(username_full, " ");
    // Read the file line by line
    while (fgets(line, sizeof(line), file)) {
        // Check if the username is present in the line
        if (strstr(line, username_full) != nullptr) {
            // Append the line to the buffer if the username is found
            strcat(buffer, line);
            strcat(buffer, " ");
        }
    }

    if(strcmp(buffer, "") == 0){
        char* mess = "none";
        strcat(buffer, mess);
    }

    sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr*)&cliaddr, len);
    printf("The Server C finished sending the response to the Main Server.\n\n");
    fclose(file);

}

void send_max_serial_num(int sockfd, struct sockaddr_in &cliaddr, socklen_t len){
    // Open the block1.txt file to check transactions
    FILE* file = fopen(BLOCK_FILE, "r");
    if (file == nullptr) {
        perror("Error opening file");
        return;
    }

    char line[MAXLINE];  // To store each line from the file
    int max_num = -1;
    // Read the file line by line
    while (fgets(line, sizeof(line), file)) {
        // Check if the username is present in the line
        char* serial_num = strtok(line, " ");
        if(atoi(serial_num) > max_num){
            max_num = atoi(serial_num);
        }
    }
    char buffer[MAXLINE*10] = "";  // Initialize buffer with zeros
    snprintf(buffer,sizeof(buffer),"%d",max_num);
    sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr*)&cliaddr, len);
    fclose(file);
}

void add_transaction(char* transaction_info, int sockfd, struct sockaddr_in &cliaddr, socklen_t len){
    int success_code = 0;
    FILE* file = fopen(BLOCK_FILE, "a");
    if (file == nullptr) {
        perror("Error opening file");
        success_code = -1;
    }else{
        fprintf(file, "\n%s\n", transaction_info);
        // Close the file
        fclose(file);
    }
    char buffer[MAXLINE*10] = "";  // Initialize buffer with zeros
    snprintf(buffer,sizeof(buffer),"%d",success_code);
    sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr*)&cliaddr, len);
}

void send_all_transactions(int sockfd, struct sockaddr_in &cliaddr, socklen_t len){
    FILE* file = fopen(BLOCK_FILE, "r");
    if (file == nullptr) {
        perror("Error opening file");
        return;
    }

    char buffer[MAXLINE*10] = "\n";  // Initialize buffer with zeros
    char line[MAXLINE];  // To store each line from the file

    // Read the file line by line
    while (fgets(line, sizeof(line), file)) {
        if (strlen(buffer) + strlen(line) + 2 < sizeof(buffer)) {
            strcat(buffer, line);
            strcat(buffer, "\n");
        } else {
            break; // prevent overflow
        }        
    }

    if(strcmp(buffer, "") == 0){
        char* mess = "none";
        strcat(buffer, mess);
    }
    sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr*)&cliaddr, len);
    fclose(file);
}

int main(){
     /* PHASE 1: 
         Boot up server (C). 
            Reference: "Sample UDP Server Code" from geeksforgeeks provided in Brightspace. 
    */
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;


    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { //using IPv4
        perror("socket creation failed"); //error message if socket creation fails
        exit(EXIT_FAILURE); 
    }

    // Clearing Server and Client memory
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 

    // Server Info
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
    
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Booting up message:
    printf("The Server C is up and running using UDP on port %d.\n\n", PORT);
    
    // Actively running after boot up (unless stopped manually)
    while(true){
        socklen_t len; //
        int n; 
        len = sizeof(cliaddr);  //len is value/result 
    
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
        buffer[n] = '\0'; 
        // Extract operation type and username
        char* operation = strtok(buffer, " ");

        if (operation != nullptr && strcmp(operation, "check_wallet") == 0) {
            // If the operation is "check_wallet", check the user's wallet
            char* username = strtok(nullptr, " ");  // Extract the username
            check_wallet(sockfd, cliaddr, len, username);
        } else if (operation != nullptr && strcmp(operation, "check_serial_num") == 0) {
            // If the operation is "check_wallet", check the user's wallet
            send_max_serial_num(sockfd, cliaddr, len);
        }else if(operation != nullptr && strcmp(operation, "add_transaction") == 0) {
            char* transaction_info = (char*)malloc(MAXLINE * 1000 * sizeof(char)); 
            transaction_info[0] = '\0';  // Initialize to empty string
            // Get the first token (sender)
            char* token = strtok(nullptr, " ");
            if (token != nullptr) {
                strcat(transaction_info, token);  // Add sender
            }
            while ((token = strtok(nullptr, " ")) != nullptr) {
                strcat(transaction_info, " ");  // Add space between tokens
                strcat(transaction_info, token);  // Add token (receiver or amount)
            }
            add_transaction(transaction_info, sockfd, cliaddr, len);
            free(transaction_info);
        }else if(operation != nullptr && strcmp(operation, "txlist") == 0) {
            send_all_transactions(sockfd, cliaddr, len);
        }else{
            printf("Invalid operation or unsupported request: %s\n\n", operation);
        }
    }
    
    close(sockfd);
    return 0; 
}