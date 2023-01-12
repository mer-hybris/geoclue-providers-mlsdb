#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DATA_SIZE 8 // 64 bits / 8 bits to the byte

// This program isn't part of the geoclue-mlsdb -suite per se. It's only
// for testing that the files produced by geoclue-mlsdb-tool can be properly
// read. If you want to use the "testall.sh" -script you need to compile this
// file to a binary named "reader".

void print_bin(uint64_t n) {
    if (n > 1) {
        print_bin(n >> 1);
    }
    printf("%lu", n & 1);
}

int main(int argc, char **argv) {
    if (argc != 6) {
        printf("Usage: reader [mcc] [net] [area] [cell] [radio]\n");
        return 1;
    }
    uint16_t mcc = atoi(argv[1]);
    uint64_t net = atoi(argv[2]);
    uint64_t area = atoi(argv[3]);
    uint64_t cell = atoi(argv[4]);
    net <<= 46;
    area <<= 30;
    cell <<= 2;
    uint64_t target = net | area | cell;
    switch (argv[5][0]) {
    case 'G':
        case 'g':
        break;
    case 'L':
    case 'l':
        ++target;
        break;
    case 'U':
    case 'u':
        target += 2;
        break;
    default:
        target += 3;
        break;
    }
    uint64_t network = 0;
    size_t fs, pos, half;
    size_t old_pos = 1;
    size_t old_pos2 = 1;
    float lon, lat;
    char nname[16];
    if (snprintf(nname, 16, "./%s.dat", argv[1]) < 0) {
        fprintf(stderr, "ERROR: Invalid mcc %s\n", argv[1]);
        return 1;
    }
    FILE *fp;
    fp = fopen(nname, "r");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open infile %s.\n", &nname);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    fs = ftell(fp);
    if (fs % (2 * DATA_SIZE) != 0) {
        fprintf(stderr, "File size is not a multiple of data size. Corrupt file?\n");
        return 1;
    }
    pos = fs / 4; // Halfway through the network data (== first half of file).
    if (pos % DATA_SIZE != 0) {
        pos -= DATA_SIZE / 2;
    }
    half = pos;
    while (target != network) {
        if (old_pos == pos || old_pos2 == pos) {
            fprintf(stderr, "Could not find exact record. Target is %lu, closest match is %lu\n", target, network);
            return 1;
        }
        old_pos2 = old_pos;
        old_pos = pos;
        fseek(fp, pos, SEEK_SET);
        fread(&network, DATA_SIZE, 1, fp);
        half /= 2;
        if (half % DATA_SIZE != 0) {
            half -= DATA_SIZE / 2;
            if (half == 0) {
                half = DATA_SIZE;
            }
        }
        if (target > network) {
            pos += half;
        }
        else if (target < network){
            pos -= half;
        }
    }
    fseek(fp, pos + (fs / 2), SEEK_SET); // Jump to location data (== 2nd half)
    fread(&lat, DATA_SIZE / 2, 1, fp);
    fread(&lon, DATA_SIZE / 2, 1, fp);
    printf("long: %f lat: %f\n", lon, lat);
    fclose(fp);
    return 0;
}
