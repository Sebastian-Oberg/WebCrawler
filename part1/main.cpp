#include "pch.h"
#include "socket.h"
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

char* truncate(char* str, char c)
{
    char* index = strchr(str, c); // Get position where c is at in str
    size_t length = index - str; // Calculate length of truncated piece
    char* truncated = new char[length + 1]; // Allocate memory for truncated string
    strncpy_s(truncated, length + 1, str, length); // Copy truncated part into new buffer
    truncated[length] = '\0'; // Null-terminate new string
    return truncated;
}

int main(int argc, char** argv)
{

    /*
        Step 1: Get URL and validate
    */
    char* url = argv[1]; // Get URL
    char* originalURL = argv[1];
    printf("URL: %s\n", url);

    // Check Command - Line Arguments
    if (argc != 2)
    {
        printf("\n Incorrect Number of Arguments Passed In: %d", argc);
        return 1;
    }

    if (strlen(url) > MAX_URL_LEN || strlen(url) > MAX_REQUEST_LEN)
    {
        printf("ERROR: Length of string violates max host length: %d or max length: %d", MAX_HOST_LEN, MAX_REQUEST_LEN);
        return -1;
    }

    /*
         Step 2: URL Parsing
            Extract URL components + validate the extracted components
            - Scheme can only be http
            - If the path is not present, must use root “/” in its place
            - Non zero port if there is one
    */

    // Check Scheme
    if (strncmp(url, "http://", 7) != 0) 
    {
        printf("\tParsing URL... failed with invalid scheme\n");
        return 1;
    }

    else
    {
        url += 7; // Contiguous, so this "removes" the scheme from the url
    }
  
    // 1a. Find #, extract fragment, truncate
    char* fragment = strchr(url, '#');
    bool fragmentIsNull = false;
     
    // Throw error if fragment not found
    if (fragment == nullptr)
    {
        fragmentIsNull = true;
        fragment = new char[1];
        fragment[0] = '\0';
    }

    else
    {
        url = truncate(url, '#');
    }
    
    // 1b. Find ?, extract query, truncate
    char* query = strchr(url, '?');
    bool queryIsNull = false;

    if (query == nullptr)
    {
        queryIsNull = true;
        query = new char[1];
        query[0] = '\0';
    }

    else
    {
        url = truncate(url, '?');
    }

    // 1c. Find /, extract path, truncate
    char* path = strchr(url, '/');
    bool pathIsNull = false;

    // If path not present, use root “/” in its place
    if (path == nullptr)
    {
        pathIsNull = true;
        path = new char[2];
        path[0] = '/';
        path[1] = '\0';
    }

    else
    {
        url = truncate(url, '/');
    }

    // 1d. Find :, extract port, truncate, obtain host
    char* portPos = strchr(url, ':');
    bool portIsValid = true;
    bool portIsNull = false;

    if (portPos == nullptr)
    {
        portIsNull = true;
        portPos = new char[3];
        portPos[0] = '8';
        portPos[1] = '0';
        portPos[2] = '\0';
    }

    else
    {
        int port = atoi(portPos + 1);
        if (port <= 0)
        {
            /*return 1;*/
            portIsValid = false;
        }

        else
        {
            portPos += 1;
            url = truncate(url, ':');
        }
    }

    if (!portIsValid)
    {
        printf("\tParsing URL... failed with invalid port\n");
        return 0;
    }

    char* host = url; // Host as leftover from truncated URL

    // Additional Checks
    if (strlen(host) > MAX_HOST_LEN)
    {
        printf("Exceeded max host length %d\n", MAX_HOST_LEN);
    }

    printf("\tParsing URL... host %s, port %s, request %s%s%s\n", host, portPos, path, query, fragment);

    /*
         Step 3: DNS (Clock this!)
            Perform DNS lookup - translates human readable domain names (for example, www.amazon.com) to machine readable IP
            Handle cases where the DNS lookup fails
            - 1. Connect but not valid reply
            - 2. Cannot connect
    */

    // Removing beginning characters for query fragment and path if not nullptr
    if (!queryIsNull)
    {
        query += 1;
    }

    if (!fragmentIsNull)
    {
        fragment += 1;
    }

    // Step 4: Socket Initialization and Connection
    winsock_test();

    // Create socket
    Socket mySocket;
    mySocket.openSocket(); // Error handling is included in method

    // Find DNS
    mySocket.dns(host, portPos); // Calls connect

    // Step 5: HTTP Request Construction
 
    // Construct the HTTP request
    char request[2048];
    sprintf_s(request, "GET %s%s HTTP/1.0\r\nHost: %s\r\n\r\n", path, (query != nullptr) ? query : "", host);

    // Step 6: Sending and Receiving Data

    // Send the request
    if (!mySocket.Send(request, static_cast<int>(strlen(request))))
    {
        printf("Failed to send request.\n");
        return 1;
    }

    // Receive and process data in a loop
    if (!mySocket.Read())
    {
        printf("Failed to receive response.\n");
        return 1;
    }

    //Step 7: Parsing HTTP Response
    bool fullHTMLParse = false;
    int statusCode = mySocket.getStatusCode(); // Extract status code

    // Status code is null
    if (statusCode < 0)
    {
        printf("ERROR: Buffer is equal to NULL\n");
        return -1;
    }

    // Status code is invalid
    if (statusCode < 100 || statusCode > 599)
    {
        printf("ERROR: Invalid status code %i\n", statusCode);
        return -1;
    }

    // Positive Status Codes
    printf("\tVerifying header... status code %i\n", statusCode);

    // Range where we want to parse
    if ((statusCode >= 200) && (statusCode < 300))
    {
        fullHTMLParse = true;
    }

    // Step 8: Displaying Results

    // Range where we want to skip the HTML parser, but print everything else
    if (mySocket.validConnection && fullHTMLParse)
    {
        // Call html parser
        char* position = strstr(mySocket.buf, "\r\n\r\n");
        mySocket.parse(position, mySocket.curPos - atoi(position + 4), originalURL, strlen(originalURL));

        printf("----------------------------------------\n");

        // Print truncated html
        char* truncated = truncate(mySocket.buf, '<');
        printf(truncated);
    }

    else if (mySocket.validConnection && !fullHTMLParse)
    {
        printf("----------------------------------------\n");
        char* truncated = truncate(mySocket.buf, '<');
        printf(truncated);
    }

    else
    {
        printf("\tConnection Error: %d\n", WSAGetLastError());
        printf("----------------------------------------\n");
    }

    // Step 10: Cleanup and Finalization
    mySocket.Close();
    mySocket.End();

    // Deallocate dynamic memory
    if (pathIsNull)
    {
        delete[] path;
    }

    if (portIsNull)
    {
        delete[] portPos;
    }

    if (queryIsNull)
    {
        delete[] query;
    }

    return 0;
}