#ifndef VERIFYPROCESS_H
#define VERIFYPROCESS_H
#include <sys/types.h>

bool IsCaCertAuthorizedByRootCa(void const* data, size_t size);

int verifyprocess(int argc, char* argv[]);

#endif
