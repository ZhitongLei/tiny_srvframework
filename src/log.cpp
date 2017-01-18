// File Name: log.cpp
// Author: lei
// Created Time: 2016-10-07 23:11:51

#include "log.h"
#include <stdarg.h>
#include <chrono>  // std::chrono::seconds
#include <assert.h>
#include <iterator>

using namespace std;

namespace tiny_srv {

pthread_once_t Logger::m_ponce = PTHREAD_ONCE_INIT;
Logger* Logger::m_singleton = NULL;

static const uint32_t kBufferNumber = 10;
static const uint32_t kRollIntervalSec = 60 * 60;

#define MAX_LOG_CONTENT_LEN 2048
__thread char time_buf[32];
__thread char one_log_buff[MAX_LOG_CONTENT_LEN];
__thread time_t last_second;

static const char* LogLevelName[Logger::LOG_LEVELS_NUM] =
{
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL "
};

Logger::Logger(const std::string &log_directory, const std::string &base_name, 
               uint32_t buffer_size_kb, uint32_t flush_interval) 
    :m_log_directory(log_directory),
     m_base_name(base_name), 
     m_buffer_size_kb(buffer_size_kb), 
     m_flush_interval(flush_interval),
     m_running(false),
     m_fp(NULL),
     m_roll_file(false) {
    Initialize();
}

Logger::Logger()
    :m_buffer_size_kb(0), 
     m_flush_interval(0),
     m_running(false),
     m_fp(NULL),
     m_roll_file(false),
     m_last_roll_time(0) {
}

Logger::~Logger() {
    if(m_running) { Stop(); }
    if(NULL != m_fp) { ::fclose(m_fp); }
}

bool Logger::Initialize(const std::string &log_directory, const std::string &base_name,
                        uint32_t buffer_size_kb, uint32_t flush_interval) {
    m_log_directory = log_directory;
    m_base_name = base_name;
    m_buffer_size_kb = buffer_size_kb;
    m_flush_interval = flush_interval;

    for(uint32_t i = 0; i < kBufferNumber; ++i) {
        unique_ptr<Buffer> buffer_ptr(new Buffer(m_buffer_size_kb*1024));
        m_empty_buffers.push_back(std::move(buffer_ptr));
    }
    m_curr_buffer_ptr = std::move(m_empty_buffers.front());
    m_empty_buffers.pop_front();
    if(!RollFile()) { return false; }
    return true;
}

bool Logger::Initialize() {
    for(uint32_t i = 0; i < kBufferNumber; ++i) {
        unique_ptr<Buffer> buffer_ptr(new Buffer(m_buffer_size_kb*1024));
        m_empty_buffers.push_back(std::move(buffer_ptr));
    }
    m_curr_buffer_ptr = std::move(m_empty_buffers.front());
    m_empty_buffers.pop_front();
    m_last_roll_time = ::time(NULL);

    struct tm time_info;
    localtime_r(&m_last_roll_time, &time_info);
    string log_full_name(m_log_directory);
    log_full_name.append("/");
    log_full_name.append(current_logfile_name(&time_info));

    m_fp = ::fopen(log_full_name.c_str(), "a");
    if(NULL == m_fp) { return false; }

    Start();
    return true;
}

bool Logger::Write(LogLevel log_level, const char *file, const char *function, const char* format, ...) {
    time_t now = ::time(NULL);
    double write_interval = difftime(now, last_second);
    if(write_interval >= 1.0) {
        struct tm time_info;
        localtime_r(&now, &time_info);
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S ", &time_info);
        last_second = now;
        //if(write_interval >= 3600.0) { m_roll_file = true; }
        //
    }

    va_list args;
    va_start(args, format);
    int ret = vsnprintf(one_log_buff, MAX_LOG_CONTENT_LEN, format, args);  
    // TODO: if ret == MAX_LOG_CONTENT_LEN, output some message
    va_end(args);

    string record(time_buf, 20);
    record.append(LogLevelName[log_level], 6);
    record.append(file); record.append(" ");
    record.append(function); record.append(" ");
    record.append(one_log_buff, ret);
    record.append("\n");
    
    /*
    bool reach_flush_time = false;
    if(difftime(now, m_last_roll_time) >= m_flush_interval && 
       m_curr_buffer_ptr->used() > 0) { reach_flush_time = true; }
    bool curr_buffer_full = false;
    if(m_curr_buffer_ptr->space_left() < record.length()) { 
        curr_buffer_full = true; 
    }*/

    // In both cases, we need to write log entry into new empty buffer
    // 1. current buffer space left < record length
    // 2. is time to roll file and current buffer is not empty
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_curr_buffer_ptr->space_left() < record.length()) {
           //(m_roll_file && m_curr_buffer_ptr->used() > 0)) {
            if(m_empty_buffers.empty()) {
                // No empty buffers, discard log 
                fprintf(stderr, "%s [ERROR] write log error, no empty buffer\n", time_buf);
                return false;
            }

            // When 80% of buffer is used, alert
            if(m_full_buffers.size()*1.0 / kBufferNumber >= 0.8) {
                fprintf(stderr, "%s [WARN] write log error, no empty buffer\n", time_buf);
            }

            m_full_buffers.push_back(std::move(m_curr_buffer_ptr));
            m_curr_buffer_ptr = std::move(m_empty_buffers.front());
            m_empty_buffers.pop_front();
            m_not_empty_cond.notify_all();
        }
        assert(record.length() <= m_curr_buffer_ptr->space_left());
        m_curr_buffer_ptr->Append(record);
    }
    return true;
}

