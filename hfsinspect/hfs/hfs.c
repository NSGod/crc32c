//
//  hfs.c
//  hfsinspect
//
//  Created by Adam Knight on 5/5/13.
//  Copyright (c) 2013 Adam Knight. All rights reserved.
//

#include <sys/disk.h>
#include <fcntl.h>
#include "hfs.h"

#pragma mark Volume Abstractions

int hfs_load_mbd(Volume *vol, HFSMasterDirectoryBlock *mdb)
{
    if ( vol_read(vol, mdb, sizeof(HFSMasterDirectoryBlock), 1024) < 0)
        return -1;
    
    swap_HFSMasterDirectoryBlock(mdb);
    
    return 0;
}

int hfs_load_header(Volume *vol, HFSPlusVolumeHeader *vh)
{
    if ( vol_read(vol, vh, sizeof(HFSPlusVolumeHeader), 1024) < 0)
        return -1;
    
    swap_HFSPlusVolumeHeader(vh);
    
    return 0;
}

int hfs_attach(HFSVolume* hfs, Volume *vol)
{
    if (hfs == NULL || vol == NULL) { errno = EINVAL; return -1; }
    
    int type;
    int result;
    
    // Test to see if we support the volume.
    result = hfs_test(vol);
    if (result < 0) return -1;
    
    type = (unsigned)result;
    
    if (type == kVolumeSubtypeUnknown || type == kFilesystemTypeHFS) {
        errno = EINVAL;
        return -1;
    }
    
    // Clear the HFSVolume struct (hope you didn't need that)
    ZERO_STRUCT(*hfs);
    
    // Handle wrapped volumes.
    if (type == kFilesystemTypeWrappedHFSPlus) {
        HFSMasterDirectoryBlock mdb; ZERO_STRUCT(mdb);
        if ( hfs_load_mbd(vol, &mdb) < 0) return -1;
        
        hfs->offset = (mdb.drAlBlSt * 512) + (mdb.drEmbedExtent.startBlock * mdb.drAlBlkSiz);
    }
    
    // Load the volume header.
    if ( hfs_load_header(vol, &hfs->vh) < 0 )
        return -1;
    
    // Update the HFSVolume struct.
    hfs->vol = vol;
    hfs->block_size = hfs->vh.blockSize;
    hfs->block_count = hfs->vh.totalBlocks;
    
    hfs->offset += vol->offset;
    hfs->length = (vol->length ? vol->length : hfs->block_size * hfs->block_count);
    
    return 0;
}

/**
 Tests to see if a volume is HFS or not.
 @return Returns -1 on failure or a VolumeSubtype constant representing the detected filesystem.
 */
int hfs_test(Volume *vol)
{
    // First, test for HFS or wrapped HFS+ volumes.
    HFSMasterDirectoryBlock mdb;
    
    if ( hfs_load_mbd(vol, &mdb) < 0)
        return -1;
    
    if (mdb.drSigWord == kHFSSigWord && mdb.drEmbedSigWord == kHFSPlusSigWord) {
        info("Found a wrapped HFS+ volume");
        return kFilesystemTypeWrappedHFSPlus;
    } else if (mdb.drSigWord == kHFSSigWord) {
        info("Found an HFS volume");
        return kFilesystemTypeHFS;
    }
    
    // Now test for a modern HFS+ volume.
    HFSPlusVolumeHeader vh;
    
    if ( hfs_load_header(vol, &vh) < 0 )
        return -1;
    
    if (vh.signature == kHFSPlusSigWord || vh.signature == kHFSXSigWord) {
        info("Found an HFS+ volume");
        return kFilesystemTypeHFSPlus;
    }
    
    info("Unknown volume type");
    return kVolumeSubtypeUnknown;
}

