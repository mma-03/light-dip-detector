/*
udpServer.h
UDP command server on port 12345 running in its own thread
answers help, count, length, dips, history, and stop. Enter repeats the last command
Call UdpServer_init() after the sampler is running, and UdpServer_cleanup() at shutdown
*/

#ifndef UDPSERVER_H
#define UDPSERVER_H

bool UdpServer_init(void); // create socket, bind, start thread, return false on failure

bool UdpServer_isStopRequested(void); // true once "stop" has been received

void UdpServer_cleanup(void); // wait for the thread to finish, close the socket

#endif