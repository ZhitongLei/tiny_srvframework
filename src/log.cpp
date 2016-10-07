// File Name: log.cpp
// Author: lei
// Created Time: 2016-10-07 23:11:51

#include "log.h"

using namespace std;

namespace tiny_srv {

static const kBufferNumber = 10;

Logger::Logger(const std::string &log_directory, const std::string &base_name, 
               uint32_t buffer_length, uint32_t flush_interval) 
    :m_log_directory(log_directory),
     m_base_name(base_name), 
     m_buffer_length(buffer_length), 
     m_flush_interval(flush_interval) {
    Initialize();
}

void Logger::Initialize() {
    for(uint32_t i = 0; i < kBufferNumber; ++i) {
        shared_ptr<Buffer> buffer_ptr(new Buffer(m_buffer_length));
        m_empty_buffers.Push(buffer_ptr);
    }
    m_curr_buffer_ptr = m_empty_buffers.Pop();
}

}  // end of namespace