int hfs_load(HFSVolume *hfs) {
    debug("Loading volume header for descriptor %u", hfs->vol->fd);

    bool success;
    
    HFSMasterDirectoryBlock mdb;
    HFSMasterDirectoryBlock* vcb = &mdb;
    
    success = hfs_get_HFSMasterDirectoryBlock(&mdb, hfs);
    if (!success) critical("Could not read volume header!");

    if (vcb->drSigWord == kHFSSigWord) {
        PrintHFSMasterDirectoryBlock(vcb);
        if (vcb->drEmbedSigWord == kHFSPlusSigWord) {
            hfs->offset += (vcb->drAlBlSt * 512) + (vcb->drEmbedExtent.startBlock * vcb->drAlBlkSiz);
            debug("Found a wrapped volume at offset %llu", hfs->offset);
            
        } else {
            error("This tool does not currently support standalone HFS Standard volumes (%#04x).", vcb->drEmbedSigWord);
            errno = EFTYPE;
            return -1;
        }
    }
    
    success = hfs_get_HFSPlusVolumeHeader(&hfs->vh, hfs);
    if (!success) critical("Could not read volume header!");
    
    if (hfs->vh.signature != kHFSPlusSigWord && hfs->vh.signature != kHFSXSigWord) {
        debug("Not HFS+ or HFSX. Detecting format...");
        if (! sniff_and_print(hfs)) {
            char* buffer; INIT_BUFFER(buffer, 1024);
            hfs_read_raw(buffer, hfs, 1024, 0);
            error("not an HFS+ or HFSX volume signature: 0x%x", hfs->vh.signature);
            VisualizeData(buffer, 1024);
            errno = EFTYPE;
            free(buffer);
        }
        errno = 0;
        return -1;
    }
    
    hfs->block_size     = hfs->vh.blockSize;
    hfs->block_count    = hfs->vh.totalBlocks;
    hfs->length         = hfs->block_count * hfs->block_size;
    
    return 0;
}

int hfs_close(HFSVolume *hfs) {
    debug("Closing volume.");
    int result = vol_close(hfs->vol);
    return result;
}

#pragma mark Volume Structures

bool hfs_get_HFSMasterDirectoryBlock(HFSMasterDirectoryBlock* vh, const HFSVolume* hfs)
{
    if (hfs->vol) {
        char* buffer;
        INIT_BUFFER(buffer, 2048)
        
        ssize_t size = hfs_read_raw(buffer, hfs, 2048, 0);
        
        if (size < 1) {
            perror("read");
            critical("Cannot read volume.");
            FREE_BUFFER(buffer);
            return -1;
        }
        
        *vh = *(HFSMasterDirectoryBlock*)(buffer+1024);
        FREE_BUFFER(buffer);
        
        swap_HFSMasterDirectoryBlock(vh);
        
        return true;
    }
    
    return false;
}

bool hfs_get_HFSPlusVolumeHeader(HFSPlusVolumeHeader* vh, const HFSVolume* hfs)
{
    if (hfs->vol) {
        char* buffer;
        INIT_BUFFER(buffer, 2048)
        
        ssize_t size = hfs_read_raw(buffer, hfs, 2048, 0);
        
        if (size < 1) {
            perror("read");
            critical("Cannot read volume.");
            FREE_BUFFER(buffer);
            return -1;
        }
        
        *vh = *(HFSPlusVolumeHeader*)(buffer+1024);
        FREE_BUFFER(buffer);
        
        swap_HFSPlusVolumeHeader(vh);
        
        return true;
    }
    
    return false;
}

bool hfs_get_JournalInfoBlock(JournalInfoBlock* block, const HFSVolume* hfs)
{
    if (hfs->vh.journalInfoBlock) {
        char* buffer;
        INIT_BUFFER(buffer, hfs->block_size);
        
        ssize_t read = hfs_read_blocks(buffer, hfs, 1, hfs->vh.journalInfoBlock);
        if (read < 0) {
            perror("read");
            critical("Read error when fetching journal info block");
        } else if (read < 1) {
            critical("Didn't read the whole journal info block!");
        }
        *block = *(JournalInfoBlock*)buffer; // copies
        FREE_BUFFER(buffer);
        
        swap_JournalInfoBlock(block);
        return true;
    }
    
    return false;
}
