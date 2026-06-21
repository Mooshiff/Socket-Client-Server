#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{

    std::cout << "SocketClient Main End." << std::endl;
    return 0;
}

void StartClient_UDP()
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in local_addr;
    bzero(&local_addr, sizeof(sockaddr_in));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(7778);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    int bind_ret = bind(sock_fd, (sockaddr*)(&local_addr), sizeof(sockaddr));
    if (bind_ret == -1)
    {
        perror("bind error");
    }

    // 允许发送广播消息
    int broad_flag = 1;
    int setopt_ret = setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &broad_flag, sizeof(broad_flag));
    if (setopt_ret == -1)
    {
        perror("setsockopt error");
    }

    // 在 SOCK_DGRAM 中是合法的, 随后可使用 send/write 和 recv/read
    // int connect_ret = connect(sock_fd, (sockaddr*)(&remote_addr), sizeof(sockaddr));
    // if (connect_ret == -1)
    // {
    //     perror("connect error");
    // }

    // 远程广播地址
    sockaddr_in remote_addr;
    bzero(&remote_addr, sizeof(sockaddr_in));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(7777);
    remote_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        const char* msg_to_others = "Client say hello.";
        int send_ret = sendto(sock_fd, msg_to_others, strlen(msg_to_others), 0, (sockaddr*)(&remote_addr), sizeof(sockaddr));
        if (send_ret == -1)
        {
            perror("sendto error");
        }

        char msg_from_others[1024];
        bzero(msg_from_others, 1024);
        sockaddr_in recv_addr;
        bzero(&recv_addr, sizeof(sockaddr_in));
        socklen_t socklen = sizeof(sockaddr);
        int recv_ret = recvfrom(sock_fd, msg_from_others, 1024, 0, (sockaddr*)(&recv_addr), &socklen);
        if (recv_ret == -1)
        {
            perror("recvfrom error");
        }
        else
        {
            std::cout << "recv from "
                      << inet_ntoa(recv_addr.sin_addr)
                      << ":" << ntohs(recv_addr.sin_port)
                      << " - " << msg_from_others << std::endl;
        }
    }
}

void StartClient_TCP()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in dest_addr;
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(7777);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int conn_ret = connect(sockfd, (sockaddr*)(&dest_addr), sizeof(sockaddr));
    if (conn_ret != 0)
    {
        int error_no = errno;
        char* err_msg = strerror(error_no);
        perror("socket connect error");
    }

    char msg_from_recv[1024];
    bzero(msg_from_recv, 1024);
    int recv_ret = recv(sockfd, msg_from_recv, 1024, 0);
    if (recv_ret == -1)
    {
        int error_no = errno;
        char* err_msg = strerror(error_no);
        perror("socket send error");
    }
    else
    {
        std::cout << "recv msg(" << recv_ret << "): "
                  << msg_from_recv << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(6));

    const char* msg_reply_to_server = "Ok, Received.";
    int send_ret = send(sockfd, msg_reply_to_server, strlen(msg_reply_to_server), 0);
    if (send_ret == -1)
    {
        int error_no = errno;
        char* err_msg = strerror(error_no);
        perror("socket send error");
    }
    else
    {
        std::cout << "send reply msg(" << send_ret << "): "
                  << msg_reply_to_server << std::endl;
    }
}
