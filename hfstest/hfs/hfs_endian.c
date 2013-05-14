//
//  hfs_endian.c
//  hfstest
//
//  Created by Adam Knight on 5/5/13.
//  Copyright (c) 2013 Adam Knight. All rights reserved.
//

#include <stdlib.h>
#include <hfs/hfs_format.h>
#include <string.h>
#include "hfs_endian.h"
#include "hfs_structs.h"


#define Convert16(x)    x = OSSwapBigToHostInt16(x)
#define Convert32(x)    x = OSSwapBigToHostInt32(x)
#define Convert64(x)    x = OSSwapBigToHostInt64(x)


void swap_HFSPlusVolumeHeader(HFSPlusVolumeHeader *record)
{
    Convert16(record->signature);
    Convert16(record->version);
    Convert32(record->attributes);
    Convert32(record->lastMountedVersion);
    Convert32(record->journalInfoBlock);
    
    Convert32(record->createDate);
    Convert32(record->modifyDate);
    Convert32(record->backupDate);
    Convert32(record->checkedDate);
    
    Convert32(record->fileCount);
    Convert32(record->folderCount);
    
    Convert32(record->blockSize);
    Convert32(record->totalBlocks);
    Convert32(record->freeBlocks);
    
    Convert32(record->nextAllocation);
    Convert32(record->rsrcClumpSize);
    Convert32(record->dataClumpSize);
    Convert32(record->nextCatalogID);
    
    Convert32(record->writeCount);
    Convert64(record->encodingsBitmap);
    
    //    record->finderInfo is an array of bytes; swap as-used.
    
    swap_HFSPlusForkData(&record->allocationFile);
    swap_HFSPlusForkData(&record->extentsFile);
    swap_HFSPlusForkData(&record->catalogFile);
    swap_HFSPlusForkData(&record->attributesFile);
    swap_HFSPlusForkData(&record->startupFile);
}

void swap_HFSPlusForkData(HFSPlusForkData *record)
{
    Convert64(record->logicalSize);
    Convert32(record->totalBlocks);
    Convert32(record->clumpSize);
    
    for (int i = 0; i < kHFSPlusExtentDensity; i++) {
        HFSPlusExtentDescriptor *extent_descriptor = &record->extents[i];
        swap_HFSPlusExtentDescriptor(extent_descriptor);
    }
}

void swap_HFSPlusExtentDescriptor(HFSPlusExtentDescriptor *record)
{
    Convert32(record->startBlock);
    Convert32(record->blockCount);
}

void swap_BTNodeDescriptor(BTNodeDescriptor *record)
{
    Convert32(record->fLink);
    Convert32(record->bLink);
    //    record->kind is a short
    //    record->height is a short
    Convert16(record->numRecords);
    //    record->reserved is reserved
}

void swap_BTHeaderRec(BTHeaderRec *record)
{
    Convert16(record->treeDepth);
    Convert32(record->rootNode);
    Convert32(record->leafRecords);
    Convert32(record->firstLeafNode);
    Convert32(record->lastLeafNode);
    Convert16(record->nodeSize);
    Convert16(record->maxKeyLength);
    Convert32(record->totalNodes);
    Convert32(record->freeNodes);
    //    record->reserved1
    Convert32(record->clumpSize);
    //    record->btreeType is a short
    //    record->keyCompareType is a short
    Convert32(record->attributes);
    //    header->reserved3
}

void swap_BTreeKey(BTreeKey *record)
{
    // length8 is a short
    Convert16(record->length16);
    // rawData is opaque
}

void swap_HFSPlusCatalogKey(HFSPlusCatalogKey *record)
{
    Convert16(record->keyLength);
    Convert32(record->parentID);
    swap_HFSUniStr255(&record->nodeName);
}

void swap_HFSUniStr255(HFSUniStr255 *unistr)
{
    Convert16(unistr->length);
    //    for (int i = 0; i < unistr->length; i++) {
    //        Convert16(unistr->unicode[i]);
    //    }
}

void swap_HFSPlusCatalogFolder(HFSPlusCatalogFolder *record)
{
    Convert16(record->recordType);
    Convert16(record->flags);
    Convert32(record->valence);
    Convert32(record->folderID);
    Convert32(record->createDate);
    Convert32(record->contentModDate);
    Convert32(record->attributeModDate);
    Convert32(record->accessDate);
    Convert32(record->backupDate);
    swap_HFSPlusBSDInfo(&record->bsdInfo);
    swap_FndrDirInfo(&record->userInfo);
    swap_FndrOpaqueInfo(&record->finderInfo);
    Convert32(record->textEncoding);
    Convert32(record->folderCount);
}

void swap_HFSPlusBSDInfo(HFSPlusBSDInfo *record)
{
    Convert32(record->ownerID);
    Convert32(record->groupID);
    // adminFlags is a short
    // ownerFlags is a short
    Convert16(record->fileMode);
    Convert32(record->special.iNodeNum);
}

void swap_FndrDirInfo(FndrDirInfo *record)
{
    Convert16(record->frRect.top);
    Convert16(record->frRect.left);
    Convert16(record->frRect.bottom);
    Convert16(record->frRect.right);
    // frFlags is a short
    Convert16(record->frLocation.v);
    Convert16(record->frLocation.h);
    Convert16(record->opaque);
}

