/*
gcc VDIFuse.c `pkg-config fuse --cflags --libs` -o VDIFuse

~/Desktop/VDIFuse/a.out -d -s -f ~/Desktop/VDIMount
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include "include/vdi.h"
#include "include/vdif_structs.h"

#define VDIF_DATA ((VDIFData *) fuse_get_context()->private_data)
#define ALLOCATE(type) (type*)malloc(sizeof(type))

static const char *filepath = "/file";
static const char *filename = "file";
static const char *filecontent = "I'm the content of the only file available there\n";

static int vdif_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    if (strcmp(path, filepath) == 0) {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(filecontent);
        return 0;
    }

    return -ENOENT;
}

static int vdif_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

     (buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    filler(buf, filename, NULL, 0);

    return 0;
}

static int vdif_open(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int vdif_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {

    if (strcmp(path, filepath) == 0) {
        size_t len = strlen(filecontent);
        if (offset >= len) {
            return 0;
        }

        if (offset + size > len) {
            memcpy(buf, filecontent + offset, len - offset);
            return len - offset;
        }

        memcpy(buf, filecontent + offset, size);
        return size;
    }

    return -ENOENT;
}

// Note from Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>  :
// Undocumented but extraordinarily useful fact:  the fuse_context is
// set up before this function is called, and
// fuse_get_context()->private_data returns the user_data passed to
// fuse_main().  Really seems like either it should be a third
// parameter coming in here, or else the fact should be documented
// (and this might as well return void, as it did in older versions of
// FUSE).
static void* vdif_init(struct fuse_conn_info *conn) {
    printf("Initializing vdif...\n");
    printf("Initialization complete.\n");
    // vdiOpen done in main because for some reason fuse will seg fault if it's done here
    return VDIF_DATA;
}

static void vdif_destroy(void* private_data) {
    printf("Destroying vdif...\n");

    // Cleanup private data
    VDIFData* data = (VDIFData*)private_data;
    vdiClose(data->vdiFile);
    free(data);
    printf("Destruction Complete.\n");
}

static struct fuse_operations fuse_example_operations = {
        .getattr = vdif_getattr,
        .open = vdif_open,
        .read = vdif_read,
        .readdir = vdif_readdir,
        .init = vdif_init,
        .destroy = vdif_destroy
};

int main(int argc, char *argv[])
{
    printf("Welcome to VDI Fuse!\n");
    // Cleanup is done in vdif_destroy
    VDIFData* vdif_data = ALLOCATE(VDIFData);
    vdif_data->vdiFilePath = argv[argc-1];
    vdif_data->vdiFile = vdiOpen(vdif_data->vdiFilePath);
    return fuse_main(argc-1, argv, &fuse_example_operations, vdif_data);
}