//
// Created by corey on 3/11/23.
//

#include "../include/DTLSCertificate.h"


RTCCertificate::RTCCertificate(std::string &cert_pem, std::string &pkey_pem) {
    BIO *bio = BIO_new(BIO_s_mem());
    BIO_write(bio, cert_pem.c_str(), (int)cert_pem.length());

    x509_ = std::shared_ptr<X509>(PEM_read_bio_X509(bio, nullptr, nullptr, nullptr), X509_free);
    BIO_free(bio);
    if (!x509_) {
        throw std::invalid_argument("Could not read cert_pem");
    }

    /* evp_pkey */
    bio = BIO_new(BIO_s_mem());
    BIO_write(bio, pkey_pem.c_str(), (int)pkey_pem.length());

    evp_pkey_ = std::shared_ptr<EVP_PKEY>(PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr), EVP_PKEY_free);
    BIO_free(bio);

    if (!evp_pkey_) {
        throw std::invalid_argument("Could not read pkey_pem");
    }

    fingerprint_ = GenerateFingerprint(x509_);
}

std::shared_ptr<X509> RTCCertificate::GenerateX509(const std::shared_ptr<EVP_PKEY> &evp_pkey,
                                                   const std::string &common_name, int &days) {

    std::shared_ptr<X509> null_result;

    std::shared_ptr<X509> x509(X509_new(), X509_free);
    std::shared_ptr<BIGNUM> serial_number(BN_new(), BN_free);
    std::shared_ptr<X509_NAME> name(X509_NAME_new(), X509_NAME_free);

    if (!x509 || !serial_number || !name) {
        return null_result;
    }

    if (!X509_set_pubkey(x509.get(), evp_pkey.get())) {
        return null_result;
    }

    if (!BN_rand(serial_number.get(), 64, 0, 0)) {
        return null_result;
    }

    ASN1_INTEGER *asn1_serial_number = X509_get_serialNumber(x509.get());
    if (!asn1_serial_number) {
        return null_result;
    }

    if (!BN_to_ASN1_INTEGER(serial_number.get(), asn1_serial_number)) {
        return null_result;
    }

    if (!X509_set_version(x509.get(), 0L)) {
        return null_result;
    }

    if (!X509_NAME_add_entry_by_NID(name.get(), NID_commonName, MBSTRING_UTF8, (unsigned char *)common_name.c_str(), -1, -1, 0)) {
        return null_result;
    }

    if (!X509_set_subject_name(x509.get(), name.get()) || !X509_set_issuer_name(x509.get(), name.get())) {
        return null_result;
    }

    if (!X509_gmtime_adj(X509_get_notBefore(x509.get()), 0) || !X509_gmtime_adj(X509_get_notAfter(x509.get()), days * 24 * 3600)) {
        return null_result;
    }

    if (!X509_sign(x509.get(), evp_pkey.get(), EVP_sha1())) {
        return null_result;
    }

    return x509;
}

std::string RTCCertificate::GenerateFingerprint(const std::shared_ptr<X509> &x509) {

    unsigned int len;
    unsigned char buf[4096] = {0};
    if (!X509_digest(x509.get(), EVP_sha256(), buf, &len)) {
        throw std::runtime_error("GenerateFingerprint(): X509_digest error");
    }

    if (len > SHA256_FINGERPRINT_SIZE) {
        throw std::runtime_error("GenerateFingerprint(): fingerprint size too large for buffer!");
    }

    int offset = 0;
    char fp[SHA256_FINGERPRINT_SIZE];
    std::memset(fp, 0, SHA256_FINGERPRINT_SIZE);
    for (unsigned int i = 0; i < len; ++i) {
        snprintf(fp + offset, 4, "%02X:", buf[i]);
        offset += 3;
    }

    fp[offset - 1] = '\0';
    return std::string {fp };

}

/**
 * Deprecated openssl methods
 * @param common_name
 * @param days
 * @return
 *
 **/
RTCCertificate RTCCertificate::GenerateCertificate(const std::string& common_name, int days) {
    std::shared_ptr<EVP_PKEY> pkey(EVP_PKEY_new(), EVP_PKEY_free);

    if (!pkey) {
        throw std::runtime_error("GenerateCertificate: Failed to create EVP_PKEY");
    }

    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);

    if (!pctx) {
        throw std::runtime_error("GenerateCertificate: Failed to create EVP_PKEY_CTX");
    }

    if (EVP_PKEY_keygen_init(pctx) <= 0 || EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        throw std::runtime_error("GenerateCertificate: Failed to initialize EVP_PKEY_CTX");
    }

    EVP_PKEY* pkey_ptr = pkey.get();
    if (EVP_PKEY_keygen(pctx, &pkey_ptr) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        throw std::runtime_error("GenerateCertificate: Failed to generate RSA key pair");
    }

    EVP_PKEY_CTX_free(pctx);

    auto cert = GenerateX509(pkey, common_name, days);

    if (!cert) {
        throw std::runtime_error("GenerateCertificate: Error in GenerateX509");
    }

    return { cert, pkey };
}



/**
*
 * RTCCertificate RTCCertificate::GenerateCertificate(const std::string& common_name, int days) {
    EVP_PKEY_CTX* pkey_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!pkey_ctx) {
        throw std::runtime_error("GenerateCertificate: Error creating EVP_PKEY_CTX");
    }

    if (EVP_PKEY_keygen_init(pkey_ctx) <= 0
        || EVP_PKEY_CTX_set_rsa_keygen_bits(pkey_ctx, 2048) <= 0) {

        EVP_PKEY_CTX_free(pkey_ctx);
        throw std::runtime_error("GenerateCertificate: Error initializing EVP_PKEY_CTX");
    }

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(pkey_ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(pkey_ctx);
        throw std::runtime_error("GenerateCertificate: Error generating key");
    }

    EVP_PKEY_CTX_free(pkey_ctx);

    auto cert = GenerateX509(pkey, common_name, days);
    if (!cert) {
        EVP_PKEY_free(pkey);
        throw std::runtime_error("GenerateCertificate: Error in GenerateX509");
    }

    return { cert, pkey };
}

*/