#ifndef YEVENT_NET_INETADDRESS_H
#define YEVENT_NET_INETADDRESS_H

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace yevent {

inline uint64_t hostToNetwork64(uint64_t host64) {
  return htobe64(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32) {
  return htonl(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16) {
  return htons(host16);
}

inline uint64_t networkToHost64(uint64_t net64) {
  return be64toh(net64);
}

inline uint32_t networkToHost32(uint32_t net32) {
  return ntohl(net32);
}

inline uint16_t networkToHost16(uint16_t net16) {
  return ntohs(net16);
}

class InetAddress {
 public:
  InetAddress() { }
  InetAddress(const std::string& ip, int port);
  InetAddress(const struct sockaddr_in& addr);
  ~InetAddress();

  std::string ToString() const;

  const struct sockaddr_in& GetSockAddrInet() const { return addr_; }
  void SetSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

 private:
  struct sockaddr_in addr_;
};

namespace sockets {

int Socket(); 
void Bind(int sockfd, const struct sockaddr_in& addr); 
void Listen(int sockfd); 
int Accept(int sockfd, struct sockaddr_in& addr); 
void Close(int sockfd); 
struct sockaddr_in GetLocalAddr(int sockfd);
int GetSocketError(int sockfd);

}

}

#endif

