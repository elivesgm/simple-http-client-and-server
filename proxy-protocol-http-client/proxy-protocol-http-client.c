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

      union {
          struct {
              char line[108];
          } v1;
          struct {
              uint8_t sig[12];
              uint8_t ver_cmd;
              uint8_t fam;
              uint16_t len;
              union {
                  struct {  /* for TCP/UDP over IPv4, len = 12 */
                      uint32_t src_addr;
                      uint32_t dst_addr;
                      uint16_t src_port;
                      uint16_t dst_port;
                  } ip4;
                  struct {  /* for TCP/UDP over IPv6, len = 36 */
                       uint8_t  src_addr[16];
                       uint8_t  dst_addr[16];
                       uint16_t src_port;
                       uint16_t dst_port;
                  } ip6;
                  struct {  /* for AF_UNIX sockets, len = 216 */
                       uint8_t src_addr[108];
                       uint8_t dst_addr[108];
                  } unx;
              } addr;
          } v2;
      } hdr;

    bzero(&hdr, sizeof(hdr));
    static const u_char signature[] = "\r\n\r\n\0\r\nQUIT\n";

    memcpy(&hdr.v2.sig, signature, 12);
    hdr.v2.ver_cmd = (0xff & (0x2 << 4 | 0x01));

/*
    /* v4 */
    hdr.v2.fam = 0x11;
    hdr.v2.len = htons(12);
    hdr.v2.addr.ip4.src_addr = 168430090; //10.10.10.10
    hdr.v2.addr.ip4.src_port = 8456;
    hdr.v2.addr.ip4.dst_addr = htonl(174843878);//10.107.231.230
    hdr.v2.addr.ip4.dst_port = htons(80);
*/
   /* v6 */
    char src_addr[16] = {0};
    char dst_addr[16] = {0};

    src_addr[0] = 0x24;
    src_addr[1] = 0xe;
    src_addr[2] = 0x09;
    src_addr[3] = 0x46;
    src_addr[4] = 0x30;
    src_addr[5] = 0x02;
    src_addr[6] = 0x10;
    src_addr[14] = 0x1f;
    src_addr[15] = 0xf7;

    dst_addr[0] = 0x24;
    dst_addr[1] = 0xe;
    dst_addr[2] = 0x09;
    dst_addr[3] = 0x46;
    dst_addr[4] = 0x30;
    dst_addr[5] = 0x02;
    dst_addr[6] = 0x10;
    dst_addr[14] = 0x1f;
    dst_addr[15] = 0xf8;

    hdr.v2.fam = 0x21;
    hdr.v2.len = htons(36);
    //hdr.v2.addr.ip6.src_addr = 168430090; //10.10.10.10
    memcpy(hdr.v2.addr.ip6.src_addr, src_addr, 16);
    hdr.v2.addr.ip6.src_port = 8456;
    //hdr.v2.addr.ip6.dst_addr = htonl(174843878);//10.107.231.85
    memcpy(hdr.v2.addr.ip6.dst_addr, dst_addr, 16);
    hdr.v2.addr.ip6.dst_port = htons(30006);

    int hdr_len = 16+36;

    //int hdr_len = 16+12;
    memcpy(buffer, &hdr, hdr_len);
    static const u_char hdline[] = "GET /index.html HTTP/1.1\r\nHOST: zqz.com\r\nAccept: */*\r\nConnection: Close\r\n\r\n";
    int hdlinelen = strlen(hdline);
    memcpy(buffer + hdr_len, hdline, hdlinelen);
    //strcat(buffer, "GET /index.nginx-debian.html HTTP/1.1\r\nHOST: 192.168.1.4:443\r\nAccept: */*\r\nConnection: Close\r\n\r\n");
/*
    strcat(buffer, "GET /index.html HTTP/1.1\n");
    strcat(buffer, "Host: www.gemao.com\n");
    strcat(buffer, "Content-Type: text/html\n");
    strcat(buffer, "Content-Length: 4\n");
    strcat(buffer, "\n\n");

    strcat(buffer, "abc");
    strcat(buffer, "\r\n\r\n");
    printf("\n\nClient send:\n%s\n", buffer);
*/
    len = send(sockfd, buffer, hdr_len + hdlinelen, 0);
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
