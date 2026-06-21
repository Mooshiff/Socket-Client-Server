#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main()
{
    char net_info[1024];
    int ret = gethostname(net_info, 1024);
    if (ret == -1)
    {
        perror("gethostname error");
    }
    std::cout << net_info << "  hostid: " << gethostid() << std::endl;

    std::cout << "SocketServer Main End." << std::endl;
    return 0;
}

void PrintAddrInfo()
{
    addrinfo* addrinfo_result = nullptr;
    int ret = getaddrinfo("www.baidu.com", "http", nullptr, &addrinfo_result);
    if (ret != 0)
    {
        const char* err_str = gai_strerror(ret);
        std::cout << "getaddrinfo error: " << err_str << std::endl;
    }
    else
    {
        for (addrinfo* current = addrinfo_result; current; current = current->ai_next)
        {
            std::cout << "addrinfo->ai_flags: " << current->ai_flags << std::endl;
            std::cout << "addrinfo->ai_family: " << (current->ai_family == AF_INET ? "AF_INET" : "AF_INET6") << std::endl;
            std::cout << "addrinfo->ai_socktype: " << (current->ai_socktype == SOCK_STREAM ? "SOCK_STREAM" : "SOCK_DGRAM") << std::endl;
            std::cout << "addrinfo->ai_protocol: " << current->ai_protocol << std::endl;
            std::cout << "addrinfo->ai_addrlen: " << current->ai_addrlen << std::endl;
            sockaddr_in* ai_addr = (sockaddr_in*)(current->ai_addr);
            std::cout << "addrinfo->ai_addr: " << inet_ntoa(ai_addr->sin_addr) << ":" << ntohs(ai_addr->sin_port) << std::endl;
            // std::cout << "addrinfo->ai_canonname: " << current->ai_canonname << std::endl; ///< 若 ai_canonname 指向 0x0 地址, 将导致流变为 badbit
            std::cout << "addrinfo->ai_canonname: " << (current->ai_canonname ? current->ai_canonname : "") << std::endl;
            std::cout << "\n\n";
        }
    }
    freeaddrinfo(addrinfo_result);
}

void PrintHostInfo()
{
    hostent* host_entry = gethostbyname("www.baidu.com");
    if (!host_entry)
    {
        herror("gethostbyname error");
    }
    else
    {
        std::cout << "host_entry->h_name: " << host_entry->h_name << std::endl;
        std::cout << "host_entry->h_aliases: \n";
        for (int alia_index = 0; host_entry->h_aliases[alia_index]; ++alia_index)
        {
            std::cout << "\t" << host_entry->h_aliases[alia_index] << std::endl;
        }
        std::cout << "host_entry->h_addrtype: " << (host_entry->h_addrtype == AF_INET ? "AF_INET" : "AF_INET6") << std::endl;
        std::cout << "host_entry->h_length: " << host_entry->h_length << std::endl;
        std::cout << "host_entry->h_addr_list: \n";
        for (int addr_index = 0; host_entry->h_addr_list[addr_index]; ++addr_index)
        {
            std::cout << "\t" << inet_ntoa(*(in_addr*)(host_entry->h_addr_list[addr_index])) << std::endl;
        }
    }
}

void StartServer_UDP()
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in local_addr;
    bzero(&local_addr, sizeof(sockaddr_in));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(7777);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    int bind_ret = bind(sock_fd, (sockaddr*)(&local_addr), sizeof(sockaddr));
    if (bind_ret == -1)
    {
        perror("bind error");
    }

    while (true)
    {
        char msg_from_others[1024];
        bzero(msg_from_others, 1024);
        sockaddr_in remote_addr;
        bzero(&remote_addr, sizeof(sockaddr_in));
        socklen_t socklen = sizeof(sockaddr); ///< 必须初始化为 remote_addr 的大小, 否则, 太小将截断, 为 0 则直接不会赋值
        int recvfrom_ret = recvfrom(sock_fd, msg_from_others, 1024, 0, (sockaddr*)(&remote_addr), &socklen);
        if (recvfrom_ret == -1)
        {
            perror("recvfrom error");
        }
        else
        {
            std::cout << "recvfrom "
                      << inet_ntoa(remote_addr.sin_addr)
                      << ":" << ntohs(remote_addr.sin_port)
                      << " - " << msg_from_others << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));

        const char* msg_to_others = "Server say Hi.";
        int sendto_ret = sendto(sock_fd, msg_to_others, strlen(msg_to_others), 0, (sockaddr*)(&remote_addr), sizeof(sockaddr));
        if (sendto_ret == -1)
        {
            perror("sendto error");
        }
    }
}

void StartServer_TCP()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(7777);
    // addr_in.sin_port = 0; ///< 随机选择一个没有使用的端口
    addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr_in.sin_addr.s_addr = INADDR_ANY; ///< 使用自己的IP地址??
    // char* ip = inet_ntoa(addr_in.sin_addr);
    bzero(&addr_in.sin_zero, sizeof(addr_in.sin_zero));

    int bind_ret = bind(sock_fd, (sockaddr*)(&addr_in), sizeof(sockaddr));
    if (bind_ret != 0)
    {
        int error_no = errno;
        char* err_msg = strerror(error_no);
        perror("socket bind error");
    }

    int listen_ret = listen(sock_fd, 5);
    if (listen_ret != 0)
    {
        int error_no = errno;
        char* err_msg = strerror(error_no);
        perror("socket listen error");
    }

    while (true)
    {
        sockaddr_in remote_addr;
        socklen_t sin_size = sizeof(sockaddr_in);
        int new_sock_fd = accept(sock_fd, (sockaddr*)(&remote_addr), &sin_size);
        if (new_sock_fd == -1)
        {
            int error_no = errno;
            char* err_msg = strerror(error_no);
            perror("socket accept error");
        }
        else
        {
            uint16_t port = ntohs(remote_addr.sin_port);
            char* remote_addr_ascii = inet_ntoa(remote_addr.sin_addr);
            std::cout << remote_addr_ascii << ":" << port << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));

        const char* msg_to_send = "Reply from SocketServer.";
        size_t msg_len = strlen(msg_to_send);
        int send_ret = send(new_sock_fd, msg_to_send, msg_len, 0);
        if (send_ret == -1)
        {
            int error_no = errno;
            char* err_msg = strerror(error_no);
            perror("socket send error");
        }
        else
        {
            std::cout << "send msg(" << send_ret << "): "
                      << msg_to_send << std::endl;
        }

        char msg_from_reply[1024];
        bzero(msg_from_reply, 1024);
        int recv_ret = recv(new_sock_fd, msg_from_reply, 1024, 0);
        if (recv_ret == -1)
        {
            int error_no = errno;
            char* err_msg = strerror(error_no);
            perror("socket send error");
        }
        else
        {
            std::cout << "recv reply msg(" << recv_ret << "): "
                      << msg_from_reply << std::endl;
        }
    }
}
