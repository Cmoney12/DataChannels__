//
// Created by corey on 3/11/23.
//
#include <utility>

#include "../include/IceTransport.h"

IceTransport::IceTransport(Configuration &config, candidate_callback candidate_cb)
: config_(config), candidate_callback_(std::move(candidate_cb)), agent_(nullptr, nullptr), loop(nullptr, nullptr) {}

IceTransport::~IceTransport() {}

void IceTransport::init() {
    logger = get_logger("logger");

    int log_flags = G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION;
    g_log_set_handler(nullptr, (GLogLevelFlags)log_flags, nice_log_handler, this);

    this->loop = std::unique_ptr<GMainLoop, void (*)(GMainLoop *)>(g_main_loop_new(nullptr, FALSE), g_main_loop_unref);
    if (!this->loop) {
        SPDLOG_TRACE(logger, "Failed to initialize GMainLoop");
    }


}

void IceTransport::nice_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message,
                                    gpointer user_data) {
    IceTransport *ice = (IceTransport *)user_data;
    ice->log_message(message);
}

void IceTransport::log_message(const gchar *message) { SPDLOG_TRACE(logger, "libnice: {}", message); }





