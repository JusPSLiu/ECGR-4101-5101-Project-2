/**
 * @file    pet_flash.h
 * @brief   Save/load the global pet struct (hunger, happiness, state) in the
 *          STM32's internal flash, so the pet survives a power-off or reset.
 *
 * Owner: Marly (pet stats)
 *
 * How it works:
 *   - Uses the LAST 2 KB page of flash (bank 2, page 255) The linker script
 *     is shortened by 2 KB so program code can never be placed there
 *   - Flash can only be erased a whole page at a time (slow, ~10,000 times max),
 *     but it can be written 8 bytes (one "double word") at a time.
 *   - So the page is used as a log: each save writes the next empty 8-byte slot.
 *     The page is only erased when all 256 slots are full.
 *   - On load - the newest valid slot wins
 *
 * Record layout (one 64-bit double word):
 *   bits 63..48  magic number 0x5045 ("PE") - marks a real record
 *   bits 47..40  hunger
 *   bits 39..32  happiness
 *   bits 31..24  ~hunger      (inverted copy, catches corrupted writes)
 *   bits 23..16  ~happiness   (inverted copy)
 *   bits 15..8   state       (PET_ALIVE / PET_DEAD / PET_RAN_AWAY)
 *   bits 7..0    ~state      (inverted copy)
 *   An erased slot reads as 0xFFFFFFFFFFFFFFFF, which fails the magic check.
 */

#ifndef PET_FLASH_H
#define PET_FLASH_H

#include <stdint.h>
#include "pet_stats.h"   /* PetStats_t */

#define PETFLASH_PAGE_ADDR   0x080FF800UL   /* start of the last 2 KB page of 1 MB flash */
#define PETFLASH_PAGE_SIZE   2048U          /* bytes in one flash page on the L496       */
#define PETFLASH_BANK        2U             /* the 1 MB L496 is dual bank; last page is in bank 2 */
#define PETFLASH_PAGE_NUM    255U           /* page number inside bank 2 (0..255)        */
#define PETFLASH_SLOTS       (PETFLASH_PAGE_SIZE / 8U)   /* 256 records per page          */
#define PETFLASH_MAGIC       0x5046U        /* "PE" for pet                              */

/* Find the newest saved record - Returns 1 and fills *out if one exists,
   0 if the page is empty or holds nothing valid (first power-up). */
uint8_t PetFlash_Load(PetStats_t *out);

/* Write a new record -Returns 1 on success, 0 if the flash write failed
   Blocks for ~0.1 ms normally, ~25 ms on the rare save that erases the page */
uint8_t PetFlash_Save(const PetStats_t *in);

#endif /* PET_FLASH_H */