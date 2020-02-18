#include "../include/ext2.h"

static void fetchBlock(VDIFData* private_data, uint8_t* buffer, uint32_t blockNumber);
static Inode* fetchInode(VDIFData* private_data, uint32_t iNodeNumber);
static void fetchBlockFromInode(VDIFData* private_data, Inode *inode, int blockNum, uint8_t *blockBuf);
static void fetchSingle(VDIFData* private_data, Inode* inode, int blockNum, uint8_t* blockBuf, size_t ipb, int start);
static void fetchDouble(VDIFData* private_data, Inode* inode, int blockNum, uint8_t* blockBuf, size_t ipb, int start);
static void fetchTriple(VDIFData* private_data, Inode* inode, int blockNum, uint8_t* blockBuf, size_t ipb);

static void rewindDirectory(Directory* dir, uint32_t location);
static Directory* openDirectory(VDIFData* private_data, uint32_t inodeNumber);
static void closeDirectory(Directory* dir);
static uint32_t getNextEntry(Directory* dir);

static void readSuperBlock(VDIFData* private_data);
static void readBlockDescTable(VDIFData* private_data);

void traverse(VDIFData* private_data, Directory* dir) {
    while (getNextEntry(dir)) {
        printf("%s\n", dir->name);
        Directory* next = openDirectory(private_data, dir->inodeNumber);
        if (next != NULL) {
            traverse(private_data, next);
            closeDirectory(next);
        }
    }
}

void ext2ReadRoot(VDIFData* private_data, void *buf, fuse_fill_dir_t filler) {
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    filler(buf, "file", NULL, 0);
    /*Directory* dir = openDirectory(private_data, 2);
    while(getNextEntry(dir)) {
        //printf("%s\n", dir->name);
        filler(buf, dir->name, NULL, 0);
    }
    closeDirectory(dir);*/
}

void ext2Init(VDIFData* private_data) {
    printf("EXT2: Initializing EXT2 Data Structures...\n");
    private_data->fs = malloc(sizeof(Ext2));
    Ext2* ext = private_data->fs;
    ext->superBlock = malloc(sizeof(SuperBlock));
    ext->blockGroupDescriptorTable = NULL;

    readSuperBlock(private_data);
    readBlockDescTable(private_data);

    /*Directory* dir = openDirectory(private_data, 2);
    traverse(private_data, dir);
    closeDirectory(dir);*/

}

void ext2Destroy(VDIFData* private_data) {
    printf("EXT2: Destroying EXT2 Data Structures...\n");
    Ext2* ext = private_data->fs;
    if(ext->blockGroupDescriptorTable != NULL)
    {
        for (size_t i = 0; i < ext->superBlock->numBlockGroups; i++)
        {
            free(ext->blockGroupDescriptorTable[i]);
        }
        free(ext->blockGroupDescriptorTable);
    }
    free(ext->blockGroupDescriptorFullContents);
    free(ext->superBlock);
    free(ext);
}

// Helper function that retrieves a given block by its number into a buffer
static void fetchBlock(VDIFData* private_data, uint8_t* buffer, uint32_t blockNumber)
{
    Ext2* ext = private_data->fs;
    vdiSeek(private_data->vdi, blockNumber * ext->superBlock->blockSize, VDI_SET);
    vdiRead(private_data->vdi, buffer, ext->superBlock->blockSize);
}

