#ifndef DISTANCE_H
#define DISTANCE_H

#include <cmath>
#include <string>
#include "sensors_data.h"


namespace distance
{
    std::string AddLeadingZeros(int number);
    double DMMtoDD(const std::string& dmm);
    void CalculateDistance(SensorsData& sensors_data);

    void CalculateDistance(SensorsData& sensors_data)
    {
        constexpr int meridian_length{20004};   // in km
        constexpr int equator_length{40075};    // in km
        double lat1{};
        double lat2{};
        double lng1{};
        double lng2{};
        double rad_lat{};                       // in radians

        lat1 = DMMtoDD(sensors_data.prev_latitude);
        lat2 = DMMtoDD(sensors_data.latitude);
        lng1 = DMMtoDD(sensors_data.prev_longitude);
        lng2 = DMMtoDD(sensors_data.longitude);
        rad_lat = ((lat1 + lat2) / 2) * M_PI / 180;
        sensors_data.delta_lat = meridian_length * (lat2 - lat1) / 180;
        sensors_data.delta_lng = equator_length * (lng2 - lng1) / 360 * std::cos(rad_lat);
        
        sensors_data.distance = std::sqrt(sensors_data.delta_lat*sensors_data.delta_lat + sensors_data.delta_lng*sensors_data.delta_lng);
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
        CalculateDistance(sd);
        printf("distance::TestCalculateDistance: %2.8f - %2.8f || %2.8f == %2.8f \n \n", sd.delta_lat, sd.delta_lng, sd.distance, expected_distance);
    }

    void TestSuiteCalculateDistance()
    {
        SensorsData sd0;
        // S-W
        // -0
        sd0.prev_latitude = "1000.0000S";
        sd0.prev_longitude = "200.1000W";
        sd0.latitude = "1100.0000S";
        sd0.longitude = "200.1000W";
        double expected_distance_0 = 111.2;
        TestCalculateDistance(sd0, expected_distance_0);
        // --
        sd0.prev_latitude = "0100.0100S";
        sd0.prev_longitude = "0100.0000W";
        sd0.latitude = "0100.0100S";
        sd0.longitude = "0120.0000W";
        expected_distance_0 = 111.18;
        TestCalculateDistance(sd0, expected_distance_0);
        // +0
        sd0.prev_latitude = "1000.0020S";
        sd0.prev_longitude = "1020.0020W";
        sd0.latitude = "1000.0010S";
        sd0.longitude = "1020.0020W";
        expected_distance_0 = 0.1112;
        TestCalculateDistance(sd0, expected_distance_0);
        // ++
        sd0.prev_latitude = "1000.0001S";
        sd0.prev_longitude = "3020.0030W";
        sd0.latitude = "1000.0001S";
        sd0.longitude = "3020.0000W";
        expected_distance_0 = 0.1254;
        TestCalculateDistance(sd0, expected_distance_0);

        // N-W
        // +0
        sd0.prev_latitude = "4124.2982N";
        sd0.prev_longitude = "212.3740W";
        sd0.latitude = "4124.3982N";
        sd0.longitude = "212.3740W";
        expected_distance_0 = 11.12;
        TestCalculateDistance(sd0, expected_distance_0);
        // +-
        sd0.prev_latitude = "4124.2982N";
        sd0.prev_longitude = "212.3740W";
        sd0.latitude = "4124.3982N";
        sd0.longitude = "212.3840W";
        expected_distance_0 = 11.17;
        TestCalculateDistance(sd0, expected_distance_0);
        // -0
        sd0.prev_latitude = "1000.0020N";
        sd0.prev_longitude = "1020.0020W";
        sd0.latitude = "1000.0010N";
        sd0.longitude = "1020.0020W";
        expected_distance_0 = 0.1112;
        TestCalculateDistance(sd0, expected_distance_0);
        // -+
        sd0.prev_latitude = "1000.0020N";
        sd0.prev_longitude = "3020.0030W";
        sd0.latitude = "1000.0010N";
        sd0.longitude = "3020.0000W";
        expected_distance_0 = 0.1254;
        TestCalculateDistance(sd0, expected_distance_0);

        // N-E
        // +0
        sd0.prev_latitude = "4124.2982N";
        sd0.prev_longitude = "212.3740E";
        sd0.latitude = "4124.3982N";
        sd0.longitude = "212.3740E";
        expected_distance_0 = 11.12;
        TestCalculateDistance(sd0, expected_distance_0);
        // ++
        sd0.prev_latitude = "4124.2982N";
        sd0.prev_longitude = "212.3740E";
        sd0.latitude = "4124.3982N";
        sd0.longitude = "212.3840E";
        expected_distance_0 = 11.17;
        TestCalculateDistance(sd0, expected_distance_0);
        // -0
        sd0.prev_latitude = "1000.0020N";
        sd0.prev_longitude = "1020.0020E";
        sd0.latitude = "1000.0010N";
        sd0.longitude = "1020.0020E";
        expected_distance_0 = 0.1112;
        TestCalculateDistance(sd0, expected_distance_0);
        // --
        sd0.prev_latitude = "1000.0020N";
        sd0.prev_longitude = "3020.0030E";
        sd0.latitude = "1000.0010N";
        sd0.longitude = "3020.0000E";
        expected_distance_0 = 0.1254;
        TestCalculateDistance(sd0, expected_distance_0);
    }


}


#endif  // DISTANCE_H