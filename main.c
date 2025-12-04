// Martin Ganen-Villa
// U49246681
// This program is meant to map the location of sectors in a RAID device to the corresponding sectors on the hard disks that compsoe it

// Include basic libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char level[3];      // RAID level (0, 01, 10, 4, or 5)
    int N, C, S;        // N = number of disks, C = stripes per group, S = total stripes
    char outputNaame[256];

    // Read input: RAID level, disk count, stripes per group, total stripes, and output filename
    if (scanf("%2s %d %d %d %255s", level, &N, &C, &S, outputNaame) != 5) {
        return 1;
    }

    // Open file
    FILE *fp = fopen(outputNaame, "w");
    if (!fp) {
        return 1;
    }

    // Write header row with disk labels
    fprintf(fp, "\t");
    for (int d = 0; d < N; ++d) {
        fprintf(fp, "Disk%d", d);
        if  (d != N-1) {
            fprintf(fp, "\t");
        }
    }
    fprintf(fp, "\n");

    long long current_sector = 0;  // Track the current sector number across all disks

    // RAID 0: Simple striping -> data is distributed across all disks
    if (strcmp(level, "0") == 0) {
        int stripes_per_group =  C;
        int groups = S / stripes_per_group;  // Number of stripe groups

        for (int g = 0; g < groups; ++g) {
            long long block[16][16];  // 2D array to store sector mappings for this group

            // Fill the block by incrementing sectors across all disks
            for (int d = 0; d < N; ++d) {
                for (int r = 0; r < stripes_per_group; ++r) {
                    block[r][d] = current_sector++;
                }
            }

            // Write the stripe data for this group
            for (int r = 0; r < stripes_per_group; ++r) {
                int stripe_index = g * stripes_per_group + r;
                fprintf(fp, "Stripe%d\t", stripe_index);
                for (int d = 0; d < N; ++d) {
                    fprintf(fp, "%lld", block[r][d]);
                    if (d != N - 1) fprintf(fp, "\t");
                }
                fprintf(fp, "\n");
            }
        }
    }

    // RAID 01: Striped mirror -> stripe across half the disks, mirrored to the other half
    else if (strcmp(level, "01") == 0) {
        int data_disks = N / 2;  // First half are data disks, second half are mirrors
        int stripes_per_group = 1;
        int groups = S / stripes_per_group;

        for (int g = 0; g < groups; ++g) {
            long long block[16][16];

            // Fill data disks with sectors
            for (int d = 0; d < data_disks; ++d) {
                for (int r = 0;  r < stripes_per_group; ++r) {
                    block[r][d] = current_sector++;
                }
            }

            // Write stripe with mirrored data
            for (int r = 0; r < stripes_per_group; ++r) {
                int stripe_index = g * stripes_per_group + r;
                fprintf(fp, "Stripe%d\t", stripe_index);

                // Output data disks followed by mirror copies
                for (int d = 0; d < N; ++d) {
                    int data_index = (d < data_disks) ? d : (d-data_disks);
                    fprintf(fp, "%lld", block[r][data_index]);
                    if (d != N - 1) fprintf(fp, "\t");
                }
                fprintf(fp, "\n");
            }
        }
    }

    // RAID 10: Mirrored stripe -> pair of disks striped, each pair is a mirror
    else if (strcmp(level, "10") == 0) {
        int pairs = N / 2;  // Number of disk pairs (each pair contains original + mirror)
        int stripes_per_group = 1;
        int groups = S / stripes_per_group;

        for (int g = 0; g < groups; ++g) {
            long long block[16][16];

            // Fill each pair with a sector
            for (int d = 0; d < pairs; ++d) {
                for (int r = 0;  r < stripes_per_group; ++r) {
                    block[r][d] = current_sector++;
                }
            }

            // Write stripe with mirrored pairs
            for (int r = 0; r < stripes_per_group; ++r) {
                int stripe_index = g * stripes_per_group + r;
                fprintf(fp, "Stripe%d\t", stripe_index);

                // Output each pair twice (original + mirror)
                for (int d = 0; d < N; ++d) {
                    int pair_index = d / 2;
                    fprintf(fp, "%lld", block[r][pair_index]);
                    if (d != N - 1) fprintf(fp, "\t");
                }
                fprintf(fp, "\n");
            }
        }
    }

    // RAID 4: Block striping with dedicated parity disk
    else if (strcmp(level, "4") == 0) {
        // Each stripe has data sectors on data disks and parity on last disk
        for (int s = 0; s < S; ++s) {
            fprintf(fp, "Stripe%d\t", s);
            long long base = (long long)s * (N-1);  // Calculate starting sector for this stripe

            for (int d = 0; d < N; ++d) {
                if (d == N - 1) {
                    // Last disk stores parity
                    fprintf(fp, "P");
                }

                else {
                    // Data disks get sequential sectors
                    fprintf(fp, "%lld", base + d);
                }

                if (d != N - 1) fprintf(fp, "\t");
            }
            fprintf(fp, "\n");
        }
    }

    // RAID 5: Block striping with distributed parity (parity disk rotates)
    else if (strcmp(level, "5") == 0) {
        // Parity is rotated across disks for better performance
        for (int s = 0; s < S; ++s) {
            fprintf(fp, "Stripe%d\t", s);

            long long base = (long long)s * (N-1);  // Calculate starting sector for this stripe
            int parity_disk = (N - 1) - (s % N);   // Rotate parity position based on stripe

            for (int d = 0; d < N; ++d) {
                if (d == parity_disk) {
                    // This disk holds parity for this stripe
                    fprintf(fp, "P");
                }

                else {
                    // Calculate which data sector goes on this disk
                    int j  = (d - parity_disk - 1 + N) % N;
                    long long val = base + j;
                    fprintf(fp, "%lld", val);
                }

                if (d != N - 1) fprintf(fp, "\t");
            }
            fprintf(fp, "\n");
        }
    }

    else {
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}