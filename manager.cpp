#include "configs.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include <iostream>
#include <deque>
#include <chrono>

#define HEADER_SIZE (4 + 2 + 1 + 2)
#define MSG_SIZE 91

uint8_t content[91] = 
{
  // Start word
  0xAA,0xAA,0xAA,0xAA,
  // Data length, little endian, dec 80, hex 0x0050
  0x50,0x00,
  // protocol id
  0x55,
  // CRC16
  0xBE, 0x88,
  // Payload, 0~79
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
  0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,
  0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,
  0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
  0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,
  0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,
  0x3C,0x3D,0x3E,0x3F,0x40,0x41,0x42,0x43,0x44,0x45,
  0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
  // CRC16
  0x94,0x2A
};


std::deque<uint8_t> rx_deque;
uint32_t rx_total = 0;
uint32_t msg_count = 0;
void decode(uint8_t *buf, int len)
{
    rx_total += len;

    rx_deque.insert(rx_deque.end(), buf, buf + len);

    int size = rx_deque.size();
    std::cout << "deque size = " << size << std::endl;

    static auto start = std::chrono::high_resolution_clock::now();

    if(size >=  MSG_SIZE * 2)
    {
        
        std::cout << "manager: rx_total = " << rx_total << std::endl;
        // pseudo crc check
        for(int i = 0; i < MSG_SIZE; i++)
        {
            if(rx_deque[i] == content[0])
            {
                // found first byte
                rx_deque.erase(rx_deque.begin(), rx_deque.begin() + i);

                for(int j = 0; j < MSG_SIZE; j++)
                {
                    if(rx_deque[j] != content[j])
                    {
                        // corrupted message

                        rx_deque.erase(rx_deque.begin(), rx_deque.begin() + MSG_SIZE);

                        std::cout << "manager: corrupted message" << std::endl;
                        return;
                    }
                }
                // found a message

                auto end = std::chrono::high_resolution_clock::now();

                // Calculate the elapsed time
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                msg_count++;
                // std::cout << "manager: " << duration.count() << " milliseconds, " << msg_count<< " msg, " << "freq = "<< std::endl;
                std::cout << "manager: " << duration.count() << " milliseconds, " << msg_count<< " msg, " << "freq = " << (duration.count() ? ((float)msg_count / duration.count()) * 1000 : -1 )<< std::endl;

                // erase message found
                rx_deque.erase(rx_deque.begin(), rx_deque.begin() + MSG_SIZE);
                return;
            }
        }

        // not found
        std::cout << "manager: not found" << std::endl;
        rx_deque.erase(rx_deque.begin(), rx_deque.begin() + MSG_SIZE);
    }
}


int main()
{
    // create named pipe R
    if(access(CH343_2_ROS_FIFO_NAME, F_OK) != 0)
    {
        //如果文件存在
        int err = mkfifo(CH343_2_ROS_FIFO_NAME, 0777);
        if(err != 0)
        {
            perror("manager: Create r fifo failed");
            return -1;
        }
    }

    // create named pipe W
    if(access(ROS_2_CH343_FIFO_NAME, F_OK) != 0)
    {
        //如果文件存在
        int err = mkfifo(ROS_2_CH343_FIFO_NAME, 0777);
        if(err != 0)
        {
            perror("manager: Create w fifo failed");
            return -1;
        }
    }

    printf("manager: create fifo succeed!\n");

    int fifo_r_fd = open(CH343_2_ROS_FIFO_NAME, O_RDONLY | O_NONBLOCK);
    int fifo_w_fd = open(ROS_2_CH343_FIFO_NAME, O_WRONLY | O_NONBLOCK);

    if(fifo_r_fd || fifo_w_fd)
    {
        printf("manager: open fifo succeed!\n");
    }
    else
    {
        printf("manager: open fifo failed!\n");
        return 1;
    }

    // create r process using popen
    FILE *r_process;
    char r_buffer[512];

    r_process = popen(CH343_R_PROCESS_SHELL_COMMAND, "r");
    if (r_process == NULL)
    {
        printf("manager: Failed to open r process.\n"); 

        return 1;
    }
    else
    {
        printf("manager: Open r process succeed!\n");
    }
    // char write_buf[4096];
    // uint32_t nwrite;
    // create w process using popen

    // FILE *w_process;
    // char w_buffer[128];

    // w_process = popen("./write", "r");
    // if (w_process == NULL)
    // {
    //     printf("Failed to open w process.\n");
    //     return 1;
    // }
    // else
    // {
    //     printf("Open w process succeed!\n");
    // }


    uint8_t read_buf[4096];
    int nread;
    uint32_t total = 0;


    // char write_buf[4096];
    // uint32_t nwrite;


    char buffer[512];
    // Read the output of the command

    printf("manager: read from r process:\n");


    // if(fgets(r_buffer, 128, r_process) != NULL) 
    // {
    //     printf("%s", r_buffer);
    // }

    printf("manager: read from r fifo:\n");

    while (1)
    {
        // read from r fifo

        nread = read(fifo_r_fd, read_buf, 4096);
        if(nread > 0)
        {
            decode(read_buf, nread);
        }

    }
    

}