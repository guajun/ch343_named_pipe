#include <stdio.h>
 #include <unistd.h>

int main() 
{
    FILE *pipe;
    char buffer[128];

    // Execute the command and open a pipe to read the output
    pipe = popen("./read", "r");
    if (pipe == NULL)
    {
        printf("Failed to open pipe.\n");
        return 1;
    }
    else
    {
        printf("Open pipe succeed!\n");
    }

    sleep(4);

    // Read the output of the command
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) 
    {
        printf("%s", buffer);
    }

    // Close the pipe
    pclose(pipe);

    return 0;
}