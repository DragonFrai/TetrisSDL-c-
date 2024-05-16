//
// Created by dragon on 22.04.2024.
//


char* copyStr(const char* source) {
    auto strLen = strlen(source);
    auto bufLen = strLen + 1;
    auto buf = (char*) malloc(bufLen);
    memcpy(buf, source, bufLen);
    return buf;
}

