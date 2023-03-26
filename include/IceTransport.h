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
#include <utility>
#include <sstream>
#include "DataChannelCommon.h"
#include "DTLSTransport.h"
#include "Logger.h"
#include "../DataStructures/ThreadPool/ThreadPool.h"
#include "../DataStructures/ThreadSafeMemoryPool/MemoryPool.h"

using candidate_callback = std::function<void(const std::string& string)>;

class IceTransport {
public:
    IceTransport(Configuration& config,
                 candidate_callback candidate_cb,
                 std::shared_ptr<DTLSTransport> dtls_transport,
                 std::shared_ptr<ThreadPool> thread_pool);

    ~IceTransport();

    void init();

    void log_message(const gchar *message);

    void on_candidate_gathering_done();

    void on_state_change(std::uint32_t stream_id, std::uint32_t component_id, std::uint32_t state);

    void on_candidate(std::string& candidate);

    void parse_remote_sdp(std::string sdp);

    static void replace_all(std::string &s, const std::string &search, const std::string &replace);

    std::string generate_local_sdp();

    void set_remote_ice_candidate(std::string& candidate);

    void set_remote_ice_candidates(std::vector<std::string>& candidates);


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
    std::shared_ptr<DTLSTransport> dtls_transport_;
    std::uint32_t stream_id;
    std::thread main_loop_thread;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<ThreadPool> thread_pool_;
    std::shared_ptr<MemoryPool<std::uint8_t>> memory_pool_;
};

#endif //DATACHANNELS_ICETRANSPORT_H
