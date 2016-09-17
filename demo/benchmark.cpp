#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>

#include "udp_socket.h"
#include "host_addr.h"

using namespace tiny_srv;
using namespace std;

class BenchmarkConfig{
public:
    string host;
    uint16_t port;
    uint32_t concurrency;
    uint32_t request;
};

class BenchmarkResult{
public:
   BenchmarkResult(uint32_t request_num)
       :request(request_num),
        request_finished(0),
		start_time(0),
		finish_time(0)
   {}

   uint32_t request;
   uint32_t request_finished;
   int64_t start_time;
   int64_t finish_time;
   vector<int64_t> latency;
};
static vector<std::shared_ptr<BenchmarkResult>> kResult;
static std::mutex kResultMutex;

void Usage()
{
    fprintf(stderr, "Usage: benchmark -h <host> -p <port> -[t/u] -c <concurrency> -n <request_num>\n");
}

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

// unit: millisecond
static int64_t time_diff(struct timeval &beg, struct timeval &end)
{
    return (end.tv_sec - beg.tv_sec)*1000 + (end.tv_usec - beg.tv_usec)/1000;
}

void UdpBenchmark(const BenchmarkConfig &config)
{
    std::shared_ptr<BenchmarkResult> benchmark_result(new BenchmarkResult(config.request));
    char buffer[4096] = {0};
    int64_t start_time = 0, finish_time = 0;
    int64_t request_time = 0, response_time = 0;
    string str("benchmark testing");

	udp_socket us;
	us.create();
	host_addr ha(config.host, config.port);
    benchmark_result->start_time = mstime();
    for(uint32_t i = 0; i < config.request; ++i){
        //fprintf(stdout, "round %u, send:%s, length:%u\n", i, str.c_str(), str.length());
        request_time = ustime();
	    us.sendto(str.c_str(), str.length(), ha);
        int32_t n = us.recvfrom(buffer, sizeof(buffer), ha);
        //fprintf(stdout, "response: %s\n", buffer);
        response_time = ustime();
        if(n > 0){ 
            benchmark_result->latency.push_back(response_time-request_time);
            benchmark_result->request_finished++;
        }
    }
    benchmark_result->finish_time = mstime();

    std::lock_guard<std::mutex> sync_lock(kResultMutex);
    kResult.push_back(benchmark_result);
}

void ShowLatencyReport(const BenchmarkConfig &config)
{
    uint32_t request = 0;
    uint32_t request_finished = 0;
    int64_t start_time = INT64_MAX, finish_time = 0;
    vector<int64_t> latency;
    for(const auto &bs : kResult){
        request += bs->request;
        request_finished += bs->request_finished;
        latency.insert(latency.end(), bs->latency.begin(), bs->latency.end());
        start_time = std::min(start_time, bs->start_time);
        finish_time = std::max(finish_time, bs->finish_time);
    }
    double total_latency = (double)(finish_time - start_time)/1000;
    sort(latency.begin(), latency.end());
    
    fprintf(stdout, "Concurrency: %u\n", config.concurrency);
    fprintf(stdout, "Requests: %u\n", request);
    fprintf(stdout, "Request_finished: %u\n", request_finished);
    fprintf(stdout, "Total latency: %.2lf seconds (QPS: %.0lf)\n", total_latency, (double)request_finished/total_latency);
    fprintf(stdout, "Percentage of the requests served within a certain time:\n");
    int64_t current_latency = 0;
    double percent = 0.0;
    for(size_t i = 0; i < latency.size(); ++i){
        if(latency[i]/1000 != current_latency || i == latency.size()-1){
            if(i > 0){
                uint32_t count = i;
                if(i == latency.size()-1){ count++; }
                percent = count*100.0/request_finished;
                fprintf(stdout, "%u (%.2lf%%) <= %lu milliseconds\n", count, percent, current_latency+1);
            }
            current_latency = latency[i]/1000;
        }
    }
}

int main(int argc, char **argv)
{
    int32_t ch = 0;
    bool tcp_mode = false, udp_mode = false;
    BenchmarkConfig config;
    while(-1 != (ch = getopt(argc, argv, "tuh:p:c:n:"))){
        switch(ch){
            case 'h':
                config.host.assign(optarg);
                break;
            case 'p':
                config.port = atoi(optarg);
                break;
            case 't':
                tcp_mode = true;
                break;
            case 'u':
                udp_mode = true;
                break;
            case 'c':
                config.concurrency = atoi(optarg);
                break;
            case 'n':
                config.request = atoi(optarg);
                break;
            default:
                Usage();
                exit(0);
        }
    }

    if(!tcp_mode && !udp_mode){
        fprintf(stderr, "use -t(tcp) or -u(udp) to indicate protocol type\n");
        Usage();
        exit(-1);
    }

    string mode = tcp_mode ? "tcp" : "udp";
    if(udp_mode){
        vector<std::thread> threads;
        for(uint32_t i = 0; i < config.concurrency; ++i){
            threads.push_back(std::thread(UdpBenchmark, std::cref(config)));
        }  
        for(auto &t : threads){ t.join(); }
    }

    ShowLatencyReport(config);
    return 0;
}
