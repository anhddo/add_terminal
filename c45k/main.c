#include <stdio.h>
#include <WinSock2.h>
#include <windef.h>
#include <inaddr.h>
#include <ws2def.h>
#include "add.h"

int main() {
	int c = Add(1, 2);
	printf("1 + 2 = %d\n", c);
	return 0;
}