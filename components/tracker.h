#ifndef TRACKER_H
#define TRACKER_H

#include <stdint.h>

#include "../drivers/lsm303d.h"
#include "../drivers/lsm6dsox.h"
#include "../drivers/pa1010d.h"
#include "../drivers/storage.h"
#include "screen_controller.h"

class Tracker
{
public:
    Tracker(ScreenController& screen, Storage& storage, GPS& gps, i2c_inst_t *i2c_imu, i2c_inst_t *i2c_compass);

    void Update();
    void Reset();

private:
    std::string GetTrackingDuration();
    std::string GetDistance();


    ScreenController& screen_;
    Storage& storage_;
    GPS& gps_;
    Imu imu_;
    Compass compass_;
    SensorsData sensors_data_;
    uint8_t tick_counter_;
    
    // --- trainig-duration
    uint8_t seconds_{0};
    uint8_t minutes_{0};
    uint8_t hours_{0};
};


#endif // TRACKER_H