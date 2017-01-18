#ifndef _TINY_SRVFRAMEWORK_LOG_H_
#define _TINY_SRVFRAMEWORK_LOG_H_

#include <iostream>
#include <stdint.h>
#include <string.h>
#include <time.h>   // time_t, struct tm, time, localtime, strftime
#include <string>
#include <thread>
#include <mutex>
#include <memory>   // std::shared_ptr
#include <atomic>
#include <condition_variable> 
#include <deque>
//#include "blocking_queue.h"

namespace tiny_srv {

class Buffer {
public:
    Buffer(uint32_t length)
        :m_length(length), m_used(0) {
        m_buffer = new char[length];
        m_curr_ptr = m_buffer;
    }

    ~Buffer() {
        if(NULL != m_buffer) { 
            delete []m_buffer; 
            m_buffer = NULL;
        }
        m_curr_ptr = NULL;
    }

    Buffer(const Buffer& buffer) {
        // std::cout << "copying" << std::endl;
        m_buffer = new char[buffer.m_length];
        ::memcpy(m_buffer, buffer.m_buffer, buffer.m_used);
        m_curr_ptr = buffer.m_curr_ptr;
        m_length = buffer.m_length;
        m_used = buffer.m_used;
    }

    Buffer& operator=(const Buffer& rhs) {
        if(this != &rhs) {
            if(m_length != rhs.m_length) { delete []m_buffer; }
            m_buffer = new char[rhs.m_length];
            ::memcpy(m_buffer, rhs.m_buffer, rhs.m_used);
            m_curr_ptr = rhs.m_curr_ptr;
            m_length = rhs.m_length;
            m_used = rhs.m_used;
        }
        return *this;
    }

    bool Append(const char* buffer, size_t length) {
        if(space_left() < length) { return false; }
        memcpy(m_curr_ptr, buffer, length);
        m_curr_ptr += length;
        m_used += static_cast<uint32_t>(length);
        return true;
    }
    
    bool Append(const std::string &buffer) {
        return Append(buffer.c_str(), buffer.length());
    }

    void Clear() {
        ::bzero(m_buffer, m_length);
        Reset();
    }

    void Reset() {
        m_curr_ptr = m_buffer;
        m_used = 0;
    }

    const char *data() const { return m_buffer; }
    std::string toString() const { return std::string(m_buffer, m_used); }

    uint32_t length() const { return m_length; }
    uint32_t used() const { return m_used; }
    uint32_t space_left() const { return m_length - m_used; }

private:
    char *m_buffer;
    char *m_curr_ptr;
    uint32_t m_length;
    uint32_t m_used;
};

class Logger {
public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        LOG_LEVELS_NUM
    };

    static Logger* instance() {
        pthread_once(&m_ponce, &Logger::SingletonInit);
        return m_singleton;
    }

    // log name format: /log/directory/base_name-YYYYMMDDHH.log
    // e.g. logfile_test-2016100712.log
    Logger(const std::string &log_directory, const std::string &base_name, 
           uint32_t buffer_size_kb, uint32_t flush_interval);

    Logger();
    ~Logger();

    bool Initialize();
    bool Initialize(const std::string &log_directory, const std::string &base_name,
                    uint32_t buffer_size_kb, uint32_t flush_interval);
    void Start();
    void Stop();

    bool Write(LogLevel log_level, const char *file, const char *function, const char* format, ...);
    //std::string current_logfile_name(const time_t *now);
    std::string current_logfile_name(const struct tm *time_info); 


private:
    // noncopyable
    Logger(const Logger& logger);
    Logger& operator=(const Logger& rhs);

    bool RollFile();
    void Flush();

    static void SingletonInit() {
        m_singleton = new Logger();
    }

private:
    std::string m_log_directory;
    std::string m_base_name;
    uint32_t m_buffer_size_kb;
    uint32_t m_flush_interval;
    std::atomic<bool> m_running;
    std::mutex m_mutex;
    std::unique_ptr<Buffer> m_curr_buffer_ptr;    
    std::deque<std::unique_ptr<Buffer>> m_empty_buffers;
    std::deque<std::unique_ptr<Buffer>> m_full_buffers;

    FILE *m_fp;
    std::atomic<bool> m_roll_file;
    time_t m_last_roll_time;
    std::thread m_thread;
    std::condition_variable m_not_empty_cond;

    // for singleton
    static pthread_once_t m_ponce;
    static Logger *m_singleton;
};


#define LOG_TRACE(format, ...) Logger::instance()->Write(Logger::TRACE, __FILE__, __func__, format, __VA_ARGS__)
#define LOG_DEBUG(format, ...) Logger::instance()->Write(Logger::DEBUG, __FILE__, __func__, format, __VA_ARGS__)
#define LOG_INFO(format, ...) Logger::instance()->Write(Logger::INFO, __FILE__, __func__, format, __VA_ARGS__)
#define LOG_WARN(format, ...) Logger::instance()->Write(Logger::WARN, __FILE__, __func__,  format, __VA_ARGS__)
#define LOG_ERROR(format, ...) Logger::instance()->Write(Logger::ERROR, __FILE__, __func__, format, __VA_ARGS__)
#define LOG_FATAL(format, ...) Logger::instance()->Write(Logger::FATAL, __FILE__, __func__, format, __VA_ARGS__)

}  // end of namespace
#endif

