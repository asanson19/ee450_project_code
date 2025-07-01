/* PHASE 1: 
    b). Connect monitor to the main server (Server-M). -> Connection Type: TCP
    c). Boot up all 4 servers (Server-M, -A, -B, and -C).
*/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 26099


int main(int argc, char* argv[]){
    // Set up monitor TCP connection -> Reference: "Sample Client Code" from geeksforgeeks provided in Brightspace.

    int sockfd = 0;
    struct sockaddr_in serv_addr;
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;


    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("The monitor is up and running.\n\n");


    char message[1024] = "";
    if(argc == 2){
        // TXLIST
        char* operation = argv[1];
        if (strcmp(operation,"TXLIST") !=0){
            printf("Invalid operation \n");
            return -1;
        }
        strcat(message,"txlist");
        // send request 
        write(sockfd, message, strlen(message));
        printf("Monitor sent a sorted list request to the main server. \n\n");
        // check if txchain.txt is generated
        
        char response[1024];
        read(sockfd, response, 1024);
        if(strcmp(response,"0")){
            printf("Successfully received a sorted list of transactions from the main server.\n");
        }else{
            printf("Something went wrong.\n");
        }
    }
    
    close(sockfd);
    return 0;
}
