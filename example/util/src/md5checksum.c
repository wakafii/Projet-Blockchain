#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/md5.h>
#include "util/md5checksum.h"

unsigned char result[MD5_DIGEST_LENGTH];

// Print the MD5 sum as hex-digits.
void print_md5_sum(unsigned char* md, unsigned char* hex_buf) {
    int i;
    for(i=0; i <MD5_DIGEST_LENGTH; i++) {
            sprintf(hex_buf, "%02x",md[i]);
    }
}

// Get the size of the file by its file descriptor
unsigned long get_size_by_fd(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}

unsigned char* md5checksum(char* filename) {
    int file_descript;
    unsigned long file_size;
    char* file_buffer;
    unsigned char* hex_buf;
     
    hex_buf = (unsigned char*) malloc(MD5_DIGEST_LENGTH * sizeof(unsigned char));
    printf("using file:\t%s\n", filename);

    file_descript = open(filename, O_RDONLY);
    if(file_descript < 0) 
   		printf("open %s failed\n", filename);

    file_size = get_size_by_fd(file_descript);
    printf("file size:\t%lu\n", file_size);

    file_buffer = mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0);
    MD5((unsigned char*) file_buffer, file_size, result);
    munmap(file_buffer, file_size); 

    print_md5_sum(result, hex_buf);
    printf("  %s\n", filename);

    return hex_buf;
}


