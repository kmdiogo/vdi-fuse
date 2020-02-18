//
// Created by Kenny on 2/17/2020.
//

#ifndef VDIFUSE_EXT2_STRUCTS_H
#define VDIFUSE_EXT2_STRUCTS_H

#include <stdint.h>

typedef struct SuperBlock
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


typedef struct BlockGroupDescriptor
{
    uint32_t blockUsageBitmap;
    uint32_t inodeUsageBitmap;
    uint32_t inodeTableAddress;
    uint16_t numUnallocatedBlocks;
    uint16_t numUnallocatediNodes;
    uint16_t numDirectories;

    // some unused bytes here

} BlockGroupDescriptor;


typedef struct Inode
{
    uint16_t typePermissions;
    uint16_t userId;
    uint32_t lower32BitsSize;
    uint32_t lastAccessTime;
    uint32_t creationTime;
    uint32_t lastModificationTime;
    uint32_t deletionTime;
    uint16_t groupId;
    uint16_t hardLinkCount;
    uint32_t diskSectorCount;
    uint32_t flags;
    uint32_t opSystemValue1;
    uint32_t pointers[15];
    uint32_t generationNumber;
    // for ext2 version 0, reserved field, for ext2 version >= 1, extended attribute block
    uint32_t reservedField;
    // for ext2 version 0, reserved field, for ext2 version >= 1, upper 32 bits of file size
    uint32_t reservedField2;
    uint32_t fragmentBlockAddress;
    uint8_t opSysValue2[12];
} Inode;


typedef struct Directory
{
    Inode* inode;
    long long cursor;
    uint8_t* contents;
    char* name;
    uint32_t inodeNumber;
    uint32_t type;
} Directory;


typedef struct Ext2 {
    BlockGroupDescriptor ** blockGroupDescriptorTable;
    SuperBlock* superBlock;
    uint8_t* blockGroupDescriptorFullContents;
} Ext2;

#endif //VDIFUSE_EXT2_STRUCTS_H
