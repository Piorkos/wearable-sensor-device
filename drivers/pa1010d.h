#ifndef PA1010D_H
#define PA1010D_H

#include "hardware/i2c.h"
#include <string>

#include "../utils/sensors_data.h"
#include "../utils/distance.h"      // used only for tests-mock

#define ADDR        0x10
#define MAX_READ    250

class GPS
{
public:
    GPS();
    bool HasFix(i2c_inst_t *i2c, SensorsData& sensors_data);
    bool ReadData1Per10(i2c_inst_t *i2c, SensorsData& sensors_data, int& error);
    int TestConnection(i2c_inst_t *i2c);

private:
    void parse_GNMRC(char output[], char protocol[], std::string& latitude, std::string& longitude, std::string& utc_time);
    int read_raw(i2c_inst_t *i2c, SensorsData& sensors_data);

    i2c_inst_t *i2c{i2c1};
    char init_command[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
    int gps_fix_count{0};
    int read_gps_flag{0};
    char numcommand[MAX_READ];
    // 140814.000,4124.2987N,00212.3749E
    double mock_latitude{1000.0000};
    double mock_longitude{1000.0000};
    std::string mock_time{"120000.000"};
    int error_cache{1};     //1 = no error
};

namespace pa1010d
{
    int gps_fix_count{0};
    int read_gps_flag{0};
    char numcommand[MAX_READ];
    // 140814.000,4124.2987N,00212.3749E
    double mock_latitude{1000.0000};
    double mock_longitude{1000.0000};
    std::string mock_time{"120000.000"};
    int error_cache{1};     //1 = no error


    void init(i2c_inst_t *i2c, const char command[], int com_length);
    void parse_GNMRC(char output[], char protocol[], std::string& latitude, std::string& longitude, std::string& utc_time);
    int read_raw(i2c_inst_t *i2c, char numcommand[]);
    bool HasFix(i2c_inst_t *i2c, SensorsData& sensors_data);
    bool HasFixMock(i2c_inst_t *i2c, SensorsData& sensors_data);
    bool ReadData1Per10Mock(i2c_inst_t *i2c, SensorsData& sensors_data, int error);
    void MockLatLng(SensorsData& sensors_data);


    void init(i2c_inst_t *i2c, const char command[], int com_length) 
    {
        printf("pa1010d::init \n");

        // Convert character array to bytes for writing
        uint8_t int_command[com_length];

        for (int i = 0; i < com_length; ++i) {
            int_command[i] = command[i];
            if(i < (com_length - 1)){
                i2c_write_blocking(i2c, ADDR, &int_command[i], 1, true);
            }
            else{
                i2c_write_blocking(i2c, ADDR, &int_command[i], 1, false);            
            }
        }
    }

