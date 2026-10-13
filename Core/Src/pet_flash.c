/**
 * @file    pet_flash.c
 * @brief   Flash save/load for the pet's stats - See pet_flash.h for the layout.
 *
 * Owner: Marly (pet stats)
 */

#include "pet_flash.h"
#include "stm32l4xx_hal.h"   /* HAL_FLASH_Unlock/Program/Lock, HAL_FLASHEx_Erase */

/* Index of the next empty slot - or -1 if we haven't scanned the page yet */
static int16_t next_slot = -1;

/* Read one 64-bit slot straight from flash (flash is memory-mapped)*/
static uint64_t read_slot(uint16_t slot) {
  return *(volatile const uint64_t *)(PETFLASH_PAGE_ADDR + (uint32_t)slot * 8U);
}

/* Pack the pet struct into one record (layout in pet_flash.h) */
static uint64_t pack(const PetStats_t *p) {
  return ((uint64_t)PETFLASH_MAGIC              << 48) |
         ((uint64_t)p->hunger                   << 40) |
         ((uint64_t)p->happiness                << 32) |
         ((uint64_t)(uint8_t)~p->hunger         << 24) |
         ((uint64_t)(uint8_t)~p->happiness      << 16) |
         ((uint64_t)p->state                    <<  8) |
         ((uint64_t)(uint8_t)~p->state);
}

/* Check a record and unpack it - Returns 1 if it is a real, uncorrupted record */
static uint8_t unpack(uint64_t rec, PetStats_t *out) {
  uint8_t h  = (uint8_t)(rec >> 40);
  uint8_t p  = (uint8_t)(rec >> 32);
  uint8_t nh = (uint8_t)(rec >> 24);
  uint8_t np = (uint8_t)(rec >> 16);
  uint8_t s  = (uint8_t)(rec >>  8);
  uint8_t ns = (uint8_t)(rec);

  if ((uint16_t)(rec >> 48) != PETFLASH_MAGIC) return 0;   /* not ours / erased */
  if ((uint8_t)~h != nh || (uint8_t)~p != np || (uint8_t)~s != ns) return 0;   /* corrupted */

  out->hunger    = h;
  out->happiness = p;
  out->state     = s;
  return 1;
}

/* Find the first erased slot (all 1s). Returns PETFLASH_SLOTS if the page is full. */
static uint16_t find_next_slot(void) {
  uint16_t slot = 0;
  while (slot < PETFLASH_SLOTS && read_slot(slot) != 0xFFFFFFFFFFFFFFFFULL) {
    slot++;
  }
  return slot;
}

uint8_t PetFlash_Load(PetStats_t *out) {
  uint8_t found = 0;

  next_slot = (int16_t)find_next_slot();

  /* Walk backwards from the newest written slot to the first valid record */
  for (int16_t slot = next_slot - 1; slot >= 0; slot--) {
    if (unpack(read_slot((uint16_t)slot), out)) {
      found = 1;
      break;
    }
  }
  return found;
}

/* Erase the whole save page -Only happens once every PETFLASH_SLOTS saves */
static uint8_t erase_page(void) {
  FLASH_EraseInitTypeDef erase = {0};
  uint32_t page_error = 0;

  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.Banks     = FLASH_BANK_2;          /* PETFLASH_BANK */
  erase.Page      = PETFLASH_PAGE_NUM;
  erase.NbPages   = 1;

  return (HAL_FLASHEx_Erase(&erase, &page_error) == HAL_OK) ? 1U : 0U;
}

uint8_t PetFlash_Save(const PetStats_t *in) {
  uint8_t ok = 1;

  if (next_slot < 0) next_slot = (int16_t)find_next_slot();   /* Load() not called yet */

  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);   /* leftover error flags block writes */

  /* Page full: erase it and start over at slot 0 */
  if (next_slot >= (int16_t)PETFLASH_SLOTS) {
    ok = erase_page();
    next_slot = 0;
  }

  if (ok) {
    uint32_t addr = PETFLASH_PAGE_ADDR + (uint32_t)next_slot * 8U;
    ok = (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, pack(in)) == HAL_OK) ? 1U : 0U;
    next_slot++;                                   /* move on even if it failed: never rewrite a slot */
  }

  HAL_FLASH_Lock();
  return ok;
}