void Logger::Start() {
    m_running = true;
    m_thread = std::thread(&Logger::Flush, this);
}

void Logger::Stop() {
    m_running = false;
    m_not_empty_cond.notify_all();
    m_thread.join();

    // flush rest buffer
    if(m_curr_buffer_ptr->used() > 0) {
        fwrite(m_curr_buffer_ptr->data(), 1, m_curr_buffer_ptr->used(), m_fp);
        m_curr_buffer_ptr->Reset();
        fprintf(stderr, "flush rest buffer done\n");
    }
}


bool Logger::RollFile() {
    time_t now = ::time(NULL);
    if(now / kRollIntervalSec == m_last_roll_time / kRollIntervalSec) { 
        return true; 
    }

    if(NULL != m_fp) { ::fclose(m_fp); }

    m_last_roll_time = now;
    struct tm time_info;
    localtime_r(&m_last_roll_time, &time_info);
    string log_full_name(m_log_directory);
    log_full_name.append("/");
    log_full_name.append(current_logfile_name(&time_info));

    m_fp = ::fopen(log_full_name.c_str(), "a");
    if(NULL == m_fp) { return false; }
    return true;
}


// difference between unique_lock and lock_guard
// http://stackoverflow.com/questions/20516773/stdunique-lockstdmutex-or-stdlock-guardstdmutex
void Logger::Flush() {
    // if time up to 3s or m_full_buffers is not empty, write file       
    std::deque<std::unique_ptr<Buffer>> buffers_to_write;
    std::deque<std::unique_ptr<Buffer>> buffers_released;

    while(m_running) {
        buffers_to_write.clear();

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_not_empty_cond.wait_for(lock, std::chrono::seconds(m_flush_interval), [this]{return !m_full_buffers.empty();});
            m_empty_buffers.insert(m_empty_buffers.end(), 
                                   std::make_move_iterator(buffers_released.begin()), 
                                   std::make_move_iterator(buffers_released.end()));
            buffers_released.clear();

            if(m_full_buffers.empty() && m_curr_buffer_ptr->used() > 0) {
                assert(!m_empty_buffers.empty());
                m_full_buffers.push_back(std::move(m_curr_buffer_ptr));
                m_curr_buffer_ptr = std::move(m_empty_buffers.front());
                m_empty_buffers.pop_front();
            }
            m_full_buffers.swap(buffers_to_write);
        }

        std::unique_ptr<Buffer> buffer;
        while(!buffers_to_write.empty()) {
            RollFile();
            buffer = std::move(buffers_to_write.front());
            buffers_to_write.pop_front();
            fwrite(buffer->data(), 1, buffer->used(), m_fp);
            buffer->Reset();
            buffers_released.push_back(std::move(buffer));
        } // end while(!buffers_to_write ...)
    }
}

string Logger::current_logfile_name(const struct tm *time_info) {
    string file_name(m_base_name);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "-%Y%m%d%H.log", time_info);
    file_name.append(time_buf);
    return file_name;
}

}  // end of namespace
