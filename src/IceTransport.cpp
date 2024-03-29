//
// Created by corey on 3/11/23.
//

#include <utility>

#include "../include/IceTransport.h"

IceTransport::IceTransport(Configuration &config,
                           candidate_callback candidate_cb,
                           std::shared_ptr<DTLSTransport> dtls_transport,
                           std::shared_ptr<ThreadPool> thread_pool): config_(config),
                           candidate_callback_(std::move(candidate_cb)), agent_(nullptr, nullptr), loop(nullptr, nullptr),
                           dtls_transport_(dtls_transport),
                           thread_pool_(thread_pool) {}

IceTransport::~IceTransport() {}

void IceTransport::init() {
    logger = get_logger("logger");

    int log_flags = G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION;
    g_log_set_handler(nullptr, (GLogLevelFlags)log_flags, nice_log_handler, this);

    loop = std::unique_ptr<GMainLoop, void (*)(GMainLoop *)>(g_main_loop_new(nullptr, FALSE), g_main_loop_unref);

    if (!loop) {
        SPDLOG_TRACE(logger, "Failed to initialize GMainLoop");
    }

    agent_ = std::unique_ptr<NiceAgent, decltype(&g_object_unref)>(nice_agent_new(g_main_loop_get_context(loop.get()), NICE_COMPATIBILITY_RFC5245),
                                                                        g_object_unref);
    if (!agent_) {
        SPDLOG_TRACE(logger, "Failed to initialize nice agent");
    }

    main_loop_thread = std::thread(g_main_loop_run, this->loop.get());

    g_object_set(G_OBJECT(agent_.get()), "upnp", FALSE, nullptr);
    g_object_set(G_OBJECT(agent_.get()), "controlling-mode", 0, nullptr);

    if (config_.ice_servers.size() > 1) {
        throw std::invalid_argument("Only up to one ICE server is currently supported");
    }

    for (const auto& ice_server : config_.ice_servers) {
        struct hostent *stun_host = gethostbyname(ice_server.hostname.c_str());
        if (stun_host == nullptr) {
            logger->warn("Failed to lookup host for server: {}", ice_server.hostname);
        } else {
            in_addr *address = (in_addr *)stun_host->h_addr;
            const char *ip_address = inet_ntoa(*address);

            g_object_set(G_OBJECT(agent_.get()), "stun-server", ip_address, NULL);
        }

        if (ice_server.port > 0) {
            g_object_set(G_OBJECT(agent_.get()), "stun-server-port", ice_server.port, NULL);
        } else {
            logger->error("stun port empty");
        }
    }

    g_signal_connect(G_OBJECT(agent_.get()), "candidate-gathering-done", G_CALLBACK(candidate_gathering_done), this);
    g_signal_connect(G_OBJECT(agent_.get()), "component-state-changed", G_CALLBACK(component_state_changed), this);
    g_signal_connect(G_OBJECT(agent_.get()), "new-candidate-full", G_CALLBACK(new_local_candidate), this);
    g_signal_connect(G_OBJECT(agent_.get()), "new-selected-pair", G_CALLBACK(new_selected_pair), this);

    stream_id = nice_agent_add_stream(agent_.get(), 1);
    if (this->stream_id == 0) {
        return;
    }

    nice_agent_set_stream_name(agent_.get(), stream_id, "application");

    if (!config_.ice_ufrag.empty() && !config_.ice_pwd.empty()) {
        //nice_agent_set_local_credentials(agent_.get(), this->stream_id, config_.ice_ufrag.c_str(), config_.ice_pwd.c_str());
    }

    if (config_.ice_port_range.first != 0 || config_.ice_port_range.second != 0) {
        nice_agent_set_port_range(agent_.get(), stream_id, 1, config_.ice_port_range.first, config_.ice_port_range.second);
    }

    nice_agent_attach_recv(agent_.get(), stream_id, 1, g_main_loop_get_context(loop.get()), data_received, this);
}



void IceTransport::nice_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message,
                                    gpointer user_data) {
    auto *ice = static_cast<IceTransport*>(user_data);
    ice->log_message(message);
}

void IceTransport::log_message(const gchar *message) { SPDLOG_TRACE(logger, "libnice: {}", message); }

void IceTransport::candidate_gathering_done(NiceAgent *agent, guint stream_id, gpointer user_data) {
    auto *ice = static_cast<IceTransport*>(user_data);
    ice->on_candidate_gathering_done();
}

void IceTransport::on_candidate_gathering_done() {
    SPDLOG_DEBUG(logger, "ICE: candidate gathering done");
    std::string empty_candidate;
    candidate_callback_(empty_candidate);
}

void IceTransport::component_state_changed(NiceAgent *agent, guint stream_id, guint component_id, guint state,
                                           gpointer user_data) {
    auto *ice = static_cast<IceTransport*>(user_data);
    ice->on_state_change(stream_id, component_id, state);
}

