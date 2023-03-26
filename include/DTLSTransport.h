//
// Created by corey on 3/11/23.
//

#ifndef DATACHANNELS_DTLSTRANSPORT_H
#define DATACHANNELS_DTLSTRANSPORT_H

#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/ssl.h>
#include "DTLSCertificate.h"

class DTLSTransport {
public:
    explicit DTLSTransport(const RTCCertificate& rtc_certificate);

    bool init();

    static int verify_peer_certificate(int ok, X509_STORE_CTX *ctx);


private:
    std::unique_ptr<RTCCertificate> certificate_;
    SSL_CTX *ctx{};
    SSL *ssl{};
    BIO *in_bio{}, *out_bio{};
    bool handshake_complete{};
};

#endif //DATACHANNELS_DTLSTRANSPORT_H
