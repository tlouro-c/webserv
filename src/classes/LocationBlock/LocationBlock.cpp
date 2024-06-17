#include "LocationBlock.hpp"

LocationBlock::LocationBlock() : m_routePath(""), m_root(""), m_indexFile(""),
	m_uploadFilesPath("./"), m_redirection(std::make_pair("", "")),
	m_allowedMethods(), m_directoryListing(false) {
		m_allowedMethods.push_back("GET");
		m_allowedMethods.push_back("POST");
		m_allowedMethods.push_back("DELETE");
		
	}

LocationBlock::~LocationBlock() {}

LocationBlock::LocationBlock(const LocationBlock& other) {

	if (this != &other) {
		*this = other;
	}
}

LocationBlock& LocationBlock::operator=(const LocationBlock& other) {

	if (this != &other) {
		m_routePath = other.m_routePath;
		m_root = other.m_root;
		m_indexFile = other.m_indexFile;
		m_redirection = other.m_redirection;
		m_allowedMethods = other.m_allowedMethods;
		m_directoryListing = other.m_directoryListing;
		m_uploadFilesPath = other.m_uploadFilesPath;
	}
	return (*this);
}

const std::string&	LocationBlock::getRoutePath() const {
	return (m_routePath);
}

void				LocationBlock::setRoutePath(const std::string& routePath) {
	m_routePath = routePath;
}

const std::string&	LocationBlock::getRoot() const {
	return (m_root);
}

void				LocationBlock::setRoot(const std::string& root) {
	m_root = root;
}

const std::string&	LocationBlock::getIndexFile() const {
	return (m_indexFile);
}

void				LocationBlock::setIndexFile(const std::string& indexFile) {
	m_indexFile = indexFile;
}

const StrPair&		LocationBlock::getRedirection() const {
	return (m_redirection);
}

void				LocationBlock::setRedirection(const StrPair& redirection) {
	m_redirection = redirection;
}

const StrVector&	LocationBlock::getAllowedMethods() const {
	return (m_allowedMethods);
}

void				LocationBlock::setAllowedMethods(const StrVector& allowedMethods) {
	m_allowedMethods = allowedMethods;
}

const bool&			LocationBlock::getDirectoryListing() const {

	return (m_directoryListing);
}

void				LocationBlock::setDirectoryListing(const bool&	directoryListing) {
	m_directoryListing = directoryListing;
}

const std::string&	LocationBlock::getUploadFilesPath() const {
	return (m_uploadFilesPath);
}

void	LocationBlock::setUploadFilesPath(const std::string& uploadFilesPath) {
	m_uploadFilesPath = uploadFilesPath;
}
