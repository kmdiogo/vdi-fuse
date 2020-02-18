#include "../include/utils.h"

void printBytes(const uint8_t* buffer, size_t size, char* description)
{
    printf("\n%s\n", description);
    for(size_t i = 0; i<size; i++)
    {
        printf("%X ", buffer[i]);
    }
    printf("\n");
}

int checkPowerOf(uint32_t power_of, uint32_t number)
{
    uint32_t count = 0;
    while(1)
    {
        uint32_t temp = power(power_of, count);
        if(number == temp) return 1;
        if(temp > number) return 0;
        count++;
    }
    return 0;
}

uint32_t power(uint32_t base, uint32_t power)
{
    uint32_t return_val = 1;
    for(uint32_t i = 0; i<power; i++)
    {
        return_val = return_val*base;
    }
    return return_val;
}

uint32_t getNumBlockGroups(const Ext2* ext)
{
    uint32_t numBlockGroups;
    if(ext->superBlock->totalBlocks % ext->superBlock->blocksPerGroup == 0)
    {
        numBlockGroups = ext->superBlock->totalBlocks / ext->superBlock->blocksPerGroup;
    }
    else
    {
        numBlockGroups = ext->superBlock->totalBlocks / ext->superBlock->blocksPerGroup + 1;
    }

    int numBlockGroups2;
    if(ext->superBlock->totalInodes % ext->superBlock->inodesPerGroup == 0)
    {
        numBlockGroups2 = ext->superBlock->totalInodes / ext->superBlock->inodesPerGroup;
    }
    else
    {
        numBlockGroups2 = ext->superBlock->totalInodes / ext->superBlock->inodesPerGroup + 1;
    }

    if(numBlockGroups == numBlockGroups2)
    {
        return numBlockGroups;
    }
    else return 0;
}

/*
uint32_t numEntriesBitmap(const uint8_t* bitmap, uint32_t numEntries)
{
    uint32_t count = 0;
    for(uint32_t i = 0; i<numEntries; i++)
    {
        uint32_t byteIndex = i/8;
        uint32_t bitIndex = i%8;

        uint8_t temp = bitmap[byteIndex];
        temp = (temp >> bitIndex) & 1u;
        if(temp == 1u)
        {
            count++;
        }
    }
    return count;
}*/
