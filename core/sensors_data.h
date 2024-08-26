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
    std::string latitude{"zero"};       // in DMM
    std::string longitude{"zero"};      // in DMM
    std::string utc_time{"zero"};       // in DMM
    std::string prev_utc_time{"zero"};  // in DMM

    int delta_lat{0};                 // in m
    int delta_lng{0};                 // in m
    int distance{0};                  // distance between previous and current position, in m 

    // only for debugging purposes
    int32_t lat_dd{0};
    int32_t lng_dd{0};
};


#endif  //SENSORS_DATA_H