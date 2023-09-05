#include "pch.h"
#include "socket.h"
//Sebastian Oberg
//CSCE 463 - 500
//Fall 2023

Socket::Socket()
{
    this->sock = INVALID_SOCKET;
	buf = new char[INITIAL_BUF_SIZE];
    curPos = 0;
	allocatedSize = INITIAL_BUF_SIZE;
    timeout = new timeval;
    parser = new HTMLParserBase;
}

bool Socket::Read(void) {
    // Initialize time timevals
    timeout->tv_sec = 10;
    timeout->tv_usec = 0;

    fd_set fd; // Declare a file descriptor
    int ret;
    clock_t start = clock();

    while (true)
    {
        FD_ZERO(&fd); // Clear file descriptors
        FD_SET(sock, &fd); // Add socket descriptor to the fd_set to monitor

        // wait to see if socket has any data (see MSDN)
        if ((ret = select(0, &fd, nullptr, nullptr, timeout)) > 0)
        {
            // Ensure we don't read beyond the buffer size
            int spaceAvailable = allocatedSize - curPos;

            if (spaceAvailable <= 0) 
            {
                printf("\tBuffer space exhausted.\n");
                return false;
            }

            // new data available; now read the next segment
            int bytes = recv(sock, buf + curPos, spaceAvailable, 0);

            if (bytes == SOCKET_ERROR)
            {
                printf("\tSocket error: %d\n", WSAGetLastError());
                break;
            }

            // normal completion
            if (bytes == 0)
            {
                buf[curPos] = '\0'; // NULL-terminate buffer
                clock_t finish = clock();
                printf("\tLoading... done in %d ms with %d bytes\n", (finish - start), curPos);
                return true;
            }

            curPos += bytes; // adjust where the next recv goes

            // When available buffer space is below the threshold, resize
            if (allocatedSize - curPos < THRESHOLD)
            {
                int newSize = allocatedSize * 2;
                char* newBuf = new (std::nothrow) char[newSize]; // Use nothrow to avoid exceptions

                if (!newBuf) 
                {
                    printf("\tFailed to allocate new buffer.\n");
                    return false;
                }

                memcpy(newBuf, buf, curPos);
                delete[] buf;
                buf = newBuf;
                allocatedSize = newSize;
            }
        }

        else if (ret == 0)
        {
            // report timeout
            printf("\tLoading... failed with timeout\n");
            break;
        }

        else
        {
            // print WSAGetLastError()
            printf("\tLoading... failed with %d\n", WSAGetLastError());
            break;
        }
    }
    return false;
}


char* Socket::dns(char str[], char* portPos) {
	// string pointing to an HTTP server (DNS name or IP)
	/*char str[] = "www.tamu.edu";*/
	//We get -> char str [] = "128.194.135.72";
    clock_t startConnect;
    clock_t finishConnect;
    clock_t startDNS;
    clock_t finishDNS;

    startDNS = clock();
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// structure used in DNS lookups
	struct hostent* remote;

	// structure for connecting to server
	struct sockaddr_in server;
    memset(&server, 0, sizeof(server)); // Fill with zeros

	// first assume that the string is an IP address
	DWORD IP = inet_addr(str);

	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		if ((remote = gethostbyname(str)) == NULL)
		{
            printf("\tDoing DNS... failed with %d\n", WSAGetLastError());
			return nullptr;
		}
		else // take the first IP address and copy into sin_addr
		{
			memcpy((char*)&(server.sin_addr), remote->h_addr, remote->h_length);
		}
	}

	else
	{
		// if a valid IP, directly drop its binary version into sin_addr
		server.sin_addr.S_un.S_addr = IP;
	}

    finishDNS = clock();
    printf("\tDoing DNS... done in %d ms, found %s\n", (finishDNS - startDNS), inet_ntoa(server.sin_addr));

	// setup the port # and protocol type
	server.sin_family = AF_INET;

    startConnect = clock();
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(portPos));

	// connect to the server on port 80
	if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
        printf("      * Connecting on page... failed with %d\n", WSAGetLastError());
		return nullptr;
	}

    finishConnect = clock();
    printf("      * Connecting on page... done in %d ms\n", finishConnect - startConnect);

    // At the end, before returning, return the IP as a string
    return inet_ntoa(server.sin_addr);
}

void Socket::openSocket() {
    // open a TCP socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) 
    {
        printf("Oppening socket generated error %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }
}

void Socket::Close() 
{
    closesocket(this->sock);
}

void Socket::End() 
{
	WSACleanup();
}

Socket::~Socket() {
	delete[] buf;
    delete parser;
}

bool Socket::Send(const char* req, int length)
{
    int bytesSent = 0;
    while (bytesSent < length)
    {
        int ret = send(sock, req + bytesSent, length - bytesSent, 0);

        if (ret == SOCKET_ERROR)
        {
            printf("Send error: %d\n", WSAGetLastError());
            return false;
        }

        else if (ret == 0)
        {
            printf("Connection closed during send.\n");
            return false;
        }

        bytesSent += ret;
    }
    return true;
}

int Socket::getStatusCode()
{
    if (buf == nullptr)
    {
        return -1;
    }

    // Parse status code from response
    char* statusCodeStart = strstr(buf, "HTTP/"); // find start of HTTP version

    if (!statusCodeStart)
    {
        return -1;
    }

    // Move pointer ahead by length of "HTTP/1.x " to point to status code
    statusCodeStart += 9;

    // Extract 3-character status code
    char statusCodeStr[4];  // extra byte for the null terminator

    // Use strncpy_s instead of strncpy
    strncpy_s(statusCodeStr, sizeof(statusCodeStr), statusCodeStart, 3);

    statusCodeStr[3] = '\0';  // Null terminate the string

    // Convert string to int
    validConnection = true;
    int statusCode = atoi(statusCodeStr);
    return statusCode;
}

int Socket::parse(char* html, int htmlSize, char* baseURL, int baseURLSize)
{
    clock_t start = clock();
    char* baseUrl = baseURL;		// where this page came from; needed for construction of relative links
    int nLinks;
    char* linkBuffer = parser->Parse(html, htmlSize, baseURL, (int)strlen(baseURL), &nLinks);

    // check for errors indicated by negative values
    if (nLinks < 0)
    {
        nLinks = 0;
    }
    
    clock_t finish = clock();
    printf("      + Parsing page... done in %d ms with %d links\n", (finish - start), nLinks);
    return 0;
}