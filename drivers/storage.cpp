#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/time.h"
#include "storage.h"

Storage::Storage() : saved_pages_counter_{0}, trainings_counter_{0}
{
    printf("Storage Constructor \n");
    printf("Storage kFlashStorageStart_ = %x8 \n", kFlashStorageStart_);
    printf("Storage kFlashStorageStartOffset_ = %x8 \n", kFlashStorageStartOffset_);
    printf("Storage kMaxPages_ = %u \n", kMaxPages_);

    RestoreSavedPagesCounter();

}

Storage::~Storage()
{
}

void Storage::AddNewTrainingMark()
{
    data_to_store_ += "$";
}

/**
 * @brief Iterate through all pages in memory, untill it finds empty page. It updates the saved_pages_counter_.
 * 
 * @return int position of first empty page
 */
int Storage::RestoreSavedPagesCounter()
{
    printf("storage::RestoreSavedPagesCounter saved_pages_counter_ = %i \n", saved_pages_counter_);

    // TODO restore trainings_counter_ from the memory

    for (int i = 0; i < kMaxPages_; ++i) 
    {
        if(kFlashStorageStart_[(i*FLASH_PAGE_SIZE)] == 0xFF)
        {
            saved_pages_counter_ = i;
            printf("storage::RestoreSavedPagesCounter RESTORED saved_pages_counter_ = %i \n", saved_pages_counter_);
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
    // printf("storage::SaveStringInFlash saved_pages_counter_ = %i \n", saved_pages_counter_);
    if(saved_pages_counter_ < kMaxPages_)
    {
        uint32_t offset{(kFlashStorageStartOffset_ + (saved_pages_counter_ + 16)*FLASH_PAGE_SIZE)};
        uint32_t ints{};

        // erase memory of next page
        if(saved_pages_counter_ % 16 == 0)
        {
            // printf("storage::SaveStringInFlash erase sector %i \n", (saved_pages_counter_ + 1));
            ints = save_and_disable_interrupts();
            flash_range_erase(offset, FLASH_SECTOR_SIZE);
            restore_interrupts(ints);
        }

        offset = (kFlashStorageStartOffset_ + saved_pages_counter_*FLASH_PAGE_SIZE);

        // For writing data first time, erase memory of current (i.e. = 0) page
        if(saved_pages_counter_ == 0)
        {
            // printf("storage::SaveStringInFlash erase first sector \n");
            ints = save_and_disable_interrupts();
            flash_range_erase(offset, FLASH_SECTOR_SIZE);
            restore_interrupts(ints);
        }

        // write data to current page
        ints = save_and_disable_interrupts();
        flash_range_program(offset, reinterpret_cast<const unsigned char*>(data_to_save.c_str()), FLASH_PAGE_SIZE);
        restore_interrupts(ints);

        ++saved_pages_counter_;

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
    // printf("storage::UpdateDataToStore - include_gps=%b\n", include_gps);

    data_to_store_ += "|";
    if(include_gps)
    {
        data_to_store_ += sensors_data.utc_time;
        data_to_store_ += ";";
        data_to_store_ += sensors_data.latitude;
        data_to_store_ += ";";
        data_to_store_ += sensors_data.longitude;
        data_to_store_ += ";";

        // // only for debugging purposes
        // data_to_store_ += std::to_string(sensors_data.lat_dd) + ";";
        // data_to_store_ += std::to_string(sensors_data.lng_dd) + ";";
        // data_to_store_ += std::to_string(sensors_data.delta_lat) + ";";
        // data_to_store_ += std::to_string(sensors_data.delta_lng) + ";";
        // data_to_store_ += std::to_string(sensors_data.distance) + ";";
    }

    std::string compass_coordinates = std::to_string(sensors_data.mag_x) + ";" + std::to_string(sensors_data.mag_y) + ";" + std::to_string(sensors_data.mag_z);
    data_to_store_ += compass_coordinates;
    data_to_store_ += ";";

    std::string accel_str = std::to_string(sensors_data.accelerometer[0]) + ";" + std::to_string(sensors_data.accelerometer[1]) + ";" + std::to_string(sensors_data.accelerometer[2]);
    std::string gyr_str = std::to_string(sensors_data.gyroscope[0]) + ";" + std::to_string(sensors_data.gyroscope[1]) + ";" + std::to_string(sensors_data.gyroscope[2]);
    data_to_store_ += accel_str;
    data_to_store_ += ";";
    data_to_store_ += gyr_str;

    // printf("storage::UpdateDataToStore - data_to_store_=%s\n", data_to_store_.c_str());

    if(data_to_store_.length() > FLASH_PAGE_SIZE)
    {
        std::string string_to_store = data_to_store_.substr(0, FLASH_PAGE_SIZE);
        data_to_store_.erase(0, FLASH_PAGE_SIZE);
        // printf("storage::UpdateDataToStore - string sent to storage:%s\n", string_to_store.c_str());
        sleep_ms(10);
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
    if(page_id < saved_pages_counter_)
    {
        PrintBuf((kFlashStorageStart_+page_id*FLASH_PAGE_SIZE), FLASH_PAGE_SIZE);
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
    for (int i = 0; i < saved_pages_counter_; i++)
    {
        ReadDataFromFlash(i);
    }
}

/**
 * @brief Erases all data, what it actually does is erasing first 3 sectors (16*256 bytes) and setting saved_pages_counter_=0
 * 
 */
void Storage::EraseData()
{
    printf("storage::EraseData \n");
    
    uint32_t ints{};
    ints = save_and_disable_interrupts();
    flash_range_erase(kFlashStorageStartOffset_, 3*FLASH_SECTOR_SIZE);
    restore_interrupts(ints);

    saved_pages_counter_ = 0;
}

/**
 * @brief Full, real FLASH erase. Unlike EraseData(), this method erase all pages. NOT SURE IF WORKS GOOD!
 * 
 */
void Storage::FullEraseData()
{
    printf("storage::FullEraseData 1.0 \n");

    int max_sectors = kMaxPages_/16;
    uint32_t offset{kFlashStorageStartOffset_};
    printf("max_sectors = %i \n", max_sectors);
    printf("offset =  %x8 \n", offset);
    
    uint32_t ints{};


    for (int i = 1; i < max_sectors; ++i) 
    {
        // printf("storage::FullEraseData - sector = %i, offset = %x8 \n", i, offset);
        ints = save_and_disable_interrupts();
        flash_range_erase(offset, FLASH_SECTOR_SIZE);
        restore_interrupts(ints);

        offset = (kFlashStorageStartOffset_ + i*FLASH_SECTOR_SIZE);
    }
    

    saved_pages_counter_ = 0;
}