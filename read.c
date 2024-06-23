#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <getopt.h>
#include <linux/serial.h>
#define termios asmtermios
#include <asm/termios.h>
#undef termios
#include <termios.h>

extern int ioctl(int d, int request, ...);

#include "configs.h"
/**
 * libtty_setcustombaudrate - set baud rate of tty device
 * @fd: device handle
 * @speed: baud rate to set
 *
 * The function return 0 if success, or -1 if fail.
 */
static int libtty_setcustombaudrate(int fd, int baudrate)
{
	struct termios2 tio;

	if (ioctl(fd, TCGETS2, &tio)) {
		perror("TCGETS2");
		return -1;
	}

	tio.c_cflag &= ~CBAUD;
	tio.c_cflag |= BOTHER;
	tio.c_ispeed = baudrate;
	tio.c_ospeed = baudrate;

	if (ioctl(fd, TCSETS2, &tio)) {
		perror("TCSETS2");
		return -1;
	}

	if (ioctl(fd, TCGETS2, &tio)) {
		perror("TCGETS2");
		return -1;
	}

	return 0;
}

/**
 * libtty_setopt - config tty device
 * @fd: device handle
 * @speed: baud rate to set
 * @databits: data bits to set
 * @stopbits: stop bits to set
 * @parity: parity to set
 * @hardflow: hardflow to set
 *
 * The function return 0 if success, or -1 if fail.
 */
static int libtty_setopt(int fd, int speed, int databits, int stopbits, char parity, char hardflow)
{
	struct termios newtio;
	struct termios oldtio;
	int i;

	bzero(&newtio, sizeof(newtio));
	bzero(&oldtio, sizeof(oldtio));

	if (tcgetattr(fd, &oldtio) != 0) {
		perror("tcgetattr");
		return -1;
	}
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	/* set data bits */
	switch (databits) {
	case 5:
		newtio.c_cflag |= CS5;
		break;
	case 6:
		newtio.c_cflag |= CS6;
		break;
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "unsupported data size\n");
		return -1;
	}

	/* set parity */
	switch (parity) {
	case 'n':
	case 'N':
		newtio.c_cflag &= ~PARENB; /* Clear parity enable */
		newtio.c_iflag &= ~INPCK;  /* Disable input parity check */
		break;
	case 'o':
	case 'O':
		newtio.c_cflag |= (PARODD | PARENB); /* Odd parity instead of even */
		newtio.c_iflag |= INPCK;	     /* Enable input parity check */
		break;
	case 'e':
	case 'E':
		newtio.c_cflag |= PARENB;  /* Enable parity */
		newtio.c_cflag &= ~PARODD; /* Even parity instead of odd */
		newtio.c_iflag |= INPCK;   /* Enable input parity check */
		break;
	case 'm':
	case 'M':
		newtio.c_cflag |= PARENB; /* Enable parity */
		newtio.c_cflag |= CMSPAR; /* Stick parity instead */
		newtio.c_cflag |= PARODD; /* Even parity instead of odd */
		newtio.c_iflag |= INPCK;  /* Enable input parity check */
		break;
	case 's':
	case 'S':
		newtio.c_cflag |= PARENB;  /* Enable parity */
		newtio.c_cflag |= CMSPAR;  /* Stick parity instead */
		newtio.c_cflag &= ~PARODD; /* Even parity instead of odd */
		newtio.c_iflag |= INPCK;   /* Enable input parity check */
		break;
	default:
		fprintf(stderr, "unsupported parity\n");
		return -1;
	}

	/* set stop bits */
	switch (stopbits) {
	case 1:
		newtio.c_cflag &= ~CSTOPB;
		break;
	case 2:
		newtio.c_cflag |= CSTOPB;
		break;
	default:
		perror("unsupported stop bits\n");
		return -1;
	}

	if (hardflow)
		newtio.c_cflag |= CRTSCTS;
	else
		newtio.c_cflag &= ~CRTSCTS;

	newtio.c_cc[VTIME] = 10; /* Time-out value (tenths of a second) [!ICANON]. */
	newtio.c_cc[VMIN] = 0;	 /* Minimum number of bytes read at once [!ICANON]. */

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newtio) != 0) {
		perror("tcsetattr");
		return -1;
	}

	/* set tty speed */
	if (libtty_setcustombaudrate(fd, speed) != 0) {
		perror("setbaudrate");
		return -1;
	}

	return 0;
}

/**
 * libtty_open - open tty device
 * @devname: the device name to open
 *
 * In this demo device is opened blocked, you could modify it at will.
 */
static int libtty_open(const char *devname)
{
	int fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY);
	int flags = 0;

	if (fd < 0) {
		perror("open device failed");
		return -1;
	}

	flags = fcntl(fd, F_GETFL, 0);
	// flags &= ~O_NONBLOCK;
    flags &= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		printf("fcntl failed.\n");
		return -1;
	}

	if (isatty(fd) == 0) {
		printf("not tty device.\n");
		return -1;
	} else
		printf("tty device test ok.\n");

	return fd;
}


int main()
{
	sleep(5);
    printf("read: read process start!\n");
    int ch343_fd;

	do
	{
		ch343_fd = libtty_open(CH343_DEV_NAME);

	} while (ch343_fd == -1);
	


    // open device succeed
    printf("read: open device succeed!\n");
	printf("read: ch343_fd = %d\n", ch343_fd);

    // config device
    printf("read: config device!\n");

    if(libtty_setopt(ch343_fd, CH343_BAUDRATE, CH343_DATABITS, CH343_STOPBITS, CH343_PARITY, CH343_HARDFLOW) == -1)
    {
        // config device failed
        printf("read: config device failed!\n");
        // exit
        return 0;
    }
    printf("read: config device succeed!\n");

    // find named pipe
    while(access(CH343_2_ROS_FIFO_NAME, F_OK) != 0)
    {
        // named pipe not found
        printf("read: named pipe not found!\n");
        // retry
        sleep(1);
    }

    // found named pipe
    printf("read: found named pipe!\n");

    // open named pipe
    int fifo = open(CH343_2_ROS_FIFO_NAME, O_WRONLY | O_NONBLOCK);

    int nread;
    char read_buf[4096];
    uint32_t total = 0;

    // start read
    printf("read: start read!\n");

	// sleep(1);

    while(1)
    {
        nread = read(ch343_fd, read_buf, 4096);

        // write to named pipe
        if(nread > 0)
        {   
			total += nread;     
            // printf("read: read %d bytes\n", nread);

            if(write(fifo, read_buf, nread) == -1)
            {
				// const char* filePath = "output.txt";  // Specify the path and name of the output file

				// // Open the file in write mode
				// FILE* file = fopen(filePath, "w");
				// if (file == NULL) {
				// 	perror("Failed to open file");
				// 	return 1;
				// }

				// // Write content to the file
				// fprintf(file, "Hello, World!\n");
				// fputs("This is a text file.\n", file);
				// fprintf(file, "%d + %d = %d\n", 2, 3, 2 + 3);

				// // Close the file
				// fclose(file);

				// printf("File '%s' created successfully.\n", filePath);
				return 1;
            }
        }
    }

    return 0;
}