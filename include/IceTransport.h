//
// Created by corey on 3/11/23.
//

#ifndef DATACHANNELS_ICETRANSPORT_H
#define DATACHANNELS_ICETRANSPORT_H


extern "C" {
#include <agent.h>
#include <netdb.h>
};
#include <functional>
#include <memory>
#include <thread>
#include "DataChannelCommon.h"
#include "Logger.h"

using candidate_callback = std::function<void(const IceCandidate &candidate)>;

class IceTransport {
public:
    IceTransport(Configuration& config, candidate_callback  candidate_cb);

    ~IceTransport();

    void init();

    void log_message(const gchar *message);


private:
    static void nice_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);

    static void candidate_gathering_done(NiceAgent *agent, guint stream_id, gpointer user_data);

    static void component_state_changed(NiceAgent *agent, guint stream_id, guint component_id, guint state, gpointer user_data);

    static void new_local_candidate(NiceAgent *agent, NiceCandidate *candidate, gpointer user_data);

    static void new_selected_pair(NiceAgent *agent, guint stream_id, guint component_id, NiceCandidate *lcandidate, NiceCandidate *rcandidate,
                                  gpointer user_data);

    static void data_received(NiceAgent *agent, guint stream_id, guint component_id,
                              guint len, gchar *buf, gpointer user_data);

    Configuration config_;
    candidate_callback candidate_callback_;
    std::unique_ptr<NiceAgent, void (*)(gpointer)> agent_;
    std::unique_ptr<GMainLoop, void (*)(GMainLoop *)> loop;
    std::uint32_t stream_id;
    std::thread main_loop_thread;
    std::shared_ptr<Logger> logger;
};

#endif //DATACHANNELS_ICETRANSPORT_H
