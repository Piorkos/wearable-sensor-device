#ifndef DISTANCE_H
#define DISTANCE_H

#include <cmath>
#include <string>
#include "sensors_data.h"


namespace distance
{
    uint8_t data_counter{0};
    double lat1{0};
    double lat2{0};
    double lng1{0};
    double lng2{0};
            
    void Reset();
    std::string AddLeadingZeros(int number);
    double DMMtoDD(const std::string& dmm);
    void CalculateDistance(SensorsData& sensors_data);


    /**
     * @brief Sets all distance variables to initial value.
     * 
     */
    void Reset()
    {
        data_counter = 0;
        lat1 = 0;
        lat2 = 0;
        lng1 = 0;
        lng2 = 0;
    }

    /**
     * @brief Calculates distance between two points described by latitude and longitude.
     * It is using algorithm which does not require much computation and has big precision 
     * for distances smaller than 100 miles (max. error 0.36%). For bigger distance
     * the error grows, the bigger the distance the bigger the error.
     * For distances greater than 500 miles is better to use different formula, e.g. 'heversine'
     * 
     * @param sensors_data Object which contains points between which distance should be calculated.
     * Distance (result) will be writted in this object.
     */
    void CalculateDistance(SensorsData& sensors_data)
    {
        if(sensors_data.latitude != "zero")
        {
            ++data_counter;
            lat2 += DMMtoDD(sensors_data.latitude);
            lng2 += DMMtoDD(sensors_data.longitude);

            if(data_counter > 2)    //use average of 3 data readings
            {
                data_counter = 0;

                lat2 = lat2/3;
                lng2 = lng2/3;

                if(lat1 != 0)
                {
                    constexpr int meridian_length{20004};   // in km
                    constexpr int equator_length{40075};    // in km
                    double rad_lat{};                       // in radians


                    rad_lat = ((lat1 + lat2) / 2) * M_PI / 180;
                    sensors_data.delta_lat = meridian_length * (lat2 - lat1) / 180;
                    sensors_data.delta_lng = equator_length * (lng2 - lng1) / 360 * std::cos(rad_lat);
                    
                    sensors_data.distance = std::sqrt(sensors_data.delta_lat*sensors_data.delta_lat + sensors_data.delta_lng*sensors_data.delta_lng);        
                }

                lat1 = lat2;
                lng1 = lng2;
                lat2 = 0;
                lng2 = 0;
            }
        }
    }

    /**
     * @brief Converts degrees and decimal minutes (DMM) to decimal degrees (DD). DMM is of a string type and has S/N, E/W to indicate direction.
     * DD is of a float type and has +/- to indicate direction.
     * 
     * @param dmm value in degrees and decimal minutes
     * @return float value in decimal degrees.
     */
    double DMMtoDD(const std::string& dmm)
    {   
        printf("distance::DMMtoDD - %s \n", dmm.c_str());
        int32_t degree{};
        int32_t minutes{};
        int32_t decimal_minutes{};
        int sign{1};
        double dd{};
        if(dmm.find_last_of("NE") == std::string::npos)
        {
            sign = -1;
        }

        std::size_t pos =  dmm.find_last_of(".");
        degree = std::stoi(dmm.substr(0, (pos-2)));
        minutes = std::stoi(dmm.substr((pos-2), pos)) * 10'000'000 / 60;
        decimal_minutes = std::stoi(dmm.substr((pos+1), (dmm.length()-1))) * 1000 / 60;
        printf("integers: %i - %i - %i \n", degree, minutes, decimal_minutes);
        minutes = minutes + decimal_minutes;
        printf("integers 2: %i - %i - %i \n", degree, minutes, decimal_minutes);

        std::string dd_string{std::to_string(degree) + "." + AddLeadingZeros(minutes)};
        printf("dd string: %s \n", dd_string.c_str());

        dd = std::stod(dd_string) * sign;
        // printf("dd float: %f \n", dd);
        printf("dd pFloa: %3.8fd \n", dd);

        return dd;
    }

    std::string AddLeadingZeros(int number)
    {   
        std::string s_number{std::to_string(number)};
        int length{s_number.length()};

        switch (length)
        {
        case 1:
            s_number = "000000" + s_number;
            break;
        case 2:
            s_number = "00000" + s_number;
            break;
        case 3:
            s_number = "0000" + s_number;
            break;
        case 4:
            s_number = "000" + s_number;
            break;
        case 5:
            s_number = "00" + s_number;
            break;
        case 6:
            s_number = "0" + s_number;
            break;
        default:
            break;
        }

        return s_number;
    }


    void TestCalculateDistance(SensorsData& sd, const double expected_distance)
    {
        // CalculateDistance(sd);
        // printf("distance::TestCalculateDistance: %2.8f - %2.8f || %2.8f == %2.8f \n \n", sd.delta_lat, sd.delta_lng, sd.distance, expected_distance);
    }

    void TestSuiteCalculateDistance()
    {
        // SensorsData sd0;
        // sd0.prev_latitude = "1000.0000S";
        // sd0.prev_longitude = "200.1000W";
        // sd0.latitude = "1100.0000S";
        // sd0.longitude = "200.1000W";
        // double expected_distance_0 = 111.2;
        // TestCalculateDistance(sd0, expected_distance_0);
    }


}


#endif  // DISTANCE_H