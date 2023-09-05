#include "HTMLParserBase.h"
#pragma once
//Sebastian Oberg
//CSCE 463 - 500
//Fall 2023

class Socket {
public:
    SOCKET sock;
    char* buf;
    int curPos;
    int allocatedSize;
    static const int INITIAL_BUF_SIZE = 1024; // Adjust this value as needed
    static const int THRESHOLD = 256; // Adjust this value as needed
    timeval* timeout;
    HTMLParserBase* parser;
    bool validConnection = false;

    Socket();
    ~Socket();
    bool Read();
    char* dns(char str[], char* portPos, set<string>& ipSet);
    void openSocket();
    void Close();
    void End();
    bool Send(const char* req, int length);
    int parse(char* html, int htmlSize, char* baseURL, int baseURLSize);
    int getStatusCode();
};
