#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "configs.h"

int main()
{
    unlink(ROS_2_CH343_FIFO_NAME);
    unlink(CH343_2_ROS_FIFO_NAME);

    return 0;
}