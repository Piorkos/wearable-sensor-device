#include <cstring>
#include "pa1010d.h"


GPS::GPS()
{
    printf("GPS contructor \n");

    // Convert character array to bytes for writing
    size_t com_length{strlen(init_command)};
    uint8_t int_command[com_length];

    for (size_t i = 0; i < com_length; ++i) {
        int_command[i] = init_command[i];
        if(i < (com_length - 1)){
            i2c_write_blocking(i2c_, ADDR, &int_command[i], 1, true);
        }
        else{
            i2c_write_blocking(i2c_, ADDR, &int_command[i], 1, false);            
        }
    }
}

GPS::~GPS()
{
    printf("GPS destructor \n");

    i2c_ = nullptr;
    delete i2c_;
}

// TODO improve
bool GPS::HasFix()
{
    // printf("GPS::HasFix \n");

    SensorsData sensors_data;

    if(gps_fix_count < 5)  // if GPS does NOT have a fix
    {
        ++read_gps_flag;
        if(read_gps_flag == 10)
        {
            printf("HasFix: gps_fix_count = %i \n", gps_fix_count);
            read_gps_flag = 0;
            memset(numcommand, 0, MAX_READ);    //probably not needed because it is executed in ReadTaw()
            ReadRaw(sensors_data);

            if(sensors_data.latitude != "zero" && sensors_data.latitude != "err")
            {
                ++gps_fix_count; 
            }
            else if(gps_fix_count > 0)
            {
                --gps_fix_count;
            }
        }
        return false;
    }

    gps_fix_count = 0;

    return true;
}

/**
 * @brief Once per 10 calls of this function, it reads data from GPS
 * 
 * @param sensors_data structure to store data read from GPS
 * @param error 1 if no error, -1 if timeout, -2 if no connection
 * @return true if this was the 1 of 10 calls when it reads data
 * @return false otherwise
 */
bool GPS::ReadData1Per10(SensorsData& sensors_data, int& error)
{
    printf("GPS::ReadData1Per10 \n");

    ++read_gps_flag;
    if(read_gps_flag == 10)
    {
        printf("GPS::ReadData1Per10 - read_gps_flag = 10 \n");
        read_gps_flag = 0;

        memset(numcommand, 0, MAX_READ);
        printf("GPS::ReadData1Per10 - 1 \n");
        error = ReadRaw(sensors_data);
        error_cache = error;
        printf("GPS::ReadData1Per10 - 2 \n");

        return true;
    }
    else
    {
        error = error_cache;
    }
    
    return false;
}

/**
 * @brief Checks if connection is ok.
 * 
 * @return -2 if timeout, -1 if connection broken, 1 if connection ok
 */
int GPS::TestConnection()
{
    printf("GPS::TestConnection \n");

    uint8_t buffer;
    int bytes_read = i2c_read_blocking(i2c_, ADDR, &buffer, 1, false);
    if(bytes_read == PICO_ERROR_GENERIC)
    {
        printf("GPS READING ERROR - NO CONNECTION \n");
        return -2;
    }
    else if(bytes_read == PICO_ERROR_TIMEOUT)
    {
        printf("GPS READING ERROR - TIMEOUT \n");
        return -1;
    }
    return 1;
}

void GPS::ParseGNMRC(char output[], char protocol[], std::string& latitude, std::string& longitude, std::string& utc_time)
{
    printf("GPS::parse_GNMRC numcommand: %s \n", output);

    // Finds location of protocol message in output
    char *com_index = strstr(output, protocol);
    int p = com_index - output;

    // Splits components of output sentence into array
    int no_of_fields = 14;
    int max_len = 15;

    int n = 0;
    int m = 0;

    char gps_data[no_of_fields][max_len];
    memset(gps_data, 0, sizeof(gps_data));

    bool complete = false;
    while (output[p] != '$' && n < max_len && complete == false) {
        if (output[p] == ',' || output[p] == '*') {
            n += 1;
            m = 0;
        } else {
            gps_data[n][m] = output[p];
            // Checks if sentence is complete
            if (m < no_of_fields) {
                m++;
            } else {
                complete = true;
            }
        }
        p++;
    }

    if (strcmp(protocol, "GNRMC") == 0) {
        
        printf("Protcol:%s\n", gps_data[0]);
        printf("UTC Time: %s\n", gps_data[1]);
        printf("Status: %s\n", gps_data[2]);
        printf("Latitude: %s\n", gps_data[3]);
        // printf("N/S indicator: %s\n", gps_data[4]);
        printf("Longitude: %s\n", gps_data[5]);
        printf("E/W indicator: %s\n", gps_data[6]);
        // printf("Speed over ground: %s\n", gps_data[7]);
        // printf("Course over ground: %s\n", gps_data[8]);
        printf("Date: %c%c/%c%c/%c%c\n", gps_data[9][0], gps_data[9][1], gps_data[9][2], gps_data[9][3], gps_data[9][4],
            gps_data[9][5]);
        // printf("Magnetic Variation: %s\n", gps_data[10]);
        // printf("E/W degree indicator: %s\n", gps_data[11]);
        // printf("Mode: %s\n", gps_data[12]);
        // printf("Checksum: %c%c\n", gps_data[13][0], gps_data[13][1]);

        if(gps_data[2][0] == 'A')   //A-valid, V-invalid
        {
            utc_time = gps_data[1];
            latitude = gps_data[3];
            latitude += gps_data[4];
            longitude = gps_data[5];
            longitude += gps_data[6];
        }
        else
        {
            latitude = "zero";
            longitude = "zero";
            utc_time = "zero";
        }
    }
}

int GPS::ReadRaw(SensorsData& sensors_data) 
{
    memset(numcommand, 0, MAX_READ);
    uint8_t buffer[MAX_READ];

    int i = 0;
    bool complete = false;

    int bytes_read = i2c_read_blocking(i2c_, ADDR, buffer, MAX_READ, false);
    if(bytes_read == PICO_ERROR_GENERIC)
    {
        printf("GPS READING ERROR - NO CONNECTION \n");
        sensors_data.latitude = "err";
        sensors_data.longitude = "err";
        sensors_data.utc_time = "err";
        return -2;
    }
    else if(bytes_read == PICO_ERROR_TIMEOUT)
    {
        printf("GPS READING ERROR - TIMEOUT \n");
        sensors_data.latitude = "err";
        sensors_data.longitude = "err";
        sensors_data.utc_time = "err";
        return -1;
    }

    // Convert bytes to characters
    while (i < bytes_read && complete == false) 
    {
        numcommand[i] = buffer[i];
        // Stop converting at end of message 
        if (buffer[i] == 10 && buffer[i + 1] == 10) 
        {
            complete = true;
        }
        i++;
    }
    ParseGNMRC(numcommand, "GNRMC", sensors_data.latitude, sensors_data.longitude, sensors_data.utc_time);

    return 1;
}