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


typedef struct
{
    //TODO: refactor all of these to camel case
    uint32_t totalInodes;
    uint32_t totalBlocks;
    uint32_t superUserBlocks;
    uint32_t unallocatedBlocks;
    uint32_t unallocatedInodes;
    uint32_t superBlockNumber;
    uint32_t log2BlockSize;
    uint32_t log2FragmentSize;
    uint32_t blocksPerGroup;
    uint32_t fragmentsPerGroup;
    uint32_t inodesPerGroup;
    uint32_t lastMountTime;
    uint32_t lastWriteTime;
    //number of times mounted since last consistency check
    uint16_t timesMounted;
    // number of mounts allowed before consistency check
    uint16_t mountsAllowed;
    // should be 0xEF53
    uint16_t magic;
    // 1 means clean, 2 means system has errors
    uint16_t systemState;
    // what to do when there is an error
    // 1 - ignore error, 2 - remount file system as read only, 3 - kernel panic
    uint16_t errorHandler;
    // combine with major portion to get full version
    uint16_t minorVersion;
    // POSIX time of last consistency check
    uint32_t lastCheck;
    // interval between forced consistency checks
    uint32_t interval;
    // 0 - linux, 1 - GNU HURD, 2 - MASIX, 3 - FREEBSD, 4 - other "lites"
    uint32_t opSysId;
    uint32_t majorPortion;
    // user id that can use reserved blocks
    uint16_t userId;
    // group id that can use reserved blocks
    uint16_t groupId;

    uint32_t numBlockGroups;
    // left shift 1024 by log2PageSize to get page size
    uint32_t blockSize;

    uint8_t fullArray[SUPERBLOCK_SIZE];

} SuperBlock;


typedef struct //BlockGroupDescriptor
{
    uint32_t blockUsageBitmap;
    uint32_t inodeUsageBitmap;
    uint32_t inodeTableAddress;
    uint16_t numUnallocatedBlocks;
    uint16_t numUnallocatediNodes;
    uint16_t numDirectories;

    // some unused bytes here

}BlockGroupDescriptor;

typedef struct {
    BlockGroupDescriptor ** blockGroupDescriptorTable;
    uint8_t* blockGroupDescriptorFullContents;
    Header* header;
    int fileDescriptor;
    FILE* f;
    long long cursor;
    SuperBlock* superBlock;

} VDIFile;

#endif //VDIFUSE_VDI_STRUCTS_H
