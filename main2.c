#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long long raid0_sector(int disk, int stripe, int N, int C) {
    int superstripe = stripe / C;
    int r = stripe % C;
    long long base = (long long)superstripe * N * C;
    return base + r + (long long)C * disk;
}

int main(void) {
    char T[3];
    int N, C, S;
    char filename[256];

    if (scanf("%2s %d %d %d %255s", T, &N, &C, &S, filename) != 5) {
        return 1; 
    }

    FILE *f = fopen(filename, "w");
    if (!f) {
        return 1; 
    }

    fprintf(f, "\t");
    for (int d = 0; d < N; ++d) {
        fprintf(f, "Disk%d", d);
        if (d != N - 1) {
            fprintf(f, "\t");
        }
    }
    fprintf(f, "\n");

    if (strcmp(T, "0") == 0) {
        for (int s = 0; s < S; ++s) {
            fprintf(f, "Stripe%d\t", s);
            for (int d = 0; d < N; ++d) {
                long long val = raid0_sector(d, s, N, C);
                fprintf(f, "%lld", val);
                if (d != N - 1) {
                    fprintf(f, "\t");
                }
            }
            fprintf(f, "\n");
        }

    } else if (strcmp(T, "01") == 0) {
        int Nd = N / 2;
        for (int s = 0; s < S; ++s) {
            fprintf(f, "Stripe%d\t", s);
            for (int d = 0; d < N; ++d) {
                int data_disk = (d < Nd) ? d : (d - Nd);
                long long val = raid0_sector(data_disk, s, Nd, 1); 
                fprintf(f, "%lld", val);
                if (d != N - 1) {
                    fprintf(f, "\t");
                }
            }
            fprintf(f, "\n");
        }

    } else if (strcmp(T, "10") == 0) {
        int Nd = N / 2; 
        for (int s = 0; s < S; ++s) {
            fprintf(f, "Stripe%d\t", s);
            for (int d = 0; d < N; ++d) {
                int pair = d / 2; 
                long long val = raid0_sector(pair, s, Nd, 1);
                fprintf(f, "%lld", val);
                if (d != N - 1) {
                    fprintf(f, "\t");
                }
            }
            fprintf(f, "\n");
        }

    } else if (strcmp(T, "4") == 0) {
        for (int s = 0; s < S; ++s) {
            fprintf(f, "Stripe%d\t", s);
            long long base = (long long)s * (N - 1);
            for (int d = 0; d < N; ++d) {
                if (d == N - 1) {
                    fprintf(f, "P");
                } else {
                    long long val = base + d;
                    fprintf(f, "%lld", val);
                }
                if (d != N - 1) {
                    fprintf(f, "\t");
                }
            }
            fprintf(f, "\n");
        }

    } else if (strcmp(T, "5") == 0) {
        for (int s = 0; s < S; ++s) {
            fprintf(f, "Stripe%d\t", s);
            long long base = (long long)s * (N - 1);
            int parity_disk = (N - 1 - (s % N)); 
            for (int d = 0; d < N; ++d) {
                if (d == parity_disk) {
                    fprintf(f, "P");
                } else {
                    int j = (d - parity_disk - 1 + N) % N;
                    long long val = base + j;
                    fprintf(f, "%lld", val);
                }
                if (d != N - 1) {
                    fprintf(f, "\t");
                }
            }
            fprintf(f, "\n");
        }

    } else {
        fclose(f);
        return 1;
    }

    fclose(f);
    return 0;
}
