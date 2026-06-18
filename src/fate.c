// SPDX-License-Identifier: GPL-2.0
/*
 * Fate - A program providing alternative, spiritually accurate diagnostics for
 * Linux PIDs.
 *
 * Copyright (c) 2026 Joshua Crofts <joshua.crofts1@gmail.com>
 */

#include <errno.h>
#include <getopt.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef FATE_DATA_DIR
#define FATE_DATA_DIR "/usr/share/fate/"
#endif

#ifndef FATE_VERSION
#define FATE_VERSION "err"
#endif

#define FNV_PRIME 16777619
#define FNV_OFFSET_BASIS 2166136261u

struct process_info {
        int pid;
        char state;
        int pgrp;
        int processor;
        unsigned long long utime;
        unsigned long long start_time;
        char name[64];
};

struct option flags[] = {
        { "version", no_argument, 0, 'v'},
        { "help", no_argument, 0, 'h'},
        { "predict", required_argument, 0, 'p'},
        { 0, 0, 0, 0}
};

static int get_process_info(struct process_info *info)
{
        char path[64];
        FILE *f;
        int ret;

        snprintf(path, sizeof(path), "/proc/%d/stat", info->pid);
        f = fopen(path, "r");
        if (!f)
                return -ENOENT;

        ret = fscanf(f, "%d %s %c %d %lld %lld %d",
                     &info->pid,
                     info->name,
                     &info->state,
                     &info->pgrp,
                     &info->utime,
                     &info->start_time,
                     &info->processor);

        if (ret < 7) {
                fclose(f);
                return -1;
        }

        fclose(f);
        return 0;
}

int get_dat_byte_index(char * file, int rand_inx)
{
        uint32_t ver, numstr, longlen, shortlen, flags;
        uint32_t start_byte;
        uint32_t *offsets;
        char path[32] = {0};
        char delim;

        snprintf(path, sizeof(path), FATE_DATA_DIR "/%s", file);

        FILE *f = fopen(path, "r");
        if (!f)
                return -ENOENT;

        fread(&ver, sizeof(uint32_t), 1, f);
        fread(&numstr, sizeof(uint32_t), 1, f);
        fread(&longlen, sizeof(uint32_t), 1, f);
        fread(&shortlen, sizeof(uint32_t), 1, f);
        fread(&flags, sizeof(uint32_t), 1, f);
        fread(&delim, sizeof(char), 1, f);

        ver = __builtin_bswap32(ver);
        numstr = __builtin_bswap32(numstr);
        longlen = __builtin_bswap32(longlen);
        shortlen = __builtin_bswap32(shortlen);
        flags = __builtin_bswap32(flags);

        fseek(f, 24, SEEK_SET);

        offsets = malloc((numstr + 1) * sizeof(uint32_t));
        if (!offsets) {
                fclose(f);
                return -ENOMEM;
        }

        fread(offsets, sizeof(uint32_t), numstr + 1, f);

        start_byte = __builtin_bswap32(offsets[rand_inx]);

        free(offsets);
        fclose(f);

        return start_byte;
}

int read_and_write_from_file(char *filename, int start_byte)
{
        char path[32] = {0};
        char c;

        snprintf(path, sizeof(path), FATE_DATA_DIR "/%s", filename);

        FILE *f = fopen(path, "r");
        if (!f)
                return -ENOENT;

        fseek(f, start_byte, SEEK_SET);

        while ((c = fgetc(f)) != '@')
                putchar(c);

        fclose(f);
        return 0;
}

static void print_result(struct process_info *info)
{
        int byte_index;
        printf("\U0001F52E Ah, %s, a wise entity on this silicon plane.\n", info->name);
        printf("\U0001F52E Born in the epoch %lld.\n", info->start_time);
        printf("\U0001F52E Lucky syscall: ");
        byte_index = get_dat_byte_index("syscalls.txt.dat", rand() % 414);
        read_and_write_from_file("syscalls.txt", byte_index);
        printf("\U0001F52E Unlucky syscall: ");
        byte_index = get_dat_byte_index("syscalls.txt.dat", rand() % 414);
        read_and_write_from_file("syscalls.txt", byte_index);
}

/*
 * Fowler-No-Voll hash implementation
 *
 */
unsigned int get_fnv_hash(int pid, int year, int month, int day)
{
        int hash = FNV_OFFSET_BASIS;
        char pid_timestamp[64];

        snprintf(pid_timestamp, sizeof(pid_timestamp), "%d%d%d%d", pid, year, month, day);

        for (int i = 0; pid_timestamp[i] != '\0'; i++) {
                hash ^= (unsigned char)pid_timestamp[i];
                hash *= FNV_PRIME;
        }
        
        return hash;
}

int main(int argc, char **argv)
{
        struct process_info info;
        struct tm *tm;
        time_t t;
        int options_inx = 0;
        int option;
        int ret;
        
        setlocale(LC_ALL, "");

        while ((option = getopt_long(argc, argv, "p:hv", flags, &options_inx)) != -1) {
                switch (option) {
                case 'v':
                        printf("fate " FATE_VERSION "\n");
                        return 0;
                case 'p':
                        info.pid = atoi(optarg);
                        break;
                case 'h':
                default:
                        printf("Usage: fate [-hvp] [PID_VALUE]\n");
                        return 0;
                }
        }
        
        if (optind < argc)
                info.pid = atoi(argv[optind]);

        ret = get_process_info(&info);
        if (ret) {
                puts("\U0001F52E The stars haven't aligned, try again next time...");
                return ret;
        }
       
        t = time(NULL);
        tm = localtime(&t);

        srand(get_fnv_hash(info.pid, tm->tm_year, tm->tm_mon, tm->tm_mday)); 

        print_result(&info);

        return 0;
}
