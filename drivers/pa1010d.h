#ifndef PA1010D_H
#define PA1010D_H

#include "hardware/i2c.h"
#include <string>

#include "../core/sensors_data.h"

#define ADDR        0x10
#define MAX_READ    250

class GPS
{
public:
    GPS();
    ~GPS();
    bool HasFix(SensorsData& sensors_data);
    bool ReadData1Per10(SensorsData& sensors_data, int& error);
    int TestConnection();

private:
    void ParseGNMRC(char output[], char protocol[], std::string& latitude, std::string& longitude, std::string& utc_time);
    int ReadRaw(SensorsData& sensors_data);

    i2c_inst_t *i2c_{i2c1};
    char init_command[48] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
    int gps_fix_count{0};
    int read_gps_flag{0};
    char numcommand[MAX_READ];
    // 140814.000,4124.2987N,00212.3749E
    double mock_latitude{1000.0000};
    double mock_longitude{1000.0000};
    std::string mock_time{"120000.000"};
    int error_cache{1};     //1 = no error
};


#endif //PA1010D_H