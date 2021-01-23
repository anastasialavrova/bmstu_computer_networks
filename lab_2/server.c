#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#define BUFLEN 512
#define PORT 9930

void diep(char *s)
{
    perror(s);
    exit(1);
}

int convert(int number, int base)
{
    int res_number = 0, i = 1;

    while (number != 0)
    {
        res_number += (number % base) * i;
        number /= base;
        i *= 10;
    }

    return res_number;
}

int main(void)
{
    struct sockaddr_in socket_info, other_info;
    int my_socket, i, string_len = sizeof(other_info);
    char buf[BUFLEN];

    printf("Server started\n");

    // Создание дескриптора файла сокета
    // AF_INET - домен, SOCK_DGRAM - тип сокета, IPPROTO_UDP - протокол
    if ((my_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *)&socket_info, 0, sizeof(socket_info));
    
    // Заполнение информации о сервере
    socket_info.sin_family = AF_INET;
    socket_info.sin_port = htons(PORT);
    socket_info.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // Привязываем сокет с адресом сервера
    if (bind(my_socket, &socket_info, sizeof(socket_info)) == -1)
        diep("bind");

    while (1)
    {
        // получаем сообщение из сокета
        if (recvfrom(my_socket, buf, BUFLEN, 0, &other_info, &string_len) == -1)
            diep("recvfrom()");
        {
            int num_dec, num_oct, num_hex, num_bin, num_six;
            num_dec = atoi(buf);
            num_oct = num_hex = num_bin = num_six = num_dec;

            num_bin = convert(num_dec, 2);
            num_oct = convert(num_dec, 8);
            num_oct = convert(num_dec, 16);

            printf("2 СС =  %d\n", num_bin);
            printf("8 СС =  %d\n", num_oct);
            printf("10 СС =  %d\n", num_dec);
            printf("16 СС = %x\n", num_hex);
            
        }
    }

    close(my_socket);
    return 0;
}
