//
// Created by Kenny on 2/16/2020.
//

#include "../include/vdi.h"

VDIFile* vdiOpen(char* filename){
    VDIFile * vdi = (VDIFile*)malloc(sizeof (VDIFile));
    vdi->header = (Header*)malloc(sizeof (Header));
    vdi->header->diskGeometry = (DiskGeometry*)malloc( sizeof(DiskGeometry));
    vdi->superBlock = (SuperBlock*)malloc(sizeof(SuperBlock));
    vdi->blockGroupDescriptorTable = NULL;

    printf("Opening VDI File %s...\n",filename);
    vdi->f = fopen(filename, "rb+");
    if (vdi->f == NULL)
    {
        printf("Could not open VDI File %s!\n", filename);
        vdiClose(vdi);
        return NULL;
    }

    fread(vdi->header->preHeader, 1, 72, vdi->f);
    fread(&vdi->header->headerSize, 4, 1, vdi->f);
    fread(&vdi->header->imageType, 4, 1, vdi->f);
    fread(&vdi->header->imageFlags, 4, 1, vdi->f);
    fread(vdi->header->imageDescription, 1, 32, vdi->f);
    fseek(vdi->f, 0x154, SEEK_SET);
    fread(&vdi->header->offsetPages, 4, 1, vdi->f);
    fread(&vdi->header->offsetData, 4, 1, vdi->f);
    fread(&vdi->header->diskGeometry->cylinders, 4, 1, vdi->f);
    fread(&vdi->header->diskGeometry->heads, 4, 1, vdi->f);
    fread(&vdi->header->diskGeometry->sectors, 4, 1, vdi->f);
    fread(&vdi->header->diskGeometry->sectorSize, 4, 1, vdi->f);
    fread(&vdi->header->unused, 4, 1, vdi->f);
    fread(&vdi->header->diskSize, 8, 1, vdi->f);
    fread(&vdi->header->pageSize, 4, 1, vdi->f);
    fread(&vdi->header->pageExtraData, 4, 1, vdi->f);
    fread(&vdi->header->pagesInHDD, 4, 1, vdi->f);
    fread(&vdi->header->pagesAllocated, 4, 1, vdi->f);
    fread(vdi->header->UUID, 1, 16, vdi->f);
    fread(vdi->header->UUIDLastSnap, 1, 16, vdi->f);
    fread(vdi->header->UUIDLink, 1, 16, vdi->f);
    fread(vdi->header->UUIDParent, 1, 16, vdi->f);
    fread(vdi->header->notNeeded, 1, 56, vdi->f);

    fseek(vdi->f, vdi->header->offsetData + 454, SEEK_SET);
    uint32_t volumeStart;
    fread(&volumeStart, 4, 1, vdi->f);
    volumeStart = volumeStart*512;
    vdi->header->offsetData += volumeStart;

    return vdi;
}

void vdiSeek(VDIFile* vdi, long long offset, int anchor)
{
    if(anchor == VDI_SET)
    {
        vdi->cursor = vdi->header->offsetData + offset;
    }
    if(anchor == VDI_CUR)
    {
        vdi->cursor += offset;
    }
    if(anchor == VDI_END)
    {
        // cursor should be negative
        vdi->cursor = offset + vdi->header->diskSize;
    }
}

// call vdiSeek before vdiRead at all times
void vdiRead(VDIFile* vdi, uint8_t* buffer, size_t nbytes)
{
    long long page = vdi->cursor/vdi->header->pageSize;
    long long offset = vdi->cursor%vdi->header->pageSize;

    long long position = page*vdi->header->pageSize+offset;
    fseek(vdi->f, position, SEEK_SET);
    fread(buffer, 1, nbytes, vdi->f);
}

void vdiClose(VDIFile* vdi)
{
    if(vdi->blockGroupDescriptorTable != NULL)
    {
        for (size_t i = 0; i < vdi->superBlock->numBlockGroups; i++)
        {
            free(vdi->blockGroupDescriptorTable[i]);
        }
        free(vdi->blockGroupDescriptorTable);
    }

    free(vdi->blockGroupDescriptorFullContents);
    free(vdi->header->diskGeometry);
    free(vdi->header);
    free(vdi->superBlock);
    fclose(vdi->f);
    free(vdi);
}

void fetchBlock(VDIFile* vdi, uint8_t* buffer, uint32_t blockNumber)
{
    vdiSeek(vdi,  blockNumber*vdi->superBlock->blockSize, VDI_SET);
    vdiRead(vdi, buffer, vdi->superBlock->blockSize);
}

void vdiWrite(VDIFile* vdi, uint8_t* buffer, size_t nbytes)
{
    long long page = vdi->cursor/vdi->header->pageSize;
    long long offset = vdi->cursor%vdi->header->pageSize;

    long long position = page*vdi->header->pageSize+offset;
    fseek(vdi->f, position, SEEK_SET);
    fwrite(buffer, 1, nbytes, vdi->f);
}

void writeBlock(VDIFile* vdi, uint8_t* buffer, uint32_t blockNumber)
{
    vdiSeek(vdi,  blockNumber*vdi->superBlock->blockSize, VDI_SET);
    vdiWrite(vdi, buffer, vdi->superBlock->blockSize);
}



