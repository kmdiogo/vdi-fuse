#ifndef VDIFUSE_UTILS_H
#define VDIFUSE_UTILS_H

#include "vdi_structs.h"
#include "stdint.h"
#include "stdio.h"

void printBytes(const uint8_t* buffer, size_t size, char* description);
uint32_t getNumBlockGroups(const VDIFile* vdi);
uint32_t power(uint32_t base, uint32_t power);
int checkPowerOf(uint32_t power, uint32_t number);
uint32_t numEntriesBitmap(const uint8_t* bitmap, uint32_t numEntries);

#endif //VDIFUSE_UTILS_H