static void readSuperBlock(VDIFData* private_data)
{
    Ext2* ext = private_data->fs;
    uint8_t superblock[SUPERBLOCK_SIZE];
    vdiSeek(private_data->vdi, 0x400, VDI_SET);
    vdiRead(private_data->vdi, superblock, SUPERBLOCK_SIZE);

    memcpy(ext->superBlock->fullArray, superblock, SUPERBLOCK_SIZE);
    memcpy(&ext->superBlock->totalInodes, superblock, 4);
    memcpy(&ext->superBlock->totalBlocks, superblock + 4, 4);
    memcpy(&ext->superBlock->superUserBlocks, superblock + 8, 4);
    memcpy(&ext->superBlock->unallocatedBlocks, superblock + 12, 4);
    memcpy(&ext->superBlock->unallocatedInodes, superblock + 16, 4);
    memcpy(&ext->superBlock->superBlockNumber, superblock + 20, 4);
    memcpy(&ext->superBlock->log2BlockSize, superblock + 24, 4);
    memcpy(&ext->superBlock->log2FragmentSize, superblock + 28, 4);
    memcpy(&ext->superBlock->blocksPerGroup, superblock + 32, 4);
    memcpy(&ext->superBlock->fragmentsPerGroup, superblock + 36, 4);
    memcpy(&ext->superBlock->inodesPerGroup, superblock + 40, 4);
    memcpy(&ext->superBlock->lastMountTime, superblock + 44, 4);
    memcpy(&ext->superBlock->lastWriteTime, superblock + 48, 4);
    memcpy(&ext->superBlock->timesMounted, superblock + 52, 2);
    memcpy(&ext->superBlock->mountsAllowed, superblock + 54, 2);
    memcpy(&ext->superBlock->magic, superblock + 56, 2);
    memcpy(&ext->superBlock->systemState, superblock + 58, 2);
    memcpy(&ext->superBlock->errorHandler, superblock + 60, 2);
    memcpy(&ext->superBlock->minorVersion, superblock + 62, 2);
    memcpy(&ext->superBlock->lastCheck, superblock + 66, 4);
    memcpy(&ext->superBlock->interval, superblock + 70, 4);
    memcpy(&ext->superBlock->opSysId, superblock + 74, 4);
    memcpy(&ext->superBlock->majorPortion, superblock + 78, 4);
    memcpy(&ext->superBlock->userId, superblock + 80, 2);
    memcpy(&ext->superBlock->groupId, superblock + 82, 2);

    ext->superBlock->numBlockGroups = getNumBlockGroups(ext);
    if(ext->superBlock->numBlockGroups == 0)
    {
        printf("EXT2: Error determining number of block groups.\n");
        assert(1);
    }

    ext->superBlock->blockSize = 1024u << ext->superBlock->log2BlockSize;
}


static void readBlockDescTable(VDIFData* private_data)
{
    Ext2* ext = private_data->fs;

    // Initialize block group descriptor table buffer
    uint8_t blockDescTable[ext->superBlock->blockSize];
    fetchBlock(private_data, blockDescTable, 2);

    ext->blockGroupDescriptorTable = (BlockGroupDescriptor**)calloc(ext->superBlock->numBlockGroups, sizeof(BlockGroupDescriptor*));
    for(size_t i = 0; i < ext->superBlock->numBlockGroups; i++)
    {
        ext->blockGroupDescriptorTable[i] = (BlockGroupDescriptor*)malloc(sizeof(BlockGroupDescriptor));
    }

    ext->blockGroupDescriptorFullContents = (uint8_t*)calloc(ext->superBlock->blockSize, sizeof(uint8_t));
    memcpy(ext->blockGroupDescriptorFullContents, blockDescTable, ext->superBlock->blockSize);

    for(size_t i = 0; i < ext->superBlock->numBlockGroups; i++)
    {
        memcpy(&ext->blockGroupDescriptorTable[i]->blockUsageBitmap, blockDescTable + i * BLOCK_DESCRIPTOR_SIZE, 4);
        memcpy(&ext->blockGroupDescriptorTable[i]->inodeUsageBitmap, blockDescTable + i * BLOCK_DESCRIPTOR_SIZE + 4, 4);
        memcpy(&ext->blockGroupDescriptorTable[i]->inodeTableAddress, blockDescTable + i * BLOCK_DESCRIPTOR_SIZE + 8, 4);
        memcpy(&ext->blockGroupDescriptorTable[i]->numUnallocatedBlocks, blockDescTable + i * BLOCK_DESCRIPTOR_SIZE + 12, 2);
        memcpy(&ext->blockGroupDescriptorTable[i]->numUnallocatediNodes, blockDescTable + i * BLOCK_DESCRIPTOR_SIZE + 14, 2);
        memcpy(&ext->blockGroupDescriptorTable[i]->numDirectories, blockDescTable + i * BLOCK_DESCRIPTOR_SIZE + 16, 2);
    }
}


