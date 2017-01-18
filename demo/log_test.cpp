#include "log.h"
#include <time.h>   // time_t, struct tm, time, localtime, strftime
#include <thread>
#include <vector>
#include <sys/time.h>
#include <string>
#include <mutex>
#include <atomic>
#include <iomanip>      // std::setprecision

using namespace tiny_srv;
using namespace std;

std::atomic<int64_t> g_start_time(INT64_MAX);
std::atomic<int64_t> g_finish_time(0);
/*
int64_t g_start_time(INT64_MAX);
int64_t g_finish_time(0);
std::mutex g_mutex;
*/

static int64_t ustime(void){
    struct timeval tv;
    int64_t ust;

    gettimeofday(&tv, NULL);
    ust = ((long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}

static int64_t mstime(void){
    struct timeval tv;
    long long mst;

    gettimeofday(&tv, NULL);
    mst = ((long long)tv.tv_sec)*1000;
    mst += tv.tv_usec/1000;
    return mst;
}

/*
void WriteLogFunc(Logger &logger, int thread_id) {
    for(int i = 0; i < 100000; ++i) {
        logger.Write(Logger::DEBUG, __FILE__, __func__, "I'm thread-%d | %d %lf %s", thread_id, i, 4323.3, "test loglogfdsfsd fsdfsd");
    }
}
*/
void WriteLogFunc(int thread_id, int log_num) {
    //const char *msg = "test loglogfdsfsd fsdfsd 45450 78945454545 fdsfdsfdspqoeuwnkcxlsjhahvhidafieerweroafdafdsffdsfdsiioiii d038fhnxasweuu38cbncxv547477";
    const char *msg = "okofdsfdsrekolldfsjas dsadasdas";
    int64_t start_time = ustime();
    for(int i = 0; i < log_num; ++i) {
        LOG_DEBUG("I'm thread-%d | %d %lf %s", thread_id, i, 4323.3, msg);
    }
    int64_t finish_time = ustime();

    {
        //std::lock_guard<std::mutex> lock(g_mutex);
        if(start_time < g_start_time) { g_start_time = start_time; }
        if(g_finish_time < finish_time) { g_finish_time = finish_time; }
    }
}

int main(int argc, char **argv)
{
    string log_path("/home/kenlei/workplace/linux/net/tiny_srvframework_20161230/demo");
    //string log_path(argv[1]);    
    Logger::instance()->Initialize(log_path, "logfile_test", 1024, 3);
    Logger::instance()->Start();

    vector<std::thread> threads;
    uint32_t thread_num = 10;
    int log_per_thread = 100000;
    for(uint32_t i = 0; i < thread_num; ++i){
        threads.push_back(std::thread(WriteLogFunc, i, log_per_thread));
    }  
    for(auto &t : threads){ t.join(); }

    uint32_t write_logs = thread_num * log_per_thread;
    int64_t total_time = g_finish_time - g_start_time;
    uint32_t time_cost_sec = total_time/1000000;
    double ops = write_logs*1.0/time_cost_sec;

    cout << "log benchmark" << endl;
    cout << "write logs           : " << write_logs << endl;
    cout << "time cost            : " << total_time/1000 
         << "ms (" << std::setprecision(2) << total_time*1.0/write_logs << " micros/op)" << endl;
    cout << "operation per second : " << std::setprecision(5) << ops << " million/s" << endl;
    //cout << logger.current_logfile_name(&t) << endl;
    Logger::instance()->Stop();
    return 0;
}
