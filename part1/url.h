#pragma once
//Sebastian Oberg
//CSCE 463 - 500
//Fall 2023

class url {
public:
    char* currentURL;
    char* fragment;
    bool fragmentIsNull;
    char* query;
    bool queryIsNull;
    char* path;
    bool pathIsNull;
    char* portPos;
    bool portIsValid;
    bool portIsNull;
    int port;
    char* host;

    url();
    ~url();
    char* truncate(char* str, char c);
    int parseURL(char* url);
};