static Inode* fetchInode(VDIFData* private_data, uint32_t iNodeNumber)
{
    Ext2* ext = private_data->fs;
    uint8_t iNodeBuffer[128];
    uint32_t iNodeSize = 128;
    uint32_t blockGroup = (iNodeNumber-1) / ext->superBlock->inodesPerGroup;
    uint32_t index = (iNodeNumber-1) % ext->superBlock->inodesPerGroup;
    uint32_t containingBlock = (index*iNodeSize) / ext->superBlock->blockSize;

    Inode* inode = (Inode*)malloc(sizeof(Inode));

    // get the block that contains our inode
    uint8_t buf[1024];
    fetchBlock(private_data, buf, containingBlock + ext->blockGroupDescriptorTable[blockGroup]->inodeTableAddress);

    index = index % 8;

    memcpy(iNodeBuffer, buf+(index)*iNodeSize,iNodeSize);

    memcpy(&inode->typePermissions, iNodeBuffer, 2);
    memcpy(&inode->userId, iNodeBuffer + 2, 2);
    memcpy(&inode->lower32BitsSize, iNodeBuffer + 4, 4);
    memcpy(&inode->lastAccessTime, iNodeBuffer + 8, 4);
    memcpy(&inode->creationTime, iNodeBuffer + 12, 4);
    memcpy(&inode->lastModificationTime, iNodeBuffer + 16, 4);
    memcpy(&inode->deletionTime, iNodeBuffer + 20, 4);
    memcpy(&inode->groupId, iNodeBuffer + 24, 2);
    memcpy(&inode->hardLinkCount, iNodeBuffer + 26, 2);
    memcpy(&inode->diskSectorCount, iNodeBuffer + 28, 4);
    memcpy(&inode->flags, iNodeBuffer + 32, 4);
    memcpy(&inode->opSystemValue1, iNodeBuffer + 36, 4);
    memcpy(inode->pointers, iNodeBuffer + 40, 60);
    memcpy(&inode->generationNumber, iNodeBuffer + 100, 4);
    memcpy(&inode->reservedField, iNodeBuffer + 104, 4);
    memcpy(&inode->reservedField2, iNodeBuffer + 108, 4);
    memcpy(&inode->fragmentBlockAddress, iNodeBuffer + 112, 4);
    memcpy(inode->opSysValue2, iNodeBuffer + 116, 12);

    return inode;
}

static void fetchBlockFromInode(VDIFData* private_data, Inode *inode, int blockNum, uint8_t *blockBuf)
{
    Ext2* ext = private_data->fs;
    size_t ipb = ext->superBlock->blockSize / 4;

    if(blockNum < 12)
    {
        fetchBlock(private_data, blockBuf, inode->pointers[blockNum]);
        return;
    }

    blockNum -= 12;
    if(blockNum < ipb)
    {
        fetchSingle(private_data, inode, blockNum, blockBuf, ipb, 1);
        return;
    }
    blockNum -= ipb;
    if(blockNum < ipb*ipb)
    {
        fetchDouble(private_data, inode, blockNum, blockBuf, ipb, 1);
        return;
    }
    blockNum -= ipb*ipb;
    if(blockNum < ipb*ipb*ipb)
    {
        fetchTriple(private_data, inode, blockNum, blockBuf, ipb);
    }
}

static void fetchSingle(VDIFData* private_data, Inode* inode, int blockNum, uint8_t* blockBuf, size_t ipb, int start)
{
    uint8_t tempBuf[1024];
    if(start)
    {
        fetchBlock(private_data, tempBuf, inode->pointers[12]);
    }
    else
    {
        fetchBlock(private_data, tempBuf, blockNum);
    }

    blockNum %= ipb;
    uint32_t realBlock;
    uint32_t tempval = (blockNum/ipb)*sizeof(uint32_t);
    memcpy(&realBlock, tempBuf + tempval, 4);

    fetchBlock(private_data, blockBuf, realBlock);
}

