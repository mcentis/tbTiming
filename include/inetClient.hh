#ifndef INETCLIENT_HH
#define INETCLIENT_HH

#include <netinet/in.h> // sockaddr_in

class inetClient{
public:
  inetClient(long int bufSize);
  bool Connect(const char* address, int port);
  bool Send(const char* msg, int len);
  bool SendCString(const char* msg);
  long int Receive();
  char* GetBufferPointer();
  ~inetClient();
  
private:
  int _socket;
  sockaddr_in _server;
  char* _buffer;
  long int _bufSize;
};

#endif // #ifndef INETCLIENT_HH
