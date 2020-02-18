//
// Created by Kenny on 2/16/2020.
//

#ifndef VDIFUSE_VDI_STRUCTS_H
#define VDIFUSE_VDI_STRUCTS_H

#include <stdint.h>
#include <stdio.h>

#define SUPERBLOCK_SIZE 1024

typedef struct DiskGeometry
{
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectors;
    uint32_t sectorSize;
} DiskGeometry;


typedef struct
{
    //block = pages
    uint8_t preHeader[72];
    uint32_t headerSize;
    uint32_t imageType;
    uint32_t imageFlags;
    uint8_t imageDescription[32];
    DiskGeometry* diskGeometry;
    uint32_t offsetPages;
    uint32_t offsetData;
    uint32_t unused;
    long long diskSize;
    uint32_t pageSize;
    uint32_t pageExtraData;
    uint32_t pagesInHDD;
    uint32_t pagesAllocated;
    uint8_t UUID[16];
    uint8_t UUIDLastSnap[16];
    uint8_t UUIDLink[16];
    uint8_t UUIDParent[16];
    uint8_t notNeeded[56];

} Header;


typedef struct {
    Header* header;
    int fileDescriptor;
    FILE* f;
    long long cursor;

} VDIFile;

#endif //VDIFUSE_VDI_STRUCTS_H
