//
//  mbr.c
//  hfsinspect
//
//  Created by Adam Knight on 11/7/13.
//  Copyright (c) 2013 Adam Knight. All rights reserved.
//

#include "mbr.h"
#include "hfs_pstruct.h"
#include "hfs_endian.h"

int mbr_load_header(Volume *vol, MBR *mbr);


void mbr_print(HFSVolume* hfs, MBR* mbr)
{
    PrintHeaderString("Master Boot Record");
    PrintAttributeString("signature", "%#x", S16(*(u_int16_t*)mbr->signature));
    
    FOR_UNTIL(i, 4) {
        MBRPartition* partition = &mbr->partitions[i];
        
        PrintHeaderString("Partition %d", i + 1);
        PrintUIHex  (partition, status);
        PrintUI     (partition, first_sector.head);
        PrintUI     (partition, first_sector.cylinder);
        PrintUI     (partition, first_sector.sector);
        PrintUIHex  (partition, type);
        PrintUI     (partition, last_sector.head);
        PrintUI     (partition, last_sector.cylinder);
        PrintUI     (partition, last_sector.sector);
        PrintUI     (partition, first_sector_lba);
        PrintUI     (partition, sector_count);
        _PrintDataLength("(size)", partition->sector_count*hfs->block_size);
    }
}

int mbr_load_header(Volume *vol, MBR *mbr)
{
    if ( vol_read(vol, mbr, sizeof(MBR), 0) < 0 )
        return -1;
    
    return 0;
}

int mbr_test(Volume *vol)
{
    char mbr_sig[2] = { 0x55, 0xaa };
    
    MBR mbr = { .signature = {'\0', '\0'} };
    
    if ( mbr_load_header(vol, &mbr) < 0 )
        return -1;
    
    if (memcmp(&mbr_sig, &mbr.signature, 2) == 0) {
        vol->type = kVolumeTypePartitionMap;
        vol->subtype = kPartitionTypeMBR;
        return 1;
    } else {
        return 0;
    }
}

int mbr_load(Volume *vol)
{
    MBR mbr;
    bzero(&mbr, sizeof(MBR));
    
    if ( mbr_load_header(vol, &mbr) < 0)
        return -1;
    
    FOR_UNTIL(i, 4) {
        if (mbr.partitions[i].type) {
            Volume* v = NULL;
            MBRPartition p = empty_MBRPartition;
            PartitionHint hint = kHintIgnore;
            off_t offset = 0;
            size_t length = 0;
            
            p = mbr.partitions[i];
            offset = p.first_sector_lba * vol->block_size;
            length = p.sector_count * vol->block_size;
            
            v = vol_make_partition(vol, i, offset, length);
            
            const char* name = mbr_partition_type_str(p.type, &hint);
            if (name != NULL) {
                v->type = hint;
            }
        }
    }
    
    return 0;
}

const char* mbr_partition_type_str(uint16_t type, PartitionHint* hint)
{
    static char type_str[100];
    
    FOR_UNTIL(i, 256) {
        if (mbr_partition_types[i].type == type) {
            if (hint != NULL) *hint = mbr_partition_types[i].hints;
            strlcpy(type_str, mbr_partition_types[i].name, 99);
        }
    }
    
    if (type_str == NULL)
        strlcpy(type_str, "unknown", 100);
    
    return type_str;
}

int mbr_dump(Volume *vol)
{
    const char* type_str = NULL;
    MBR *mbr = NULL;
    INIT_BUFFER(mbr, sizeof(MBR));
    
    if ( mbr_load_header(vol, mbr) < 0)
        return -1;
    
    PrintHeaderString("Master Boot Record");
    PrintAttributeString("signature", "%#x", S16(*(u_int16_t*)mbr->signature));
    
    FOR_UNTIL(i, 4) {
        MBRPartition* partition = &mbr->partitions[i];
        if (partition->type == 0) continue;
        
        type_str = mbr_partition_type_str(partition->type, NULL);
        
        PrintHeaderString("Partition %d", i + 1);
        PrintUIHex  (partition, status);
        PrintUI     (partition, first_sector.head);
        PrintUI     (partition, first_sector.cylinder);
        PrintUI     (partition, first_sector.sector);
        PrintAttributeString("type", "0x%02X (%s)", partition->type, type_str);
        PrintUI     (partition, last_sector.head);
        PrintUI     (partition, last_sector.cylinder);
        PrintUI     (partition, last_sector.sector);
        PrintUI     (partition, first_sector_lba);
        PrintUI     (partition, sector_count);
        _PrintDataLength("(size)", (partition->sector_count * vol->block_size));
    }
    
    return 0;
}
