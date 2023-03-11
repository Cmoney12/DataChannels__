//
// Created by corey on 3/11/23.
//

#ifndef DATACHANNELS_DATACHANNELCOMMON_H
#define DATACHANNELS_DATACHANNELCOMMON_H

#include <string>

struct IceCandidate {
    IceCandidate(std::string candidate, std::string sdpMid, int sdpMLineIndex)
            : candidate(std::move(candidate)), sdpMid(std::move(sdpMid)), sdpMLineIndex(sdpMLineIndex) {}

    std::string candidate;
    std::string sdpMid;
    int sdpMLineIndex;
};


#endif //DATACHANNELS_DATACHANNELCOMMON_H
