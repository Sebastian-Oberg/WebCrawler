#include "pch.h"
#include "url.h"
#include "HTMLParserBase.h"
//Sebastian Oberg
//CSCE 463 - 500
//Fall 2023

url::url()
{
	// Default initialize member variables
    currentURL = nullptr;
    fragment = nullptr;
    fragmentIsNull = false;
    query = nullptr;
    queryIsNull = false;
    path = nullptr;
    pathIsNull = false;
    portPos = nullptr;
    portIsValid = true;
    portIsNull = false;
    port = 0;
    host = nullptr;

}

char* url::truncate(char* str, char c)
{
    char* index = strchr(str, c); // Get position where c is at in str
    size_t length = index - str; // Calculate length of truncated piece
    char* truncated = new char[length + 1]; // Allocate memory for truncated string
    strncpy_s(truncated, length + 1, str, length); // Copy truncated part into new buffer
    truncated[length] = '\0'; // Null-terminate new string
    return truncated;
}

int url::parseURL(char* currentURL)
{
    if (strncmp(currentURL, "http://", 7) != 0)
    {
        printf("\tParsing URL... failed with invalid scheme\n");
        return 1;
    }

    else
    {
        currentURL += 7; // Contiguous, so this "removes" the scheme from the url
    }

    // 1a. Find #, extract fragment, truncate
    fragment = strchr(currentURL, '#');

    // Throw error if fragment not found
    if (fragment == nullptr)
    {
        fragmentIsNull = true;
        fragment = new char[1];
        fragment[0] = '\0';
    }

    else
    {
        currentURL = truncate(currentURL, '#');
    }

    // 1b. Find ?, extract query, truncate
    query = strchr(currentURL, '?');

    if (query == nullptr)
    {
        queryIsNull = true;
        query = new char[1];
        query[0] = '\0';
    }

    else
    {
        currentURL = truncate(currentURL, '?');
    }

    // 1c. Find /, extract path, truncate
    path = strchr(currentURL, '/');

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
        currentURL = truncate(currentURL, '/');
    }

    // 1d. Find :, extract port, truncate, obtain host
    portPos = strchr(currentURL, ':');

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
        port = atoi(portPos + 1);
        if (port <= 0)
        {
            portIsValid = false;
        }

        else
        {
            portPos += 1;
            currentURL = truncate(currentURL, ':');
        }
    }

    if (!portIsValid)
    {
        printf("\tParsing URL... failed with invalid port\n");
        return 0;
    }

    host = currentURL; // Host as leftover from truncated URL

    // Additional Checks
    if (strlen(host) > MAX_HOST_LEN)
    {
        printf("Exceeded max host length %d\n", MAX_HOST_LEN);
    }

    printf("\tParsing URL... host %s, port %s\n", host, portPos);

    return 0;
}

url::~url()
{
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
}