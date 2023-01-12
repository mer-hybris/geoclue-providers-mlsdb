/*
  Copyright (C) 2022 Jolla Ltd.
  Contact: Daniel Suni <daniel.suni@jolla.com>

  This file is part of geoclue-mlsdb.

  Geoclue-mlsdb is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#define NEWLINE 10
#define STDIN 0

/*
This parser will read MLS full export data in CSV format. Data files can
be downloaded here: https://location.services.mozilla.com/downloads

The reader assumes the data is numerically sorted by unique cell id and that
only data lines (i.e. no headers) are passed. To achieve that, run:
tail -n +2 [CSV file] | sort -t, -k2n,2 -k3n,3 -k4n,4 -k5n,5 -k1,1 | geoclue-mlsdb-tool
or (preferrably) use the wrapper script.

---

The "network" is allocated as follows:
0000000000000000000000000000000000000000000000000000000000000000
        \____ ___/\_______ ______/\_____________ ____________/\/
             v            v                     v              v
          NET: 10b Area: 16 bits        Cell ID: 28 bits     Radio: 2b 

The "position" is allocated as 2 concatenated 32-bit floats
with longitude first and then latitude.

This program will produce 2 files per mcc: .ntw for network data and .loc
for location data. To finalize the mlsdb data file the two should simply
be concatenated into a .dat file. (The wrapper will do this too.)

Since the network portion and the data portion are exactly the same size,
the file is simply split so the the 1st half contains network data, and the
corresponding location data is found in ${net_data_pos} + ${file_size} / 2
*/
uint64_t network;
uint64_t position;
uint64_t previous = 0;

// Helper function for debugging (prints out a 64b int as binary)
void print_bin(uint64_t n)
{
    if (n > 1) {
        print_bin(n >> 1);
    }
    printf("%lu", n & 1);
}

void add_network(char *str, size_t shift)
{
    uint64_t t = atoi(str);
    t <<= shift;
    network |= t;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
void add_position(char *str, size_t shift)
{
    float f = atof(str);
    // Pretend the 32b float is a 32b int, and assign the value to a 64b int.
    uint64_t t = *(uint32_t*)&f;
    t <<= shift;
    position |= t;
}
#pragma GCC diagnostic pop

void add_radio(char *str)
{
    switch(str[0]) {
    case 'G': // GSM
        break;
    case 'L': // LTE
        ++network;
        break;
    case 'U': // UMTS
        network += 2;
        break;
    default:
        network += 3;
        break;
    }
}

int main()
{
    char c;
    char line[150];
    char mcc_fn[16];
    char loc_fn[16];
    size_t pos = 0, count = 0, mcc_old = 0, mcc_num = 0, mcc_p = 0, net_p = 0, area_p = 0, cell_p = 0, lon_p = 0, lat_p = 0;
    FILE *fp_mcc = NULL, *fp_loc = NULL;
    while (read(STDIN, &c, 1) > 0) {
        if (c == NEWLINE) {
            pos = 0;
            count = 0;
            position = 0;
            network = 0;
            mcc_num = atoi(&line[mcc_p]);
            // We have a new mcc; we need to open new files.
            if (mcc_num != mcc_old) {
                if (mcc_old > 0) {
                    fclose(fp_mcc);
                    fclose(fp_loc);
                }
                if (snprintf(mcc_fn, 16, "./%s.ntw", &line[mcc_p]) < 0 ||
                    snprintf(loc_fn, 16, "./%s.loc", &line[mcc_p]) < 0) {
                    fprintf(stderr, "ERROR: Encountered an invalid mcc %s\n", &line[mcc_p]);
                    return 1;
                }
                fp_mcc = fopen(mcc_fn, "w");
                fp_loc = fopen(loc_fn, "w");
                if (fp_mcc == NULL || fp_loc == NULL) {
                    fprintf(stderr, "Unable to open outfile for mcc %ld.\n", mcc_num);
                    return 1;
                }
                mcc_old = mcc_num;
                previous = 0;
            }
            add_network(&line[net_p], 46);
            add_network(&line[area_p], 30);
            add_network(&line[cell_p], 2);
            add_radio(line);
            if (previous > network) {
                fprintf(stderr, "ERROR: The current record has value %lu, which is smaller than the previous %lu\n", network, previous);
                fprintf(stderr, "The reader will not be able to properly search a file that isn't sorted correctly. Please check your sort.\n");
                return 1;
            }
            previous = network;
            add_position(&line[lon_p], 32);
            add_position(&line[lat_p], 0);
            fwrite(&network, sizeof(network), 1, fp_mcc);
            fwrite(&position, sizeof(position), 1, fp_loc);
        }
        else if (c == ',') {
            line[pos] = '\0'; // Null instead of comma for later string operations.
            ++count;
            ++pos;
            switch(count) {
            case 1:
                mcc_p = pos;
                break;
            case 2:
                net_p = pos;
                break;
            case 3:
                area_p = pos;
                break;
            case 4:
                cell_p = pos;
                break;
            case 6:
                lon_p = pos;
                break;
            case 7:
                lat_p = pos;
                break;
            default:
                break;
            }
        }
        else {
            line[pos] = c;
            ++pos;
        }
    }
    if (fp_mcc != NULL) {
        fclose(fp_mcc);
    }
    if (fp_loc != NULL) {
        fclose(fp_loc);
    }
    return 0;
}
