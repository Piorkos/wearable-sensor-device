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
    std::string prev_latitude{"zero"};  // in DMM
    std::string prev_longitude{"zero"}; // in DMM
    std::string prev_utc_time{"zero"};  // in DMM

    double delta_lat{};                 // in km
    double delta_lng{};                 // in km
    double distance{};                  // in km
};


#endif  //SENSORS_DATA_H