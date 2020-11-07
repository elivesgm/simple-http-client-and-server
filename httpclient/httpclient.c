#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXBUF 4096


int main(int argc, char **argv)
{
    int sockfd, len;
    struct sockaddr_in client;
    struct sockaddr_in server;
    char buffer[MAXBUF + 1];
    char *pRcv = buffer;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket");
        exit(errno);
    }

    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(atoi(argv[2]));
    if (inet_aton(argv[1], (struct in_addr *) &client.sin_addr.s_addr) == 0) {
        perror(argv[1]);
        exit(errno);
    }

    if (bind(sockfd, (struct sockaddr *) &client, sizeof(client)) != 0) {
        perror("Client bind failed");
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[4]));
    if (inet_aton(argv[3], (struct in_addr *) &server.sin_addr.s_addr) == 0) {
        perror(argv[3]);
        exit(errno);
    }

    if (connect(sockfd, (struct sockaddr *) &server, sizeof(server)) != 0) {
        perror("Connect ");
        exit(errno);
    }
  
      bzero(buffer, MAXBUF + 1);
    //sprintf(buffer, "GET /index.html HTTP/1.1/n");
    strcat(buffer, "GET /index.nginx-debian.html HTTP/1.1\r\nHOST: 192.168.1.4:443\r\nAccept: */*\r\nConnection: Close\r\n\r\n");
    len = send(sockfd, buffer, strlen(buffer), 0);
    if (len < 0) {
        printf("\n\n消息\n'%s'\n发送失败！错误代码是%d，错误信息是'%s'\n", buffer, errno, strerror(errno));
    } else {
        printf("\n\n消息\n'%s'\n发送成功! 共发送了%d个字节！\n", buffer, len);
    }

    bzero(buffer, MAXBUF + 1);
    while (1) {
        len = recv(sockfd, pRcv, MAXBUF, 0);
        pRcv += len;
        if (len == 0 || len == -1) {
            break;
        }
    }

    printf("\n\n接收消息：\n%s\n", buffer);

finish:

    close(sockfd);
    return 0;
}
