//
// Created by corey on 3/12/23.
//

#ifndef DATACHANNELS_LOGGER_H
#define DATACHANNELS_LOGGER_H

#include <memory>
#include <string>

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"
#include <spdlog/fmt/ostr.h>

typedef spdlog::logger Logger;


std::shared_ptr<Logger> get_logger(const std::string &logger_name);

#endif //DATACHANNELS_LOGGER_H
