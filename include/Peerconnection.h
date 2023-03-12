//
// Created by corey on 3/11/23.
//

#ifndef DATACHANNELS_PEERCONNECTION_H
#define DATACHANNELS_PEERCONNECTION_H


#include <functional>
#include <memory>
#include "DataChannelCommon.h"
#include "IceTransport.h"

typedef std::function<void(IceCandidate const&)> local_candidate_cb;

class PeerConnection : std::enable_shared_from_this<std::shared_ptr<PeerConnection>> {
public:
    PeerConnection(Configuration &config);


private:
    local_candidate_cb local_candidate_cb_;

};


#endif //DATACHANNELS_PEERCONNECTION_H
