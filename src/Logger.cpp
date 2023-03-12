//
// Created by corey on 3/12/23.
//

#include "../include/Logger.h"

std::shared_ptr<Logger> get_logger(const std::string &logger_name) {
    auto logger = spdlog::get(logger_name);
    //spdlog::set_level(spdlog::level::trace); // Set global log level to debug

    if (logger) {
        return logger;
    }
    return spdlog::stdout_color_mt(logger_name);
}