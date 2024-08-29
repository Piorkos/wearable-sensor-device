#include "tracker.h"

Tracker::Tracker(ScreenController& screen, Storage& storage, GPS& gps, i2c_inst_t *i2c_imu, i2c_inst_t *i2c_compass)
:screen_{screen}, storage_{storage}, gps_{gps}, imu_{i2c_imu}, compass_{i2c_compass}, sensors_data_{}, tick_counter_{0}
{
    printf("Tracker Constructor \n");
}

void Tracker::Update()
{
    printf("Tracker::Update tick_counter_ = %i \n", tick_counter_);
    ++tick_counter_;
    
    // ---Read data from sensors
    compass_.Read(sensors_data_);
    imu_.ReadAccelerometer(sensors_data_);
    imu_.ReadGyroscope(sensors_data_);

    // ---Read data from GPS
    int gps_error{0};
    bool gps_data = gps_.ReadData1Per10(sensors_data_, gps_error);
    if (gps_error < 0)
    {
        screen_.ShowError("E:GPS");
        // TODO maybe it would be good to reset GPS here
    }
    
    // ---Calculate distance
    if(gps_data)
    {
        // TODO calculate distance
        // distance::CalculateDistance(sensors_data_);
    }

    // ---write data to FLASH
    int success = storage_.UpdateDataToStore(sensors_data_, gps_data);
    printf("MAIN::training: UpdateDataToStore result = %i \n", success);

    // ---Update Screen every 1s
    if(tick_counter_ > 10)
    {
        tick_counter_ = 0;
        screen_.RefreshTrackingScreen(GetTrackingDuration(), GetDistance());
    }
}

void Tracker::Reset()
{
    tick_counter_ = 0;
    seconds_ = 0;
    minutes_ = 0;
    hours_ = 0;
}


std::string Tracker::GetTrackingDuration()
{
    // printf("Tracker::GetTrackingDuration  \n");
    std::string time{};

    ++seconds_;
    if(seconds_ == 60)
    {
        seconds_ = 0;
        ++minutes_;
    }
    if(minutes_ == 60)
    {
        minutes_ = 0;
        ++hours_;
    }     

    if(hours_ < 1)
    {
        if(seconds_ < 10)
        {
            time = std::to_string(minutes_) + ":0" + std::to_string(seconds_);
        }
        else
        {
            time = std::to_string(minutes_) + ":" + std::to_string(seconds_);
        }
    }
    else
    {
        if(minutes_ < 10)
        {
            time = std::to_string(hours_) + ":0" + std::to_string(minutes_);
        }
        else
        {
            time = std::to_string(hours_) + ":" + std::to_string(minutes_);
        }

    }

    return time;
}

std::string Tracker::GetDistance()
{
    // printf("Tracker::GetDistance  \n");
    int run_distance{0};
    std::string distance_s{std::to_string(run_distance) + "M"};

    return distance_s;
}