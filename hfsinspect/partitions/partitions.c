//
//  partitions.c
//  hfsinspect
//
//  Created by Adam Knight on 11/7/13.
//  Copyright (c) 2013 Adam Knight. All rights reserved.
//

#include "partitions.h"
#include "range.h"
#include "hfs_pstruct.h"

bool sniff_and_print(HFSVolume* hfs)
{
    if (gpt_test(hfs->vol)) {
        gpt_load(hfs->vol);
        gpt_dump(hfs->vol);
        
    } else if (mbr_test(hfs->vol)) {
        mbr_load(hfs->vol);
        mbr_dump(hfs->vol);
        
    } else if (cs_sniff(hfs)) {
        cs_print(hfs);
        
    } else if (apm_sniff(hfs)) {
        apm_print(hfs);
        
    } else {
        warning("Unknown disk or partition type.");
        return false;
    }
    
    PrintHeaderString("Parsed Volume");
    vol_dump(hfs->vol);
    return true;
}

int sniff_partitions(const HFSVolume* hfs, range_ptr partitions, int* count)
{
    // Check disk type
    // Call format-specific method to get partition list
    // Extract ranges
    // Set reference params for the ranges array and count
    // Return the number of partitions found.
    return 0;
}
