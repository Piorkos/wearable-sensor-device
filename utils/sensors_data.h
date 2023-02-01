#ifndef SENSORS_DATA_H
#define SENSORS_DATA_H

#include <string>

struct SensorsData
{
    float accelerometer[3];
    float gyroscope[3];
    int mag_x{0};
    int mag_y{0};
    int mag_z{0};
    std::string latitude{"zero"};
    std::string longitude{"zero"};
    std::string utc_time{"zero"};
};


#endif  //SENSORS_DATA_H