#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

int uart_procedure(int argc, char* argv[])
{

    struct termios serial;
    char* str = "Hello";
    char buffer[10];

    if (argc == 1)
    {
       #ifdef debugmode
          //printf("UART ERROR Usage: %s [device]", argv[0]);
       #endif

       return -1;
    }

    int fd = open(argv[1], O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1)
    {
        perror(argv[1]);
        return -1;
    }

    if (tcgetattr(fd, &serial) < 0)
    {
        perror("Getting configuration");
        return -1;
    }

    // Set up Serial Configuration
    serial.c_iflag = 0;
    serial.c_oflag = 0;
    serial.c_lflag = 0;
    serial.c_cflag = 0;

    serial.c_cc[VMIN] = 0;
    serial.c_cc[VTIME] = 0;

    serial.c_cflag = B115200 | CS8 | CREAD;

    tcsetattr(fd, TCSANOW, &serial); // Apply configuration

    // Attempt to send and receive
#ifdef debugmode
    //printf("UART Sending: %s", str);
#endif

    int wcount = write(fd, str, strlen(str));
    if (wcount < 0)
    {
        perror("Write");
        return -1;
    }
    else
    {
#ifdef debugmode
        //printf("UART Sent %d characters", wcount);
#endif
    }

    int rcount = read(fd, buffer, sizeof(buffer));
    if (rcount < 0)
    {
        perror("Read");
        return -1;
    }
    else
    {
#ifdef debugmode
        //printf("UART Received %d characters", rcount);
#endif
    }

    buffer[rcount] = '\0';

#ifdef debugmode
    //printf("UART Received: %s", buffer);
#endif
    close(fd);

    return 0;

}
