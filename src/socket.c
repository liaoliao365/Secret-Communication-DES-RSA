#include "socket.h"
#include <stdio.h>
void server(){
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
}

void readClient(int *value){
    read( new_socket , value, sizeof(int));
}

void sendClient(int *value){
    send(new_socket , value , sizeof(int) , 0 );
}

void sendClientll(long long int *value){
    send(new_socket , value , sizeof(long long  int) , 0 );
}

void sendClientull(unsigned long long int *value){
    send(new_socket , value , sizeof(unsigned long long  int) , 0 );
}

void sendClientFile(char* FILE_PATH){
    char buffer[1024];
        // 打开文件
    FILE *file = fopen(FILE_PATH, "rb");
    if (file == NULL) {
        error("Error opening file");
    }

    // 读取文件内容并发送
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (send(new_socket, buffer, bytesRead, 0) == -1) {
            error("Error sending file");
        }
    }

    // 关闭文件
    fclose(file);

    printf("File sent successfully.\n");
}

void client(){
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0){
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
}

void readServer(int *value){
    read(sock , value, sizeof(int));
}

void readServerll(long long int *value){
    read(sock , value, sizeof(long long int));
}

void readServerull(unsigned long long int *value){
    read(sock , value, sizeof(unsigned long long int));
}
void readServerFile(char* FILE_PATH){
    char buffer[1024];
    // 打开文件以写入接收到的数据
    FILE *file = fopen(FILE_PATH, "wb");
    if (file == NULL) {
        error("Error opening file");
    }

    // 接收数据并写入文件
    size_t bytesRead;
    while ((bytesRead = read(sock, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, 1, bytesRead, file) != bytesRead) {
            error("Error writing to file");
        }
    }

    // 关闭文件
    fclose(file);

    printf("File received successfully.\n");
}

void sendServer(int *value){
    send(sock , value , sizeof(int) , 0 );
}
