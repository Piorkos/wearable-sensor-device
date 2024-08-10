#include "hardware/flash.h"
#include "hardware/sync.h"
#include "storage.h"

Storage::Storage() : saved_pages_counter{0}, trainings_counter{0}
{
    printf("Storage Constructor \n");
    printf("Storage flash_storage_start = %x8 \n", flash_storage_start);
    printf("Storage flash_storage_start_offset = %x8 \n", flash_storage_start_offset);
    printf("Storage max_pages = %u \n", max_pages);

    RestoreSavedPagesCounter();

}

Storage::~Storage()
{
}

void Storage::AddNewTrainingMark()
{
    data_to_store += "$";
}

/**
 * @brief Iterate through all pages in memory, untill it finds empty page. It updates the saved_pages_counter.
 * 
 * @return int position of first empty page
 */
int Storage::RestoreSavedPagesCounter()
{
    printf("storage::RestoreSavedPagesCounter saved_pages_counter = %i \n", saved_pages_counter);

    // TODO restore trainings_counter from the memory

    for (int i = 0; i < max_pages; ++i) 
    {
        if(flash_storage_start[(i*FLASH_PAGE_SIZE)] == 0xFF)
        {
            saved_pages_counter = i;
            printf("storage::RestoreSavedPagesCounter RESTORED saved_pages_counter = %i \n", saved_pages_counter);
            return i;
        }
    }
    return -1;
}

/**
 * @brief Saves data on next empty page in FLASH memory. Page is the size of 256 bytes.
 * 
 * @param data_to_save  max 256 bytes
 * @return true if saving succesful
 * @return false if the memory is full
 */
bool Storage::SaveStringInFlash(std::string data_to_save)
{
    printf("storage::SaveStringInFlash saved_pages_counter = %i \n", saved_pages_counter);
    if(saved_pages_counter < max_pages)
    {
        uint32_t offset{(flash_storage_start_offset + (saved_pages_counter + 16)*FLASH_PAGE_SIZE)};
        uint32_t ints{};

        // erase memory of next page
        if(saved_pages_counter % 16 == 0)
        {
            printf("storage::SaveStringInFlash erase sector %i \n", (saved_pages_counter + 1));
            ints = save_and_disable_interrupts();
            flash_range_erase(offset, FLASH_SECTOR_SIZE);
            restore_interrupts(ints);
        }

        offset = (flash_storage_start_offset + saved_pages_counter*FLASH_PAGE_SIZE);

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
int Storage::UpdateDataToStore(SensorsData& sensors_data, bool include_gps)
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

        // // only for debugging purposes
        // data_to_store += std::to_string(sensors_data.lat_dd) + ";";
        // data_to_store += std::to_string(sensors_data.lng_dd) + ";";
        // data_to_store += std::to_string(sensors_data.delta_lat) + ";";
        // data_to_store += std::to_string(sensors_data.delta_lng) + ";";
        // data_to_store += std::to_string(sensors_data.distance) + ";";
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
        bool success = SaveStringInFlash(string_to_store);
        if(!success)
        {
            printf("ERROR: flash memory full\n");

            return -1;
        }
        return 1;
    }

    return 2;
}

void Storage::PrintBuf(const uint8_t *buf, size_t len) 
{
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", buf[i]);
        if (i % 16 == 15)
            printf("\n");
        else
            printf(" ");
    }
}

/**
 * @brief Reads and prints page of data (256 bytes) from the memory.
 * 
 * @param page_id positione of the page to be read
 */
void Storage::ReadDataFromFlash(const int page_id)
{
    if(page_id < saved_pages_counter)
    {
        PrintBuf((flash_storage_start+page_id*FLASH_PAGE_SIZE), FLASH_PAGE_SIZE);
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
void Storage::ReadAllData()
{
    printf("storage::ReadAllData \n");
    for (int i = 0; i < saved_pages_counter; i++)
    {
        ReadDataFromFlash(i);
    }
}

/**
 * @brief Erases all data, what it actually does is erasing first 3 sectors (16*256 bytes) and setting saved_pages_counter=0
 * 
 */
void Storage::EraseData()
{
    printf("storage::EraseData \n");
    
    uint32_t ints{};
    ints = save_and_disable_interrupts();
    flash_range_erase(flash_storage_start_offset, 3*FLASH_SECTOR_SIZE);
    restore_interrupts(ints);

    saved_pages_counter = 0;
}

/**
 * @brief Full, real FLASH erase. Unlike EraseData(), this method erase all pages. NOT SURE IF WORKS GOOD!
 * 
 */
void Storage::FullEraseData()
{
    printf("storage::FullEraseData 1.0 \n");

    int max_sectors = max_pages/16;
    uint32_t offset{flash_storage_start_offset};
    
    uint32_t ints{};
    uint8_t factor{20};


    for (int i = 1; i < max_sectors; (i + factor)) 
    {
        printf("storage::FullEraseData - sector = %i \n", i);
        ints = save_and_disable_interrupts();
        flash_range_erase(offset, factor*FLASH_SECTOR_SIZE);
        restore_interrupts(ints);

        offset = (flash_storage_start_offset + i*FLASH_SECTOR_SIZE);
    }
    

    saved_pages_counter = 0;
}