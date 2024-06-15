# include "ServerBlock.hpp"

ServerBlock::ServerBlock() : m_port(80), m_maxBodySize(1024), m_hostname("0.0.0.0"),
	m_serverName(""), m_root("./"), m_cgiFileExtension(""){
		m_errorPages["404"] = "./Website/error_pages/404.html";
		m_errorPages["500"] = "./Website/error_pages/500.html";
		m_errorPages["501"] = "./Website/error_pages/501.html";
		m_errorPages["413"] = "./Website/error_pages/413.html";
		m_errorPages["403"] = "./Website/error_pages/403.html";
		m_errorPages["204"] = "./Website/error_pages/204.html";
	}

ServerBlock::~ServerBlock() {}

ServerBlock::ServerBlock(const ServerBlock& other) {
	
	if (this != &other) {
		*this = other;
	}
}

ServerBlock& ServerBlock::operator=(const ServerBlock& other) {
	
	if (this != &other) {
		m_port = other.m_port;
		m_maxBodySize = other.m_maxBodySize;
		m_hostname = other.m_hostname;
		m_serverName = other.m_serverName;
		m_root = other.m_root;
		m_errorPages = other.m_errorPages;
		m_LocationBlocks = other.m_LocationBlocks;
		m_cgiFileExtension = other.m_cgiFileExtension;
	}
	return (*this);
}

const int&			ServerBlock::getPort() const {
	return (m_port);
}
void				ServerBlock::setPort(const int& port) {
	m_port = port;
}
const int&			ServerBlock::getMaxBodySize() const {
	return (m_maxBodySize);
}
void				ServerBlock::setMaxBodySize(const int& maxBodySize) {
	m_maxBodySize = maxBodySize;
}
const std::string&	ServerBlock::getHostname() const {
	return (m_hostname);
}
void				ServerBlock::setHostname(const std::string& hostname) {
	m_hostname = hostname;
}
const std::string&	ServerBlock::getServerName() const {
	return (m_serverName);
}
void				ServerBlock::setServerName(const std::string& serverName) {
	m_serverName = serverName;
}
const std::string&	ServerBlock::getRoot() const {
	return (m_root);
}
void				ServerBlock::setRoot(const std::string root) {
	m_root = root;
}
const std::map<std::string, std::string>&	ServerBlock::getErrorPages() const {
	return (m_errorPages);
}
void	ServerBlock::addErrorPage
			(const std::string& errorCode, const std::string& path) {
	m_errorPages[errorCode] = path;
}
const	std::vector<LocationBlock>&	ServerBlock::getLocationBlocks() const {
	return (m_LocationBlocks);
}
void	ServerBlock::addLocationBlock(const LocationBlock& newLocationBlock) {
	m_LocationBlocks.push_back(newLocationBlock);
}

const std::string&	ServerBlock::getCgiFileExtension() const {
	return (m_cgiFileExtension);
}

void	ServerBlock::setCgiFileExtension(const std::string& cgiFileExtension) {
	m_cgiFileExtension = cgiFileExtension;
}
