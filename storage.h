#ifndef STORAGE_H
#define STORAGE_H

// We're going to erase and reprogram a region 256k from the start of flash.
// Once done, we can access this at XIP_BASE + 256k.
#define FLASH_TARGET_OFFSET (132 * 1024)
const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
int saved_pages_counter{0};
// const int max_pages = 100;
const int max_pages = (PICO_FLASH_SIZE_BYTES - FLASH_TARGET_OFFSET)/FLASH_PAGE_SIZE;

namespace storage
{
    void print_buf(const uint8_t *buf, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            printf("%02x", buf[i]);
            if (i % 16 == 15)
                printf("\n");
            else
                printf(" ");
        }
    }

    bool save_string_in_flash(std::string data_to_save)
    {
        if(saved_pages_counter < max_pages)
        {
            uint32_t offset{(FLASH_TARGET_OFFSET + saved_pages_counter*FLASH_PAGE_SIZE)};
            uint32_t ints{};

            // erase memory
            if(saved_pages_counter % 16 == 0)
            {
                ints = save_and_disable_interrupts();
                flash_range_erase(offset, FLASH_SECTOR_SIZE);
                restore_interrupts(ints);
            }

            // write to memory
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

    void read_data_from_flash(const int data_id)
    {
        if(data_id < saved_pages_counter)
        {
            print_buf((flash_target_contents+data_id*FLASH_PAGE_SIZE), FLASH_PAGE_SIZE);
        }
        else
        {
            printf("ERROR: trying to read memory out of scope.");
        }
    }
}


#endif //STORAGE_H