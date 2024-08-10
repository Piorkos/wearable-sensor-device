#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include "pico/binary_info.h"
#include "hardware/flash.h"
#include "pico/binary_info.h"

#include "../utils/sensors_data.h"

extern uint8_t __flash_binary_end;

/**
 * @brief Stored data structure
 * "$" - sign for new training
 * "|" - sign for new set of data
 * ";" - separator between data fields
 * 
 */
class Storage
{
public:
    Storage();
    ~Storage();
    
    void AddNewTrainingMark();
    
    /**
     * @brief Saves data on next empty page in FLASH memory. Page is the size of 256 bytes.
     * 
     * @param data_to_save max 256 bytes
     * @return true if saving succesful
     * @return false if the memory is full
     */
    bool SaveStringInFlash(std::string data_to_save);
    
    /**
     * @brief Converts sensors data into string. Is the string's size is bigger than FLASH_PAGE_SIZE, then it will be written to the FLASH memory.
     * Otherwise the string will cached and waiting for next data, untill it's size will be bigger than FLASH_PAGE_SIZE.
     * 
     * @param sensors_data Data from sensors, which should be saved in memory.
     * @param include_gps Flag indicating if data should contain GPS as well.
     * @return int 1 - data converted and written to FLASH memory, 2 - data converted and cached, but not written to FLASH memory yet, -1 - FLASH memory full, data not saved
     */
    int UpdateDataToStore(SensorsData& sensors_data, bool include_gps);

    /**
     * @brief Reads and prints page of data (256 bytes) from the memory.
     * 
     * @param page_id positione of the page to be read
     */
    void ReadDataFromFlash(const int page_id);
    
    /**
     * @brief Reads and prints all pages of data from the FLASH memory.
     * 
     */
    void ReadAllData();
    
    /**
     * @brief Erases all data, what it actually does is erasing first 3 sectors (16*256 bytes) and setting saved_pages_counter=0
     * 
     */
    void EraseData();
    
    /**
     * @brief Full, real FLASH erase. Unlike EraseData(), this method erase all pages. NOT SURE IF WORKS GOOD!
     * 
     */
    void FullEraseData();

private:
    /**
     * @brief Iterate through all pages in memory, untill it finds empty page. It updates the saved_pages_counter.
     * 
     * @return int position of first empty page
     */
    int RestoreSavedPagesCounter();

    void PrintBuf(const uint8_t *buf, size_t len);


    const uint8_t * flash_storage_start = (const uint8_t *) &__flash_binary_end;
    const uintptr_t flash_storage_start_offset = (uintptr_t) (&__flash_binary_end - XIP_BASE);
    const int max_pages = (PICO_FLASH_SIZE_BYTES - flash_storage_start_offset)/FLASH_PAGE_SIZE;     // Pi Pico

    int saved_pages_counter;
    int trainings_counter;
    std::string data_to_store{};
};


#endif //STORAGE_H