static void fetchDouble(VDIFData* private_data, Inode* inode, int blockNum, uint8_t* blockBuf, size_t ipb, int start)
{
    uint8_t tempBuf[1024];
    if(start)
    {
        fetchBlock(private_data, tempBuf, inode->pointers[13]);
    }
    else
    {
        fetchBlock(private_data, tempBuf, blockNum);
    }

    blockNum %= ipb*ipb;
    uint32_t realBlock;
    memcpy(&realBlock, tempBuf + (blockNum/(ipb*ipb))*sizeof(uint32_t), 4);
    fetchSingle(private_data, inode, realBlock, blockBuf, ipb, 0);
}

static void fetchTriple(VDIFData* private_data, Inode* inode, int blockNum, uint8_t* blockBuf, size_t ipb)
{
    uint8_t tempBuf[1024];
    fetchBlock(private_data, tempBuf, inode->pointers[14]);

    uint32_t realBlock;
    memcpy(&realBlock, tempBuf + (blockNum/(ipb*ipb*ipb))*sizeof(uint32_t), 4);
    fetchDouble(private_data, inode, realBlock, blockBuf, ipb, 0);
}

static Directory* openDirectory(VDIFData* private_data, uint32_t inodeNumber)
{
    Ext2* ext = private_data->fs;
    Inode* inode = fetchInode(private_data, inodeNumber);
    // Inode is not directory
    if((inode->typePermissions & 0xF000u) != 0x4000u) return NULL;
    Directory *directory = (Directory *) malloc(sizeof(Directory));
    directory->contents = (uint8_t *) malloc(inode->lower32BitsSize);

    directory->inodeNumber = inodeNumber;
    directory->inode = inode;

    rewindDirectory(directory, REWIND_NO_DOTS);

    for (size_t i = 0; i < inode->lower32BitsSize / ext->superBlock->blockSize; i++)
    {
        fetchBlockFromInode(private_data, inode, i, directory->contents + i*ext->superBlock->blockSize);
    }


    /*uint32_t inodeNum = 0;
    uint32_t entrySize = 0;
    uint32_t nameLength = 0;
    uint32_t type = 0;
    char* name;

    memcpy(&inodeNum, directory->contents + directory->cursor, 4);

    if(inodeNum == 0) return 0;

    memcpy(&entrySize, directory->contents + directory->cursor + 4, 2);
    memcpy(&nameLength, directory->contents + directory->cursor + 6, 1);
    memcpy(&type, directory->contents + directory->cursor + 7, 1);

    name = (char*)malloc(nameLength+1);
    memcpy(name, directory->contents + directory->cursor + 8, nameLength);
    name[nameLength] = 0;

    directory->cursor += entrySize;
    directory->type = type;
    directory->name = name;
    directory->inodeNumber = inodeNum;*/

    return directory;
}

static uint32_t getNextEntry(Directory* dir)
{
    if(dir->cursor >= dir->inode->lower32BitsSize) return 0;
    uint32_t inode = 0;
    uint32_t entrySize = 0;
    uint32_t nameLength = 0;
    uint32_t type = 0;
    char* name;

    memcpy(&inode, dir->contents + dir->cursor, 4);

    if(inode == 0) return 0;

    memcpy(&entrySize, dir->contents + dir->cursor + 4, 2);
    memcpy(&nameLength, dir->contents + dir->cursor + 6, 1);
    memcpy(&type, dir->contents + dir->cursor + 7, 1);

    name = (char*)malloc(nameLength+1);
    memcpy(name, dir->contents + dir->cursor + 8, nameLength);
    name[nameLength] = 0;

    dir->cursor += entrySize;

    dir->type = type;
    dir->name = name;
    dir->inodeNumber = inode;

    return 1;

}

static void rewindDirectory(Directory* dir, uint32_t location)
{
    dir->cursor = location;
}

static void closeDirectory(Directory* dir)
{
    free(dir->inode);
    free(dir->name);
    free(dir->contents);
    free(dir);
}