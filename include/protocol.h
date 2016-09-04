#ifndef _TINY_SRVFRAMEWORK_PROTOCOL_H_
#define _TINY_SRVFRAMEWORK_PROTOCOL_H_

#include <stdint.h>
#include <string>
#include <arpa/inet.h>

namespace tiny_srv{

// begin and end flag for package
const char SRV_STX = 0x02;
const char SRV_ETX = 0x03;

const uint32_t PROTOCOL_VERSION = 1;

const uint32_t PROTOCOL_HEAD_RESERVED_LEN = 64;

const uint32_t MAX_PACKAGE_LENGTH = 1024 * 1024;


#pragma pack(push, 1)

struct ProtocolHead
{
public:
    ProtocolHead()
        :client_ip(0), 
         client_port(0),
         sequence_num(0),
         package_length(0),
         send_time(0),
         version(0)
    {}         

    uint32_t client_ip;
    uint16_t client_port;
    uint32_t sequence_num;
    uint32_t package_length;
    uint32_t send_time;
    uint32_t version;
    char reserved[PROTOCOL_HEAD_RESERVED_LEN]; 
};

#pragma pack(pop)

static const uint32_t PROTOCOL_HEAD_SIZE = sizeof(ProtocolHead);

inline void Net2Host(ProtocolHead &head)
{
    head.client_ip = ntohl(head.client_ip);
    head.client_port = ntohs(head.client_port);
    head.sequence_num = ntohl(head.sequence_num);
    head.package_length = ntohl(head.package_length);
    head.send_time = ntohl(head.send_time);
    head.version= ntohl(head.version);
}

inline void Host2Net(ProtocolHead &head)
{
    head.client_ip = htonl(head.client_ip);
    head.client_port = htons(head.client_port);
    head.sequence_num = htonl(head.sequence_num);
    head.package_length = htonl(head.package_length);
    head.send_time = htonl(head.send_time);
    head.version= htonl(head.version);
}

class BaseProtocol
{
public:
    BaseProtocol();    
    ~BaseProtocol();    

    void set_head(const ProtocolHead &protocol_head);
    const ProtocolHead& head() const;

    void set_body(const std::string &body);
    const std::string& body() const;

    bool Package(std::string &output);
    bool Unpackage(const std::string &package);
    std::string error_message() const;

private:
    ProtocolHead m_head; 
    std::string m_body;
    char *m_package;    // buffer for package output
    std::string m_error_msg;
};

} // end of namespace
#endif
