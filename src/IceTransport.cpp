//
// Created by corey on 3/11/23.
//
#include <utility>

#include "../include/IceTransport.h"

IceTransport::IceTransport(Configuration &config, candidate_callback  candidate_cb)
: config_(config), candidate_callback_(std::move(candidate_cb)) {}

