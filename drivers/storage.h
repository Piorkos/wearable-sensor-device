#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include "pico/binary_info.h"

#include "../utils/sensors_data.h"

// We're going to erase and reprogram a region 256k from the start of flash.
// Once done, we can access this at XIP_BASE + 256k.
#define FLASH_TARGET_OFFSET (132 * 1024)
const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
static int saved_pages_counter{0};
static int trainings_counter{0};
const int max_pages = 65'536;                                                            // Arduino Nano RP2040
// const int max_pages = (PICO_FLASH_SIZE_BYTES - FLASH_TARGET_OFFSET)/FLASH_PAGE_SIZE;     // Pi Pico
static std::string data_to_store{};

/**
 * @brief Stored data structure
 * "$" - sign for new training
 * "|" - sign for new set of data
 * ";" - separator between data fields
 * 
 */
namespace storage
{
    void StartNewTraining()
    {
        ++trainings_counter;
        data_to_store += "$" + std::to_string(trainings_counter);
        // TODO saved trainings_counter in FLASH memory
    }

    int GetTrainingsCounter()
    {
        return trainings_counter;
    }

    /**
     * @brief Iterate through all pages in memory, untill it finds empty page. It updates the saved_pages_counter.
     * 
     * @return int position of first empty page
     */
    int RestoreSavedPagesCounter()
    {
        printf("storage::RestoreSavedPagesCounter saved_pages_counter = %i \n", saved_pages_counter);

        // TODO restore trainings_counter from the memory

        for (int i = 0; i < max_pages; ++i) 
        {
            if(flash_target_contents[(i*FLASH_PAGE_SIZE)] == 0xFF)
            {
                printf("storage::RestoreSavedPagesCounter RESTORED saved_pages_counter = %i \n", i);
                saved_pages_counter = i;
                return i;
            }
        }
        return -1;
    }

    void PrintBuf(const uint8_t *buf, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            printf("%02x", buf[i]);
            if (i % 16 == 15)
                printf("\n");
            else
                printf(" ");
        }
    }

    /**
     * @brief Saves data on next empty page in FLASH memory. Page is the size of 256 bytes.
     * 
     * @param data_to_save 
     * @return true if saving succesful
     * @return false if the memory is full
     */
    bool SaveStringInFlash(std::string data_to_save)
    {
        printf("storage::SaveStringInFlash \n");
        if(saved_pages_counter < max_pages)
        {
            uint32_t offset{(FLASH_TARGET_OFFSET + (saved_pages_counter + 16)*FLASH_PAGE_SIZE)};
            uint32_t ints{};

            // erase memory of next page
            if(saved_pages_counter % 16 == 0)
            {
                printf("storage::SaveStringInFlash erase sector %i \n", (saved_pages_counter + 1));
                ints = save_and_disable_interrupts();
                flash_range_erase(offset, FLASH_SECTOR_SIZE);
                restore_interrupts(ints);
            }

            offset = (FLASH_TARGET_OFFSET + saved_pages_counter*FLASH_PAGE_SIZE);

            // For writing data first time, erase memory of current (i.e. = 0) page
            if(saved_pages_counter == 0)
            {
                printf("storage::SaveStringInFlash erase first sector \n");
                ints = save_and_disable_interrupts();
                flash_range_erase(offset, FLASH_SECTOR_SIZE);
                restore_interrupts(ints);
            }

            // write data to current page
            ints = save_and_disable_interrupts();
            flash_range_program(offset, reinterpret_cast<const unsigned char*>(data_to_save.c_str()), FLASH_PAGE_SIZE);
            restore_interrupts(ints);

            ++saved_pages_counter;

            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * @brief Converts sensors data into string. Is the string's size is bigger than FLASH_PAGE_SIZE, then it will be written to the FLASH memory.
     * Otherwise the string will cached and waiting for next data, untill it's size will be bigger than FLASH_PAGE_SIZE.
     * 
     * @param sensors_data Data from sensors, which should be saved in memory.
     * @param include_gps Flag indicating if data should contain GPS as well.
     * @return int 1 - data converted and written to FLASH memory, 2 - data converted and cached, but not written to FLASH memory yet, -1 - FLASH memory full, data not saved
     */
    int UpdateDataToStore(SensorsData& sensors_data, bool include_gps)
    {
        printf("storage::UpdateDataToStore - include_gps=%b\n", include_gps);

        data_to_store += "|";
        if(include_gps)
        {
            data_to_store += sensors_data.utc_time;
            data_to_store += ";";
            data_to_store += sensors_data.latitude;
            data_to_store += ";";
            data_to_store += sensors_data.longitude;
            data_to_store += ";";

            // only for debugging purposes
            data_to_store += std::to_string(sensors_data.lat_dd) + ";";
            data_to_store += std::to_string(sensors_data.lng_dd) + ";";
            data_to_store += std::to_string(sensors_data.delta_lat) + ";";
            data_to_store += std::to_string(sensors_data.delta_lng) + ";";
            data_to_store += std::to_string(sensors_data.distance) + ";";
        }

        std::string compass_coordinates = std::to_string(sensors_data.mag_x) + ";" + std::to_string(sensors_data.mag_y) + ";" + std::to_string(sensors_data.mag_z);
        data_to_store += compass_coordinates;
        data_to_store += ";";

        std::string accel_str = std::to_string(sensors_data.accelerometer[0]) + ";" + std::to_string(sensors_data.accelerometer[1]) + ";" + std::to_string(sensors_data.accelerometer[2]);
        std::string gyr_str = std::to_string(sensors_data.gyroscope[0]) + ";" + std::to_string(sensors_data.gyroscope[1]) + ";" + std::to_string(sensors_data.gyroscope[2]);
        data_to_store += accel_str;
        data_to_store += ";";
        data_to_store += gyr_str;

        printf("storage::UpdateDataToStore - data_to_store=%s\n", data_to_store.c_str());

        if(data_to_store.length() > FLASH_PAGE_SIZE)
        {
            std::string string_to_store = data_to_store.substr(0, FLASH_PAGE_SIZE);
            data_to_store.erase(0, FLASH_PAGE_SIZE);
            printf("storage::UpdateDataToStore - string sent to storage:%s\n", string_to_store.c_str());
            bool success = storage::SaveStringInFlash(string_to_store);
            if(!success)
            {
                printf("ERROR: flash memory full\n");

                return -1;
            }
            return 1;
        }

        return 2;
    }

    /**
     * @brief Reads and prints page of data (256 bytes) from the memory.
     * 
     * @param page_id positione of the page to be read
     */
    void ReadDataFromFlash(const int page_id)
    {
        if(page_id < saved_pages_counter)
        {
            PrintBuf((flash_target_contents+page_id*FLASH_PAGE_SIZE), FLASH_PAGE_SIZE);
        }
        else
        {
            printf("ERROR: trying to read memory out of scope.");
        }
    }

    /**
     * @brief Reads and prints all pages of data from the FLASH memory.
     * 
     */
    void ReadAllData()
    {
        printf("storage::ReadAllData \n");
        for (int i = 0; i < saved_pages_counter; i++)
        {
            ReadDataFromFlash(i);
        }
    }

    /**
     * @brief Erases all data, what it actually does is erasing first sector (16*256 bytes) and setting saved_pages_counter=0
     * 
     */
    void EraseData()
    {
        printf("storage::EraseData \n");
        
        uint32_t ints{};
        ints = save_and_disable_interrupts();
        flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
        restore_interrupts(ints);

        saved_pages_counter = 0;
    }
}


#endif //STORAGE_H