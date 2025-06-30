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

#define PORT 26099


int main(){
    // Set up monitor TCP connection -> Reference: "Sample Client Code" from geeksforgeeks provided in Brightspace.

    int sockfd = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from monitor";
    char buffer[1024] = {0};
    
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

    printf("The monitor is up and running.");
    
    // send(sockfd, hello, strlen(hello) , 0 );
    // printf("Hello message sent from client\n");
    // valread = read( sockfd, buffer, 1024);
    // printf("%s\n",buffer );
    
    close(sockfd);
    return 0;
}
