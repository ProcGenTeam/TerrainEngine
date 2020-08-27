#pragma once
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

struct FNetworkState
{
    BIO *sktRAW;
    SSL *sktSSL;
    uint32_t uMemoryUsed;
    uint32_t uMemoryTotal;
};