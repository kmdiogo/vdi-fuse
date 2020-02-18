//
// Created by Kenny on 2/17/2020.
//

#ifndef VDIFUSE_EXT2_H
#define VDIFUSE_EXT2_H

#include "fuse.h"
#include "vdi.h"
#include "vdif_structs.h"
#include "ext2_structs.h"

#define ALLOCATE(type) (type*)malloc(sizeof(type))
#define BLOCK_DESCRIPTOR_SIZE 32
#define SUPERBLOCK_SIZE 1024
#define REWIND_NO_DOTS 24
#define REWIND_DOTS 0

/**
 * Initializes all in-memory data structures needed
 * to navigate EXT2 filesystems.
 * @param private_data vdi-fuse context
 */
void ext2Init(VDIFData* private_data);

/**
 * Cleans up memory for all in-memory EXT2 data structures.
 * @param private_data vdi-fuse context
 */
void ext2Destroy(VDIFData* private_data);

void ext2ReadRoot(VDIFData* private_data, void *buf, fuse_fill_dir_t filler);

#endif //VDIFUSE_EXT2_H
