/*

*/

#ifndef UDPSERVER_H
#define UDPSERVER_H

bool UdpServer_init(void); // create socket, bind, start thread, return false on failure

bool UdpServer_isStopRequested(void); // true once "stop" has been received

void UdpServer_cleanup(void); // wait for the thread to finish, close the socker

#endif