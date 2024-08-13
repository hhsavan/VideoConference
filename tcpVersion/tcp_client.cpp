#include <opencv2/opencv.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <chrono>

#define PORT 8182
#define BUFFER_SIZE 65536

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // 1. Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 2. Convert IP addresses from text to binary form
    if (inet_pton(AF_INET, "10.0.2.2", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        return -1;
    }

    // 3. Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return -1;
    }

    cv::namedWindow("Received Video", cv::WINDOW_NORMAL);
    cv::resizeWindow("Received Video",800,500);

    while (true) {
        auto startTime = std::chrono::high_resolution_clock::now();
        int frame_size = 0;

        // 4. Receive the frame size (ensure the full 4 bytes are received)
        int bytes_received = recv(sock, &frame_size, sizeof(frame_size), 0);
        if (bytes_received != sizeof(frame_size)) {
            std::cerr << "Failed to receive frame size" << std::endl;
            break;
        }
        //std::cout << "frame_size: " << frame_size << std::endl;

        //std::cout << "bytes_received: " << bytes_received << std::endl;

        // Convert frame_size to host byte order
        //frame_size = ntohl(frame_size);


        std::vector<uchar> buf(frame_size);

        // 5. Receive the frame data
        int total_bytes_received = 0;
        while (total_bytes_received <= frame_size) {

            int bytes = recv(sock, buf.data() + total_bytes_received, frame_size - total_bytes_received, 0);
    
           // std::cout << "total_bytes_received: " << total_bytes_received << std::endl;

            if (bytes <= 0) {
                //std::cerr << "Failed to receive frame data" << std::endl;
                break;
            }
            total_bytes_received += bytes;
        }

        if (total_bytes_received != frame_size) {
            std::cerr << "Incomplete frame data received" << std::endl;
            break;
        }

        // 6. Decode the received frame
        cv::Mat frame = cv::imdecode(buf, cv::IMREAD_COLOR);
        if (frame.empty()) {
            std::cerr << "Decoded frame is empty" << std::endl;
            break;
        }

        // 7. Display the received frame
        cv::imshow("Received Video", frame);
        auto finishTime = std::chrono::high_resolution_clock::now();
        auto frameTime = (std::chrono::duration_cast<std::chrono::milliseconds>(finishTime - startTime)).count();
        double fps = 1000.0 / frameTime;
        std::cout << "FPS: " << fps<< std::endl;

        if (cv::waitKey(1) >= 0) break;
    }

    close(sock);
    return 0;
}