void IceTransport::on_state_change(std::uint32_t stream_id_, std::uint32_t component_id, std::uint32_t state) {
    switch (state) {
        case (NICE_COMPONENT_STATE_DISCONNECTED):
            SPDLOG_TRACE(logger, "ICE: DISCONNECTED");
            break;
        case (NICE_COMPONENT_STATE_GATHERING):
            SPDLOG_TRACE(logger, "ICE: GATHERING");
            break;
        case (NICE_COMPONENT_STATE_CONNECTING):
            SPDLOG_TRACE(logger, "ICE: CONNECTING");
            break;
        case (NICE_COMPONENT_STATE_CONNECTED):
            SPDLOG_TRACE(logger, "ICE: CONNECTED");
            break;
        case (NICE_COMPONENT_STATE_READY):
            SPDLOG_TRACE(logger, "ICE: READY");
            // TODO add a callback
            //this->OnIceReady();
            break;
        case (NICE_COMPONENT_STATE_FAILED):
            SPDLOG_TRACE(logger, "ICE FAILED: stream_id={} - component_id={}", stream_id, component_id);
            break;
        default:
            SPDLOG_TRACE(logger, "ICE: Unknown state: {}", state);
            break;
    }
}

void IceTransport::new_local_candidate(NiceAgent *agent, NiceCandidate *candidate, gpointer user_data) {
    auto *ice = static_cast<IceTransport*>(user_data);
    gchar *cand = nice_agent_generate_local_candidate_sdp(agent, candidate);
    std::string cand_str(cand);
    ice->on_candidate(cand_str);
    g_free(cand);
}

void IceTransport::on_candidate(std::string& candidate) { candidate_callback_(candidate); }


void IceTransport::new_selected_pair(NiceAgent *agent, guint stream_id, guint component_id, NiceCandidate *lcandidate,
                                     NiceCandidate *rcandidate, gpointer user_data) {
    //error
    auto logger = get_logger("logger");
    logger->error("ICE: new selected pair");
    auto *ice = static_cast<IceTransport*>(user_data);
    //nice->OnSelectedPair();

}

void IceTransport::data_received(NiceAgent *agent, guint stream_id, guint component_id, guint len, gchar *buf,
                                 gpointer user_data) {
    // TODO Data Received Callback
    auto *ice = static_cast<IceTransport*>(user_data);
    //ice->on_data_received((const uint8_t *)buf, len);
}

void IceTransport::replace_all(std::string &s, const std::string &search, const std::string &replace) {
    size_t pos = 0;
    while ((pos = s.find(search, pos)) != std::string::npos) {
        s.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

void IceTransport::parse_remote_sdp(std::string sdp) {
    std::string remote_sdp = std::move(sdp);

    replace_all(remote_sdp, "\r\n", "\n");

    int rc = nice_agent_parse_remote_sdp(this->agent_.get(), remote_sdp.c_str());

    if (rc < 0) {
        throw std::runtime_error("ParseRemoteSDP: " + std::string(strerror(rc)));
    } else {
        logger->info("ICE: Added {} Candidates", rc);
    }

    if (!nice_agent_gather_candidates(agent_.get(), stream_id)) {
        throw std::runtime_error("ParseRemoteSDP: Error gathering candidates!");
    }
}

std::string IceTransport::generate_local_sdp() {
    std::stringstream nice_sdp;
    std::stringstream result;
    std::string line;

    gchar *raw_sdp = nice_agent_generate_local_sdp(agent_.get());
    nice_sdp << raw_sdp;

    while (std::getline(nice_sdp, line)) {
        if (g_str_has_prefix(line.c_str(), "a=ice-ufrag:") || g_str_has_prefix(line.c_str(), "a=ice-pwd:")) {
            result << line << "\r\n";
        }
    }
    g_free(raw_sdp);
    return result.str();
}

void IceTransport::set_remote_ice_candidate(std::string &candidate) {
    GSList *list = nullptr;
    NiceCandidate *rcand = nice_agent_parse_remote_candidate_sdp(agent_.get(), stream_id, candidate.c_str());

    if (rcand == nullptr) {
        SPDLOG_TRACE(logger, "failed to parse remote candidate");
        return;
    }
    list = g_slist_append(list, rcand);

    bool success = (nice_agent_set_remote_candidates(agent_.get(), stream_id, 1, list) > 0);

    g_slist_free_full(list, (GDestroyNotify)&nice_candidate_free);
}

void IceTransport::set_remote_ice_candidates(std::vector<std::string> &candidate_sdps) {
    GSList *list = nullptr;

    for (const auto& candidate_sdp : candidate_sdps) {
        NiceCandidate *rcand = nice_agent_parse_remote_candidate_sdp(agent_.get(), stream_id, candidate_sdp.c_str());

        if (rcand == nullptr) {
            SPDLOG_TRACE(logger, "failed to parse remote candidate");
            return;
        }
        list = g_slist_append(list, rcand);
    }

    bool success = (nice_agent_set_remote_candidates(this->agent_.get(), stream_id, 1, list) > 0);

    g_slist_free_full(list, (GDestroyNotify)&nice_candidate_free);
}

bool IceTransport::send(std::uint8_t *data, std::size_t size) {
    int result = 0;
    result = nice_agent_send(agent_.get(), stream_id, 1, size, reinterpret_cast<const char*>(data));
    // probably should add a message if succeed
    return result;
}

