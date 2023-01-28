#ifndef PA1010D_H
#define PA1010D_H

#include "hardware/i2c.h"
#include <string>


const int addr = 0x10;
const int max_read = 250;




namespace pa1010d
{
    void pa1010d_write_command(i2c_inst_t *i2c, const char command[], int com_length);
    void parse_GNMRC(char output[], char protocol[], std::string& latitude, std::string& longitude, std::string& utc_time);
    void read_raw(i2c_inst_t *i2c, char numcommand[]);
    bool HasFix(i2c_inst_t *i2c, std::string& latitude, std::string& longitude, std::string& utc_time);


    int gps_fix_count{0};
    int read_gps_flag{0};
    char numcommand[max_read];
    // std::string latitude{"zero"};
    // std::string longitude{"zero"};
    // std::string utc_time{"zero"};
    // char init_command[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";

    void pa1010d_write_command(i2c_inst_t *i2c, const char command[], int com_length) 
    {
        printf("pa1010d::pa1010d_write_command \n");

        // Convert character array to bytes for writing
        uint8_t int_command[com_length];

        for (int i = 0; i < com_length; ++i) {
            int_command[i] = command[i];
            i2c_write_blocking(i2c, addr, &int_command[i], 1, true);
        }
    }

    void parse_GNMRC(char output[], char protocol[], std::string& latitude, std::string& longitude, std::string& utc_time)
    {
        // printf("pa1010d::parse_GNMRC numcommand: %s \n", output);

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
            
            // printf("Protcol:%s\n", gps_data[0]);
            // printf("UTC Time: %s\n", gps_data[1]);
            // printf("Status: %s\n", gps_data[2]);
            // printf("Latitude: %s\n", gps_data[3]);
            // printf("N/S indicator: %s\n", gps_data[4]);
            // printf("Longitude: %s\n", gps_data[5]);
            // printf("E/W indicator: %s\n", gps_data[6]);
            // printf("Speed over ground: %s\n", gps_data[7]);
            // printf("Course over ground: %s\n", gps_data[8]);
            // printf("Date: %c%c/%c%c/%c%c\n", gps_data[9][0], gps_data[9][1], gps_data[9][2], gps_data[9][3], gps_data[9][4],
                // gps_data[9][5]);
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
                latitude = "none";
                longitude = "none";
            }
        }
    }

    void read_raw(i2c_inst_t *i2c, char numcommand[]) 
    {
        uint8_t buffer[max_read];

        int i = 0;
        bool complete = false;

        int bytesRead = i2c_read_blocking(i2c, addr, buffer, max_read, false);

        // Convert bytes to characters
        if(bytesRead == PICO_ERROR_GENERIC)
        {
            printf("GPS READING ERROR, bytes= %d \n", bytesRead);
        }
        else
        {
            while (i < bytesRead && complete == false) 
            {
                numcommand[i] = buffer[i];
                // Stop converting at end of message 
                if (buffer[i] == 10 && buffer[i + 1] == 10) 
                {
                    complete = true;
                }
                i++;
            }
        }
    }

    // TODO improve
    bool HasFix(i2c_inst_t *i2c, std::string& latitude, std::string& longitude, std::string& utc_time)
    {
        if(gps_fix_count < 5)  // if GPS does NOT have a fix
        {
            ++read_gps_flag;
            if(read_gps_flag == 10)
            {
                printf("HasFix: gps_fix_count = %i \n", gps_fix_count);
                read_gps_flag = 0;
                memset(numcommand, 0, max_read);
                pa1010d::read_raw(i2c, numcommand);
                pa1010d::parse_GNMRC(numcommand, "GNRMC", latitude, longitude, utc_time);

                if(latitude != "none")
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

        return true;
    }
}


#endif //PA1010D_H