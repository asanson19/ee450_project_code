/* PHASE 1: 
    a). Connect client to the main server (Server-M). -> Connection Type: TCP
    b). Connect monitor to the main server (Server-M). -> Connection Type: TCP
    c). Boot up all 4 servers (Server-M, -A, -B, and -C).
*/
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sstream> 
// #include <cstring> // For strcat and strlen

#define PORT 25099

using namespace std;

int create_and_connect_tcp(int& sockfd, struct sockaddr_in& serv_addr){
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(PORT);  
    
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) != 1) {
        perror("inet_pton");
        return -1;
    }

    if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return -1;
    }
    return 0;
}


int main(int argc, char* argv[]){
    // Set up client TCP connection -> Reference: "Sample Client Code" from geeksforgeeks provided in Brightspace.
    int sockfd = 0;
    struct sockaddr_in serv_addr;
    create_and_connect_tcp(sockfd, serv_addr);
    
    // Check type of operation
    if(argc != 2 && argc != 4){
        printf("Please provide valid input arguments.");
        return -1;
    }
    

    char message[1024] = "";

    // std::stringstream ss;

    if(argc == 2){
        // CHECK WALLET
        char* username = argv[1];
        strcat(message,"check_wallet ");
        strcat(message, username);
        // send request 
        write(sockfd, message, strlen(message));
        printf("%s sent a balance enquiry request to the main server. \n\n", username);

        // get response

        char response[1024];
        // bzero(response, sizeof(response));
        
        int n = read(sockfd, response, sizeof(response)-1);   // <- capture length
        if (n <= 0) { 
            printf("An error has occurred");
            exit(EXIT_FAILURE);
        }
        response[n] = '\0';         

        char* operation = strtok(response, " ");
        if(strcmp(operation, "check_wallet")!=0){
            printf("An error has occurred");
            exit(EXIT_FAILURE);
        }
        int amount = atoi(strtok(nullptr, " "));
        if(amount <0){
            printf("%s is not a part of the network.\n", username);
        }else{
            printf("The current balance of %s is: %d txcoins\n", username, amount);
        }
    }
    else{
        // TRANSFER COINS
        char* sender_username = argv[1];
        char* receiver_username = argv[2];
        char* transaction_amount = argv[3];
        strcat(message,"tx_coins ");
        strcat(message, sender_username);
        strcat(message, " ");
        strcat(message, receiver_username);
        strcat(message, " ");
        strcat(message, transaction_amount);

        // Send constructed message and display message
        write(sockfd, message, strlen(message));
        printf("%s has requested to transfer %s txcoins to %s \n\n", sender_username, transaction_amount, receiver_username);
        
        // Get response
        char response[1024];
        int n = read(sockfd, response, sizeof(response)-1);   // <- capture length
        if (n <= 0) { 
            printf("An error has occurred");
            exit(EXIT_FAILURE);
        }
        response[n] = '\0';         
        
        char* status = strtok(response, " ");

        // Display appropriate message
        if(strcmp(status, "insufficient_funds")==0){
            char* balance = strtok(nullptr, " ");
            printf("%s was unable to transfer %s txcoins to %s because of insufficient balance ."
                    "\n\nThe current balance of %s is %s txcoins.\n", 
                    sender_username, transaction_amount, receiver_username, sender_username, balance);
        }else if(strcmp(status, "sender_no_exist")==0){
            printf("Unable to proceed with the transaction as %s is not part of the network.\n", sender_username);
        }else if(strcmp(status, "receiver_no_exist")==0){
            printf("Unable to proceed with the transaction as %s is not part of the network.\n", receiver_username);
        }else if(strcmp(status, "both_no_exist")==0){
            printf("Unable to proceed with the transaction as %s and %s are not part of the network.\n", sender_username, receiver_username);
        }else if(strcmp(status, "success")==0){
            char* sender_balance = strtok(nullptr, " ");
            printf("%s successfully transferred %s txcoins to %s. \n\nThe current balance of %s is : %s txcoins.\n", sender_username, transaction_amount, receiver_username, sender_username, sender_balance);
        }
    }
    


    
    close(sockfd);
    return 0;
}
