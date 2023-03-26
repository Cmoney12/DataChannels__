//
// Created by corey on 3/11/23.
//

#ifndef DATACHANNELS_DATACHANNELCOMMON_H
#define DATACHANNELS_DATACHANNELCOMMON_H

#include <openssl/x509.h>
#include <string>
#include <vector>

struct IceCandidate {
    IceCandidate(std::string candidate, std::string sdpMid, int sdpMLineIndex)
            : candidate(std::move(candidate)), sdpMid(std::move(sdpMid)), sdpMLineIndex(sdpMLineIndex) {}

    std::string candidate;
    std::string sdpMid;
    int sdpMLineIndex;
};

struct IceServer {
    std::string hostname;
    int port;
};

struct Configuration {
    std::vector<IceServer> ice_servers;
    std::pair<unsigned, unsigned> ice_port_range;
    std::string ice_ufrag;
    std::string ice_pwd;
    //std::vector<RTCCertificate> certificates;
};



#endif //DATACHANNELS_DATACHANNELCOMMON_H
