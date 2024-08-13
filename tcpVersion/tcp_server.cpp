#include <opencv2/opencv.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#define PORT 8182

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 1. Soket oluşturma
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Soket opsiyonları
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 3. Adres yapılandırması
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 4. Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 5. Dinleme
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // std::cout << "acceptte bekliyor\n";
    // 6. Kabul etme
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    // std::cout << "accepti geçti\n";
    // 7. Video yakalama
    cv::VideoCapture cap(0); // 0, varsayılan kamera içindir.
    if (!cap.isOpened())
    {
        std::cerr << "Error opening video stream or file" << std::endl;
        return -1;
    }

    while (true)
    {
        cv::Mat frame;
        cap >> frame; 

        if (frame.empty())
        {
            std::cerr << "Empty frame captured" << std::endl;
            break;
        }

        // 8. Kareyi JPEG formatında sıkıştırma
        std::vector<uchar> buf;
        cv::imencode(".jpg", frame, buf);

        int frame_size = buf.size();
        // std::cout<<frame_size<<std::endl;g
        if (send(new_socket, &frame_size, sizeof(int), 0) == -1)
        {
            perror("send frame size failed");
            break;
        }

        // 9. Sıkıştırılmış kareyi gönderme
        if (send(new_socket, buf.data(), frame_size, 0) == -1)
        {
            perror("send frame data failed");
            break;
        }
        // imshow("Video Player", frame); // Showing the video//
        char c = (char)cv::waitKey(100);    // Allowing 100 milliseconds frame processing time and initiating break condition//
        if (c == 27)
        { // If 'Esc' is entered break the loop//
            break;
        }
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
