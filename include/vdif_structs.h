//
// Created by Kenny on 2/16/2020.
//

#ifndef VDIFUSE_VDIF_STRUCTS_H
#define VDIFUSE_VDIF_STRUCTS_H

#include "fuse.h"
#include "vdi_structs.h"

typedef struct VDIFData VDIFData;

typedef struct FSOperations {
    void (*init) (VDIFData* private_data);
    void (*destroy) (VDIFData* private_data);
    void (*read_root) (VDIFData* private_data, void *buf, fuse_fill_dir_t filler);
} FSOperations;

struct VDIFData {
    VDIFile* vdi;
    char* vdiFilePath;
    char* vdiFs;

    void* fs;
    FSOperations fsops;
};


#endif //VDIFUSE_VDIF_STRUCTS_H
