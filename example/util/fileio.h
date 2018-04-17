#ifndef FILEIO_H
#define FILEIO_H

int sendfile(int sockfd, char* fn);
int receivefile(int sockfd, char* fn, size_t size);

#endif
