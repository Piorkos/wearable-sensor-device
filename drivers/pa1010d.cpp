#include "pa1010d.h"

GPS::GPS()
{
     printf("GPS contructor \n");

    // Convert character array to bytes for writing
    int com_length{strlen(init_command)};
    uint8_t int_command[com_length];

    for (int i = 0; i < com_length; ++i) {
        int_command[i] = init_command[i];
        if(i < (com_length - 1)){
            i2c_write_blocking(i2c, ADDR, &int_command[i], 1, true);
        }
        else{
            i2c_write_blocking(i2c, ADDR, &int_command[i], 1, false);            
        }
    }
}