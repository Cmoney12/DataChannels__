//
// Created by corey on 3/11/23.
//

#ifndef DATACHANNELS_DTLSCERTIFICATE_H
#define DATACHANNELS_DTLSCERTIFICATE_H

#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/ssl.h>
#include <memory>
#include <cstring>
#include <utility>
#include <string>
#include <stdexcept>

class RTCCertificate {
public:
    RTCCertificate(std::string& cert_pem, std::string& pkey_pem);

    static std::shared_ptr<X509> GenerateX509(const std::shared_ptr<EVP_PKEY>& evp_pkey,
                                              const std::string &common_name, int& days);

    static std::string GenerateFingerprint(const std::shared_ptr<X509>& x509);

    static RTCCertificate GenerateCertificate(const std::string& common_name, int days);

    [[nodiscard]] X509 *x509() const { return x509_.get(); }
    [[nodiscard]] EVP_PKEY *evp_pkey() const { return evp_pkey_.get(); }
    [[nodiscard]] const std::string &fingerprint() const { return fingerprint_; }

private:
    RTCCertificate(std::shared_ptr<X509> x509, std::shared_ptr<EVP_PKEY> evp_pkey)
            : x509_(std::move(x509)), evp_pkey_(std::move(evp_pkey)), fingerprint_(GenerateFingerprint(x509_)) {}

    std::shared_ptr<X509> x509_;
    std::shared_ptr<EVP_PKEY> evp_pkey_;
    std::string fingerprint_{};
    enum { SHA256_FINGERPRINT_SIZE = 96 };

};


#endif //DATACHANNELS_DTLSCERTIFICATE_H
