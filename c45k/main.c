#include <stdio.h>
#include <WinSock2.h>
#include <windef.h>
#include <inaddr.h>
#include <ws2def.h>


int main() {
	WSADATA data;
	int result = WSAStartup(MAKEWORD(2, 2), &data);
	if (result != 0) {
		printf("WSAStartup failed with error: %d\n", result);
		return 1;
	}
	SOCKET m_fd = socket(AF_INET, SOCK_STREAM, 0);

	// cannot create socket
	if (m_fd < 0) {
		printf("Cannot create socket");
	}

	struct hostent* hostEnt = gethostbyname("127.0.0.1");
	// starting to connect to server
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(7495);
	sa.sin_addr.s_addr = ((struct in_addr*)hostEnt->h_addr)->s_addr;

	if ((connect(m_fd, (struct sockaddr*)&sa, sizeof(sa))) < 0) {
		int error = WSAGetLastError();
		printf("Failed to connect. Error code: %d\n", error);
		closesocket(m_fd);
		WSACleanup();
	}
	unsigned long m_waitTimeout=100; // in milliseconds
	HANDLE m_evMsgs;
	while (1)
	{
		WaitForSingleObject(m_evMsgs, m_waitTimeout);
	}

	closesocket(m_fd);
	WSACleanup();
	return 0;
}