#include "pch.h"
#include "socket.h"
#include "url.h"
#include "HTMLParserBase.h"
#pragma comment(lib, "ws2_32.lib")
using namespace std;
//Sebastian Oberg
//CSCE 463 - 500
//Fall 2023

//Initialize WinSock; once per program run
void winsock_test(void)
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData) != 0) 
    {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}
}

// Used to compare char* values when inserting in a set
struct CharStarComparator 
{
    bool operator() (const char* lhs, const char* rhs) const 
    {
        return strcmp(lhs, rhs) < 0;
    }
};

int main(int argc, char** argv)
{
    // Step 0: Allow the code to take in two arguments: numThreads and URL
    // Check Command - Line Arguments
    if (argc != 3)
    {
        printf("\nIncorrect Number of Arguments Passed In: %d\n", argc - 1);
        return 1;
    }

    int numThreads = atoi(argv[1]);

    if (numThreads != 1)
    {
        printf("Incorrect number of threads: %i. Only 1 is accepted!\n", numThreads);
        return 1;
    }

    char* filename = argv[2];

    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile failed with %d\n", GetLastError());
        return 1;
    }

    LARGE_INTEGER li;
    if (!GetFileSizeEx(hFile, &li))
    {
        printf("GetFileSizeEx error %d\n", GetLastError());
        CloseHandle(hFile);  // Close the handle before returning
        return 1;
    }

    size_t fileSize = static_cast<size_t>(li.QuadPart);
    printf("Opened %s with size %zu\n", filename, fileSize);
    DWORD bytesRead;

    char* fileBuf = new char[fileSize + 1];  // Added +1 to null-terminate

    if (!ReadFile(hFile, fileBuf, static_cast<DWORD>(fileSize), &bytesRead, NULL) || bytesRead != fileSize)
    {
        printf("ReadFile failed with %d\n", GetLastError());
        delete[] fileBuf;  // Avoid memory leak
        CloseHandle(hFile);  // Close the handle before returning
        return 1;
    }

    fileBuf[bytesRead] = '\0';  // Null-terminate to work safely with C-string functions

    CloseHandle(hFile);

    queue<char*> urlQueue;
    char* currentStart = fileBuf;

    for (size_t i = 0; i < fileSize; i++)
    {
        if (fileBuf[i] == '\n' || i == fileSize - 1)
        {
            size_t endIndex = (i == fileSize - 1 && fileBuf[i] != '\n') ? i : i - 1;
            size_t urlLength = &fileBuf[endIndex] - currentStart + 1;
            char* currentURL = new char[urlLength + 1];
            strncpy_s(currentURL, urlLength + 1, currentStart, urlLength);
            currentURL[urlLength] = '\0';

            urlQueue.push(currentURL);
            currentStart = &fileBuf[i + 1];
        }
    }

    // Socket Initialization and Connection
    winsock_test();
    Socket mySocket;
    mySocket.openSocket(); // Error handling is included in method
    set<string> ipSet;

    url currentURL;
    set<char*, CharStarComparator> hostSet;

    while (!urlQueue.empty()) 
    {
        char* curURL = urlQueue.front();
        printf("URL: %s\n", curURL);

        currentURL.parseURL(curURL);

        // Check if host is unique using a set
        auto hostResult = hostSet.insert(currentURL.host);

        if (hostResult.second) 
        {
            printf("        Checking host uniqueness... passed\n");
            char* ipAddr = mySocket.dns(currentURL.host, currentURL.portPos, ipSet);
        }

        else 
        {
            printf("        Checking host uniqueness... failed\n");
        }

        urlQueue.pop();
    }

    return 0;
}