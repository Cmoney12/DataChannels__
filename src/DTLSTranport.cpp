//
// Created by corey on 3/11/23.
//
#include "../include/DTLSTransport.h"


DTLSTransport::DTLSTransport(const RTCCertificate &rtc_certificate): certificate_(std::make_unique<RTCCertificate>(rtc_certificate)) {}

bool DTLSTransport::init() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    ctx = SSL_CTX_new(DTLS_method());
    if (!ctx) {
        return false;
    }

    if (SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH") != 1) {
        return false;
    }

    SSL_CTX_set_read_ahead(ctx, 1);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_peer_certificate);
    SSL_CTX_use_PrivateKey(ctx, certificate_->evp_pkey());
    SSL_CTX_use_certificate(ctx, certificate_->x509());

    if (SSL_CTX_check_private_key(ctx) != 1) {
        return false;
    }

    ssl = SSL_new(ctx);
    if (!ssl) {
        return false;
    }

    in_bio = BIO_new(BIO_s_mem());
    if (!in_bio) {
        return false;
    }
    BIO_set_mem_eof_return(in_bio, -1);

    out_bio = BIO_new(BIO_s_mem());
    if (!out_bio) {
        return false;
    }
    BIO_set_mem_eof_return(out_bio, -1);

    SSL_set_bio(ssl, in_bio, out_bio);

    std::shared_ptr<EC_KEY> ecdh = std::shared_ptr<EC_KEY>(EC_KEY_new_by_curve_name(NID_X9_62_prime256v1), EC_KEY_free);
    SSL_set_options(ssl, SSL_OP_SINGLE_ECDH_USE);
    SSL_set_tmp_ecdh(ssl, ecdh.get());

    return true;
}

int DTLSTransport::verify_peer_certificate(int ok, X509_STORE_CTX *ctx) {
    return 1;
}





