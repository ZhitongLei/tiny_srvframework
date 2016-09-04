// File Name: protocol_use.cpp
// Author: lei
// Created Time: 2016-09-04 15:51:38
//
#include <iostream>
#include "protocol.h"
using namespace tiny_srv;
using namespace std;

int main()
{
    BaseProtocol base_protocol;
    string body = "tewtwrfdsfds";
    base_protocol.set_body(body);

    string package;
    base_protocol.Package(package);
    cout << "===== package =====" << endl;
    cout << "length: " << package.length() << endl;
    cout << "length in head: " << base_protocol.head().package_length << endl;
    cout << "body length: " << body.size() << endl;
    //cout << "package: " << package << endl;

    BaseProtocol p2;
    p2.Unpackage(package);
    ProtocolHead head = p2.head();
    cout << "===== unpackage =====" << endl;
    cout << "protocol length: " << head.package_length << endl;
    cout << "body length: " << p2.body().size() << endl;
    cout << "body: " << p2.body() << endl;
    return 0;
}
