#include "inetClient.hh"

#include <stdio.h> // perror
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h> // hostent
#include <iostream>
#include <unistd.h> // close
#include <string.h> // strlen

inetClient::inetClient(long int bufSize){
  _bufSize = bufSize;
  _buffer = new char[_bufSize];

  _socket = -1;
}

inetClient::~inetClient(){
  close(_socket);
  std::cout << "Connection closed" << std::endl;

  delete _buffer;
}

bool inetClient::Connect(const char* address, int port){
  // create socket if needed
  if(_socket == -1){
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == -1)
      perror("[inetClient::Connect] Could not create socket");
  }

  //check if address is an ip number or it is a name
  in_addr* bin_addr = new in_addr; // address in binary form
  if(inet_aton(address, bin_addr) == 0){ // the address is not an ip number. If the address is an ip number it is assigned to bin_addr in binary form

    // try to find host by name
    hostent* ent = gethostbyname(address);
    if(ent == NULL){
      herror("gethostbyname");
      perror("[inetClient::Connect] Could not find the ip address");
      return false;
    }

    _server.sin_addr = *((in_addr*) ent->h_addr_list[0]); // set server address
  }
  else
    _server.sin_addr = *bin_addr; // if the argument of the if worked, the address is stored in bin_addr

  delete bin_addr;

  _server.sin_family = AF_INET;
  _server.sin_port = htons(port);

  // print out information
  char str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(_server.sin_addr), str, INET_ADDRSTRLEN);
  std::cout << "Connecting to " << str << " port " << ntohs(_server.sin_port) << std::endl;

  if(connect(_socket, (sockaddr*)&_server, sizeof(_server)) < 0){ // try to connect to server, the casting is needed from connect
    perror("[inetClient::Connect] Connection failed");
    return false;
  }

  std::cout << "Connection established" << std::endl;
  
  return true;
}

bool inetClient::Send(const char* msg, int len){

  if(send(_socket, msg, len, 0) < 0){ // if the operation fails
    perror("[inetClient::Send] Send failed");
    return false;
  }
  
  return true;
}

bool inetClient::SendCString(const char* msg){

  int len = strlen(msg);
  if(send(_socket, msg, len, 0) < 0){ // if the operation fails
    perror("[inetClient::SendCString] Send failed");
    return false;
  }
  
  return true;
}

long int inetClient::Receive(){
  long int len;
  len = recv(_socket, _buffer, _bufSize, 0);
  if(len < 0){
    std::cerr << "[inetClient::Receive] Receive failed" << std::endl;
    return -1;
  }

  return len;  
}

char* inetClient::GetBufferPointer(){
  return _buffer;
}
