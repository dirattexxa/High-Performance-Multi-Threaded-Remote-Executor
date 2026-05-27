#include <iostream>
#include <cstring>
#include <utility>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <sys/wait.h>  
#include <sys/types.h> 

class CommandPacket {
    public :
    char* raw_data;
    size_t size;

    CommandPacket(size_t s) : size(s) {
        raw_data = new char[size];
        std::memset(raw_data, 0 , size);   
    }

    ~CommandPacket() {
        if(raw_data != nullptr) {
            delete[] raw_data;
        }
    }

    CommandPacket(CommandPacket&& other) noexcept {
       this->raw_data = other.raw_data;
       this->size = other.size;

       other.raw_data = nullptr;
       other.size = 0;
    }

    CommandPacket(const CommandPacket&) = delete;
    CommandPacket& operator = (const CommandPacket&) = delete;
};

void handle_client(int client_fd, CommandPacket packet) {
    std::cout << "Thread started for socket" << client_fd << "\n" ;

    ssize_t bytes_read = read(client_fd, packet.raw_data, packet.size -1 );

    if(bytes_read > 0) {
       for (size_t i = 0; i < packet.size; ++i){
        if(packet.raw_data[i] == '\n' || packet.raw_data[i] == '\r') {
            packet.raw_data[i] = '\0';
            break;
        }
       }
       std::cout << "Command executing" << packet.raw_data << "\n";

       pid_t pid = fork();

       if(pid < 0) {
        std::cerr << "Fork error\n" ;
        return;
       }

       else if ( pid == 0) {
        dup2(client_fd, STDOUT_FILENO);
        dup2(client_fd, STDERR_FILENO);

        char* args[] = {(char*)"/bin/sh", (char*)"-c", packet.raw_data, nullptr};
        execvp(args[0], args);

        std::cerr << "Unkown command! \n";
        exit(1);
       }else {
        int status;
        waitpid(pid, &status, 0);
       }
    }

    std::cout << "Close connection with client \n";
    close(client_fd); 
}

int main() {
    int server_fd =  socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        std::cerr << "Socket creation error \n";
        return 1;
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "May be bind error" << std::endl;
        close(server_fd);
        return 1;
    }

    if(listen(server_fd, 5) < 0) {
        std::cerr << " Listen error\n";
        close(server_fd);
        return 1;
    }
    std::cout << "Manager is running on port 8080 \n";

    while(true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if(client_fd < 0) {
            std::cerr << "Accept error\n" ;
            continue;
        }
        std::cout << "New client connected \n";

        CommandPacket packet(1024);

        std::thread t1(handle_client, client_fd, std::move(packet));
        t1.detach();   
        
    }

    // int client_fd = accept(server_fd, nullptr, nullptr);
    // if(client_fd < 0) {
    //     std::cerr << "Accept error \n";
    //     close(server_fd);
    //     return 1; 
    // }
    // std::cout << "Client connected \n";

    // CommandPacket packet(1024);

    // ssize_t bytes_read = read(client_fd, packet.raw_data, packet.size -1);
    // if(bytes_read > 0) {
    //     std::cout << "Received a command: " << packet.raw_data << "\n";
    // }

    // CommandPacket moved_packet = std::move(packet);

    // std::cout << "Moved packet data: " << moved_packet.raw_data << std::endl;

    // close(server_fd);
    // close(client_fd);
    // return 0;

}