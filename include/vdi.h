//
// Created by Kenny on 2/16/2020.
//

#ifndef VDIFUSE_VDI_H
#define VDIFUSE_VDI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>

#include "vdi_structs.h"
#include "utils.h"

#define VDI_SET 0
#define VDI_CUR 1
#define VDI_END 2

VDIFile* vdiOpen(char* filename);
void vdiClose(VDIFile* vdi);
void vdiSeek(VDIFile* vdi, long long offset, int anchor);
void vdiRead(VDIFile* vdi, uint8_t* buffer, size_t nbytes);

/*void vdiWrite(VDIFile* vdi, uint8_t* buffer, size_t nbytes);
void writeBlock(VDIFile* vdi, uint8_t* buffer, uint32_t blockNumber);*/

#endif //VDIFUSE_VDI_H