    void parse_GNMRC(char output[], char protocol[], std::string& latitude, std::string& longitude, std::string& utc_time)
    {
        printf("pa1010d::parse_GNMRC numcommand: %s \n", output);

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
            printf("Status: %s\n", gps_data[2]);
            printf("Latitude: %s\n", gps_data[3]);
            // printf("N/S indicator: %s\n", gps_data[4]);
            printf("Longitude: %s\n", gps_data[5]);
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
                latitude = "zero";
                longitude = "zero";
                utc_time = "zero";
            }
        }
    }

    int read_raw(i2c_inst_t *i2c, SensorsData& sensors_data) 
    {
        printf("pa1010d::read_raw \n");
        memset(numcommand, 0, MAX_READ);
        uint8_t buffer[MAX_READ];

        int i = 0;
        bool complete = false;

        printf("pa1010d::read_raw - 1 \n");
        int bytes_read = i2c_read_blocking(i2c, ADDR, buffer, MAX_READ, false);
        printf("pa1010d::read_raw - 2 \n");
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
        printf("pa1010d::read_raw - 3 \n");

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
        pa1010d::parse_GNMRC(numcommand, "GNRMC", sensors_data.latitude, sensors_data.longitude, sensors_data.utc_time);

        return 1;
    }

    // TODO improve
    bool HasFix(i2c_inst_t *i2c, SensorsData& sensors_data)
    {
        if(gps_fix_count < 5)  // if GPS does NOT have a fix
        {
            ++read_gps_flag;
            if(read_gps_flag == 10)
            {
                printf("HasFix: gps_fix_count = %i \n", gps_fix_count);
                read_gps_flag = 0;
                memset(numcommand, 0, MAX_READ);
                pa1010d::read_raw(i2c, sensors_data);

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
     * @param i2c 
     * @param sensors_data structure to store data read from GPS
     * @param error 1 if no error, -1 if timeout, -2 if no connection
     * @return true if this was the 1 of 10 calls when it reads data
     * @return false otherwise
     */
    bool ReadData1Per10(i2c_inst_t *i2c, SensorsData& sensors_data, int& error)
    {
        ++read_gps_flag;
        if(read_gps_flag == 10)
        {
            printf("pa1010d::ReadData1Per10 - read_gps_flag = 10 \n");
            read_gps_flag = 0;

            memset(numcommand, 0, MAX_READ);
            printf("pa1010d::ReadData1Per10 - 1 \n");
            error = pa1010d::read_raw(i2c, sensors_data);
            error_cache = error;
            printf("pa1010d::ReadData1Per10 - 2 \n");

            return true;
        }
        else
        {
            error = error_cache;
        }
        
        return false;
    }

    bool ReadData1Per10Mock(i2c_inst_t *i2c, SensorsData& sensors_data)
    {
        ++read_gps_flag;
        if(read_gps_flag == 10)
        {
            printf("pa1010d::ReadData1Per10Mock - read_gps_flag = 10 \n");
            read_gps_flag = 0;

            MockLatLng(sensors_data);

            return true;
        }
        
        return false;
    }

    void MockLatLng(SensorsData& sensors_data)
    {
        // 140814.000,4124.2987N,00212.3749E
        
        if(storage::GetTrainingsCounter() == 0)
        {
            // lat 1000.0001->1000.1000; lng=1000.0001
            if(mock_latitude < 1000.1001)    // distance 185,32 meters
            {
                mock_latitude += 0.001;    // distance ~0.1853 meter
            }
        }
        else if(storage::GetTrainingsCounter() == 1)
        {
            // lat 1000.1000->1000.2000; lng 1000.0000->1000.1000
            if(mock_longitude < 1000.1001)    // distance 260,1 meters
            {
                mock_latitude += 0.001;    // distance ~0.1853 meter
                mock_longitude += 0.001;
            }
        }
        else if(storage::GetTrainingsCounter() == 2)
        {
            // lat=1000.2000; lng 1000.1000->1000.2000
            if(mock_longitude < 1000.2001)    // distance 182,5 meters
            {
                mock_longitude += 0.001;
            }
        }
        else if(storage::GetTrainingsCounter() == 3)
        {
            // lat 1000.2->1001.0; lng 1000.2->1001.0
            if(mock_longitude < 1001.0000)    // distance 2'081 meters
            {
                mock_latitude += 0.01;    // distance ~0.1853 meter
                mock_longitude += 0.01;
            }
        }

        sensors_data.latitude = std::to_string(mock_latitude);
        sensors_data.longitude = std::to_string(mock_longitude);
        sensors_data.utc_time = mock_time;
    }

    /**
     * @brief Returns true after 5s
     * 
     * @param i2c 
     * @param sensors_data 
     * @return true 
     * @return false 
     */
    bool HasFixMock(i2c_inst_t *i2c, SensorsData& sensors_data)
    {
        if(gps_fix_count < 5)  // if GPS does NOT have a fix
        {
            ++read_gps_flag;
            if(read_gps_flag == 10)
            {
                printf("HasFixMock: gps_fix_count = %i \n", gps_fix_count);
                read_gps_flag = 0;

                ++gps_fix_count;
            }
            return false;
        }

        gps_fix_count = 0;

        return true;
    }

    /**
     * @brief Checks if connection is ok.
     * 
     * @param i2c Either i2c0 or i2c1
     * @return -2 if timeout, -1 if connection broken, 1 if connection ok
     */
    int TestConnection(i2c_inst_t *i2c)
    {
        uint8_t buffer;
        int bytes_read = i2c_read_blocking(i2c, ADDR, &buffer, 1, false);
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
}


#endif //PA1010D_H