#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 4096

#define true 1
#define false 0

int main(void) {
    int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(1080);
    server_sockaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    socklen_t server_sockaddr_size = sizeof(server_sockaddr);

    bind(server_sock, (struct sockaddr*)&server_sockaddr, server_sockaddr_size);
    listen(server_sock, 1);

    while (true) {
        struct sockaddr_in client_sockaddr;
        socklen_t client_sockaddr_size = sizeof(client_sockaddr);
        
        int client_sock = accept(server_sock, (struct sockaddr*)&client_sockaddr, &client_sockaddr_size);
        if (client_sock == -1) {
            continue;
        }
        
        unsigned char* info = (unsigned char*)malloc(15);
        memset(info, 0, 15);

        int bytes = recv(client_sock, info, 16, 0);
        if (bytes != 16) {
            close(client_sock);
            free(info);

            continue;
        }

        unsigned int vn = info[0];
        unsigned int cd = info[1];
        unsigned int dstport = ((unsigned int)info[2] << 8) | (unsigned int)info[3];

        char dstip[16];
        memset(dstip, 0, 16);
        sprintf(dstip, "%d.%d.%d.%d", info[4], info[5], info[6], info[7]);

        unsigned int userid_size = 0;
        while (info[8 + userid_size] != 0x00) {
            userid_size++;

            if (userid_size > 255) {
                break;
            }
        }

        char* userid = malloc(userid_size + 1);
        memcpy(userid, info + 8, userid_size);
        userid[userid_size] = '\0';

        unsigned char nullbyte = info[8 + userid_size];

        if (vn != 4 || cd != 1 || nullbyte != 0x00) {
            close(client_sock);
            free(info);
            
            continue;
        }

        int target_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        struct sockaddr_in target_sockaddr;
        target_sockaddr.sin_family = AF_INET;
        target_sockaddr.sin_port = htons(dstport);
        target_sockaddr.sin_addr.s_addr = inet_addr(dstip);
        socklen_t target_sockaddr_size = sizeof(target_sockaddr);

        int conn = connect(target_sock, (struct sockaddr*)&target_sockaddr, target_sockaddr_size);
        if (conn == -1) {
            close(target_sock);

            unsigned char* error = (unsigned char*)malloc(8);
            memset(error, 0, 8);

            error[0] = 0x00;
            error[1] = 91;

            for (unsigned int i = 2; i < 8; i++) {
                error[i] = info[i];
            }

            send(client_sock, error, 8, 0);
            close(client_sock);
            free(error);
            free(info);

            continue;
        }

        unsigned char* success = (unsigned char*)malloc(8);
        memset(success, 0, 8);

        success[0] = 0x00;
        success[1] = 90;

        for (unsigned int i = 2; i < 8; i++) {
            success[i] = info[i];
        }

        send(client_sock, success, 8, 0);
        free(success);
        free(info);

        fd_set fds;

        int max_fd = (client_sock > target_sock ? client_sock : target_sock) + 1;

        unsigned char* buffer = (unsigned char*)malloc(BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);

        while (true) {
            FD_ZERO(&fds);
            FD_SET(client_sock, &fds);
            FD_SET(target_sock, &fds);

            int returned = select(max_fd, &fds, NULL, NULL, NULL);
            if (returned < 0) {
                break;
            }

            if (FD_ISSET(client_sock, &fds)) {
                ssize_t bytes = read(client_sock, buffer, BUFFER_SIZE);
                if (bytes <= 0) {
                    break;
                }
                write(target_sock, buffer, bytes);
            }

            if (FD_ISSET(target_sock, &fds)) {
                ssize_t bytes = read(target_sock, buffer, BUFFER_SIZE);
                if (bytes <= 0) {
                    break;
                }
                write(client_sock, buffer, bytes);
            }

            memset(buffer, 0, BUFFER_SIZE);
        }

        close(target_sock);
        close(client_sock);
    }

    return 0;
}