void swap_FndrFileInfo(FndrFileInfo *record)
{
    Convert32(record->fdType);
    Convert32(record->fdCreator);
    Convert16(record->fdFlags);
    Convert16(record->fdLocation.v);
    Convert16(record->fdLocation.h);
    Convert16(record->opaque);
}

void swap_FndrOpaqueInfo(FndrOpaqueInfo *record)
{
    // A bunch of undocumented shorts.  Included for completeness.
    ;;
}

void swap_HFSPlusCatalogFile(HFSPlusCatalogFile *record)
{
    Convert16(record->recordType);
    Convert16(record->flags);
    Convert32(record->reserved1);
    Convert32(record->fileID);
    Convert32(record->createDate);
    Convert32(record->contentModDate);
    Convert32(record->attributeModDate);
    Convert32(record->accessDate);
    Convert32(record->backupDate);
    swap_HFSPlusBSDInfo(&record->bsdInfo);
    swap_FndrFileInfo(&record->userInfo);
    swap_FndrOpaqueInfo(&record->finderInfo);
    Convert32(record->textEncoding);
    Convert32(record->reserved2);
    
    swap_HFSPlusForkData(&record->dataFork);
    swap_HFSPlusForkData(&record->resourceFork);
}

void swap_HFSPlusCatalogThread(HFSPlusCatalogThread *record)
{
    Convert16(record->recordType);
    Convert32(record->reserved);
    Convert32(record->parentID);
    // nodeName is an array of unichars.  Ain't touchin' that.
}

void swap_BTreeNode(BTreeNode *node)
{
    // node->buffer is a 4-8KB block read from disk in BE format.
    // Figure out what it is and swap everything that needs swapping.
    // Good luck.
    
    // Check record offset 0 to see if we've done this before. It's a constant 14.
    u_int16_t sentinel = (u_int16_t)(node->buffer.data + node->blockSize - sizeof(u_int16_t));
    if ( sentinel == 14 ) return;
    
    
    /* First, swap things universal to all nodes. */
    
    // Swap node descriptor
    BTNodeDescriptor *nodeDescriptor = (BTNodeDescriptor*)(node->buffer.data);
    swap_BTNodeDescriptor(nodeDescriptor);
    node->nodeDescriptor = *nodeDescriptor;
    
    // Swap record offsets
    u_int16_t *offsets = (u_int16_t*)(node->buffer.data + node->blockSize - (sizeof(u_int16_t) * node->nodeDescriptor.numRecords));
    
    for (int i = 0; i < node->nodeDescriptor.numRecords; i++) {
        u_int16_t *offset = &offsets[i];
        *offset = S16(*offset);
    }
    
    // Note for branching
    u_int32_t treeID = node->bTree.fork.cnid;
    
    if (treeID == kHFSCatalogFileID) {
        // Swap catalog records
        
        if (node->nodeDescriptor.kind == kBTHeaderNode) {
            // Only swap the header.  Don't care about user and map.
            BTHeaderRec *header = (BTHeaderRec*)(node->buffer.data + sizeof(BTNodeDescriptor));
            swap_BTHeaderRec(header);
            
        } else if (node->nodeDescriptor.kind == kBTIndexNode || node->nodeDescriptor.kind == kBTLeafNode) {
            for (int i = 1; i < node->nodeDescriptor.numRecords; i++) {
                char* record;
                record = (node->buffer.data + offsets[i]);
                
                BTreeKey *key = (BTreeKey*)record;
                swap_BTreeKey(key);
                
                u_int16_t key_length = key->length16;
                if (key_length % 2) key_length++;
                
                record = (node->buffer.data + offsets[i] + key_length);
                
                if (node->nodeDescriptor.kind == kBTIndexNode) {
                } else if (node->nodeDescriptor.kind == kBTLeafNode) {
                    int16_t recordKind = *(int16_t*)record;
                    recordKind = S16(recordKind);
                    
                    if (recordKind == kHFSPlusFolderRecord) {
                        swap_HFSPlusCatalogFolder((HFSPlusCatalogFolder*)record);
                        
                    } else if (recordKind == kHFSPlusFileRecord) {
                        swap_HFSPlusCatalogFile((HFSPlusCatalogFile*)record);
                        
                    } else if (recordKind == kHFSPlusFolderThreadRecord) {
                        swap_HFSPlusCatalogThread((HFSPlusCatalogThread*)record);
                        
                    } else if (recordKind == kHFSPlusFileThreadRecord) {
                        swap_HFSPlusCatalogThread((HFSPlusCatalogThread*)record);
                        
                    }
                    
                }
            }
            
        } else if (node->nodeDescriptor.kind == kBTMapNode) {
            // Don't care.  Thanks for playing.
            
        }
        
    } else if (treeID == kHFSExtentsFileID) {
        // Swap extents records
        
    } else if (treeID == kHFSAttributesFileID) {
        // Swap attributes records
        
    } else {
        // Error? Ignore and watch the fun?
        
    }
    
}