#include "curl_wrap.h"

static size_t ReadFunction(
    char* data, size_t bytes, size_t nitems, void* ctx
) {
    size_t length = 0;
    Curl* curl = (Curl*) ctx;
    CurlRead* curl_read = curl->read_data();

    switch (curl_read->method) {
        case CURL_FILE:
            return fread(data, bytes, nitems, curl_read->file);

        case CURL_FROM: { /* TODO */ break; }

        default: break; /* ignore case */
    }

    return length;
}

static size_t WriteFunction(
    char* data, size_t bytes, size_t nitems, void* ctx
) {
    size_t length = bytes * nitems;
    Curl* curl = (Curl*) ctx;
    CurlWrite* curl_write = curl->write_data();

    switch (curl_write->method) {
        case CURL_FILE:
            return fwrite(data, bytes, nitems, curl_write->file);

        case CURL_RETURN: { /* TODO */ break; }

        default: break; /* ignore case */
    }

    return length;
}

static void SetDefaultOptions(CURL* curl) {
    if (curl == NULL) {
        return;
    }

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFunction);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
}

Curl::Curl(CURL* curl) :
    curl_(curl), write_data_(NULL),
    read_data_(NULL), last_error_(CURLE_OK) {}

Curl::~Curl() {
    if (curl_ != NULL) {
        curl_easy_cleanup(curl_);
    }
}

Curl* Curl::Initialize() {
    CURL* curl = curl_easy_init();

    if (curl == NULL) {
        return NULL;
    }

    SetDefaultOptions(curl);
    return new Curl(curl);
}

bool Curl::SetOptionHandle(CURLoption option, void* handle) {
    return true;
}

bool Curl::SetOptionInteger(CURLoption option, int param) {
    return true;
}

bool Curl::SetOptionString(CURLoption option, const char* str) {
    return true;
}

Curl* Curl::MakeDuplicate() {
    CURL* dupl_curl = curl_easy_duphandle(curl_);
    if (dupl_curl == NULL) {
        return NULL;
    }

    SetDefaultOptions(dupl_curl);

    Curl* duplicate = new Curl(dupl_curl);

    duplicate->set_write_data(write_data_);
    duplicate->set_read_data(read_data_);
    duplicate->set_last_error(last_error_);

    return duplicate;
}

bool Curl::Exec() {
    last_error_ = curl_easy_setopt(curl_, CURLOPT_WRITEDATA, write_data_);
    if (last_error_ != CURLE_OK) {
        return false;
    }

    last_error_ = curl_easy_setopt(curl_, CURLOPT_READDATA, read_data_);
    if (last_error_ != CURLE_OK) {
        return false;
    }

    last_error_ = curl_easy_perform(curl_);

    return last_error_ == CURLE_OK;
}

void Curl::Reset() {
    curl_easy_reset(curl_);
    SetDefaultOptions(curl_);
}

void Curl::UrlEncode(const char* url, ke::AString* out) {
    if (out == NULL) {
        return;
    }

    char* escaped = curl_easy_escape(curl_, url, 0);
    *out = escaped;
    curl_free(escaped);
}

void Curl::UrlDecode(const char* url, ke::AString* out) {
    if (out == NULL) {
        return;
    }

    char* unescaped = curl_easy_unescape(curl_, url, 0, NULL);
    *out = unescaped;
    curl_free(unescaped);
}