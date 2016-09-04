#include "protocol.h"
#include <string.h>

using namespace std;

namespace tiny_srv{

BaseProtocol::BaseProtocol()
    :m_package(0)
{
    m_head.version = PROTOCOL_VERSION;
    m_package = new char[MAX_PACKAGE_LENGTH];
}

BaseProtocol::~BaseProtocol()
{
    if(m_package != NULL){
        delete m_package;
        m_package = NULL;
    }
}

void BaseProtocol::set_head(const ProtocolHead &protocol_head)
{
    m_head = protocol_head;
}

const ProtocolHead& BaseProtocol::head() const
{
    return m_head;
}

void BaseProtocol::set_body(const string &body)
{
    m_body.assign(body);
}

const string& BaseProtocol::body() const
{
    return m_body;
}

bool BaseProtocol::Package(string &output)
{
    uint32_t package_length = 0;
    m_package[package_length++] = SRV_STX;
    
    m_head.package_length = 2 + PROTOCOL_HEAD_SIZE + m_body.length();
    Host2Net(m_head);
    memcpy(m_package+package_length, &m_head, PROTOCOL_HEAD_SIZE);
    package_length += PROTOCOL_HEAD_SIZE;

    if(package_length + m_body.length() > MAX_PACKAGE_LENGTH){
        m_error_msg.assign("package is too large");
        return false;
    }

    memcpy(m_package+package_length, m_body.c_str(), m_body.length());
    package_length += m_body.length();

    m_package[package_length++] = SRV_ETX;
    output.assign(m_package, package_length);
    return true;
}

bool BaseProtocol::Unpackage(const string &package)
{
    if(package.empty()){
        m_error_msg.assign("package is empty");
        return false;
    }

    const char *buffer = package.c_str();
    uint32_t length = package.size();
    if(SRV_STX != buffer[0]){
        m_error_msg.assign("package is not start with SRV_STX");
        return false;
    }

    if(SRV_ETX != buffer[length - 1]){
        m_error_msg.assign("package is not end with SRV_ETX");
        return false;
    }

    // package min size is PROTOCOL_HEAD_SIZE + sizeof(SRV_STX) + sizeof(SRV_ETX)
    if(length < PROTOCOL_HEAD_SIZE + 2){
        m_error_msg.assign("package length is less than min size(PROTOCOL_HEAD_SIZE + 2)");
        return false;
    }

    if(length > MAX_PACKAGE_LENGTH){
        m_error_msg.assign("package is too large");
        return false;
    }

    buffer += 1;
    m_head = *reinterpret_cast<ProtocolHead*>(const_cast<char*>(buffer));
    Net2Host(m_head);
    if(m_head.package_length != length){
        m_error_msg.assign("real package length is different from length in head");
        return false;
    }

    buffer += PROTOCOL_HEAD_SIZE;
    uint32_t body_length = length - 2 - PROTOCOL_HEAD_SIZE;
    m_body.assign(buffer, body_length);
    return true;
}

string BaseProtocol::error_message() const
{
    return m_error_msg;
}

} // end of namespace
