#include "ServerSocket.hpp"

using namespace sockets;

ServerSocket::ServerSocket() {}


ServerSocket::~ServerSocket() {}

ServerSocket::ServerSocket(const ServerSocket& other) :
	m_fileDescriptor(other.m_fileDescriptor) {}

/**
 * @brief Constructs a new TCP socket and binds its connection to a port and a hostname
 *
 * @param hostname hostname on which to assign a connection 
 * @param port port on which to assign a connection
 * 
 * @throw  In case of an error throws and exception of type `ServerSocketException` (std::exception)
 * 
*/
ServerSocket::ServerSocket(const std::string& hostname, const int& port)
	: m_fileDescriptor(socket(IPV4, TCP, DEFAULT_PROTOCOL)) {

	if (m_fileDescriptor < 0) {
		throw ServerSocketException("socket() " + std::string (std::strerror(errno)));
	}

	try {
		(void)hostname;
		in_addr_t IpAddress = resolveHostname(hostname);
		bindSocket(IpAddress, port);
		setListeningState();
		m_fileDescriptor.setNonBlocking();
	}
	catch (const std::string& errorStr) {
		closeSocket();
		throw ServerSocketException(errorStr);
	}
}


/**
 * @brief Resolves the given hostname to its corresponding IP address.
 * 
 * @param hostname The hostname to resolve.
 * 
 * @return The corresponding IP address in `in_addr_t` format
 * 
 * @throw If the hostname couldn't be resolved, it throws an exception of type std::string containing the error message
 */
in_addr_t	ServerSocket::resolveHostname(const std::string& hostname) {

	struct addrinfo hint, *result(0);

	std::memset(&hint, 0, sizeof(hint));
	hint.ai_family = IPV4;
	hint.ai_socktype = TCP;

	int status = getaddrinfo(hostname.c_str(), NULL, &hint, &result);
	if (status < 0 || !result) {
		if (result)
			freeaddrinfo(result);
		throw "resolveHostname() - " + std::string(gai_strerror(status));
	}

	in_addr_t ipAddress =
		reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
	freeaddrinfo(result);

	return (ipAddress);
}


/**
 * @brief Binds a socket to a certain ipAddress and port
 * 
 * @param ipAddress 
 * 
 * @throw If the bind function couldn't be resolved, it throws an exception of type std::string containing the error message
 */
void	ServerSocket::bindSocket(in_addr_t ipAddress, int port) {

	if (port < 0 || port > 65536) {
		throw (std::string("bindSocket() - Invalid port"));
	}

	sockaddr_in connectionConfig;

	std::memset(&connectionConfig, 0, sizeof(connectionConfig));
	connectionConfig.sin_family = IPV4;
	connectionConfig.sin_addr.s_addr = ipAddress;
	connectionConfig.sin_port = htons(port);

	if (bind(m_fileDescriptor.get(), reinterpret_cast<sockaddr*>(&connectionConfig),
		sizeof(connectionConfig)) == -1) {
			throw ("bindSocket() - " + std::string(std::strerror(errno)));
		}
}


/**
 * @brief Sets a listening state on the server socket
 * 
 * @throw If the listen function failed, it throws an exception of type std::string containing the error message
 */
void	ServerSocket::setListeningState() {
	if (listen(m_fileDescriptor.get(), MAX_CONNECTIONS_QUEUE) == -1) {
		throw ("setListeningState() - " + std::string(std::strerror(errno)));
	}
}


//* Basic Functions


void	ServerSocket::closeSocket() {
	m_fileDescriptor.close();
}

bool ServerSocket::isOpen() {
	return (!m_fileDescriptor.badFileDescriptor());
}


const FileDescriptor&		ServerSocket::getFileDescriptor() const {
	return (m_fileDescriptor);
}

int ServerSocket::getBoundPort() const {

	sockaddr_in addr;
	socklen_t len = sizeof(addr);
	if (getsockname(m_fileDescriptor, (struct sockaddr*)&addr, &len) == -1) {
		throw ServerSocketException("getsockname() " + std::string (std::strerror(errno)));
	}
	return ntohs(addr.sin_port);
}

std::string ServerSocket::getBoundIP() const {

	sockaddr_in addr;
	socklen_t len = sizeof(addr);
	char ipStr[INET_ADDRSTRLEN];
	
	if (getsockname(m_fileDescriptor, (struct sockaddr*)&addr, &len) == -1) {
		throw ServerSocketException("getsockname() " + std::string (std::strerror(errno)));
	}

	if (!inet_ntop(AF_INET, &addr.sin_addr, ipStr, sizeof(ipStr))) {
		throw ServerSocketException("inet_ntop() " + std::string (std::strerror(errno)));
	}
	return std::string(ipStr);
}


//* Exceptions


ServerSocket::ServerSocketException::ServerSocketException
	(const std::string& errorMessage) : m_errorMessage(
		"ServerSocketException: " + errorMessage) {}


const char* ServerSocket::ServerSocketException::what() const throw() {
	return (m_errorMessage.c_str());
}


ServerSocket::ServerSocketException::~ServerSocketException() throw() {}
