#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

# include "stdlibraries.hpp"
# include "../LocationBlock/LocationBlock.hpp"

class ServerBlock {
public:
	ServerBlock();
	~ServerBlock();
	ServerBlock(const ServerBlock& other);
	ServerBlock& operator=(const ServerBlock& other);
	const int&			getPort() const;
	void				setPort(const int& port);
	const int&			getMaxBodySize() const;
	void				setMaxBodySize(const int& maxBodySize);
	const std::string&	getHostname() const;
	void				setHostname(const std::string& hostname);
	const std::string&	getServerName() const;
	void				setServerName(const std::string& serverName);
	const std::string&	getRoot() const;
	void				setRoot(const std::string root);
	const std::string&	getScriptsPath() const;
	void				setScriptsPath(const std::string scriptsPath);
	const std::map<std::string, std::string>&	getErrorPages() const;
	void				addErrorPage (const std::string& errorCode,
							const std::string& path);
	const	std::vector<LocationBlock>&	getLocationBlocks() const;
	void	addLocationBlock(const LocationBlock& newLocationBlock);
	const std::string&	getUploadedFilesPath() const;
	void				setUploadedFilesPath(const std::string& uploadedFiles);
	const std::string&	getCgiFileExtension() const;
	void				setCgiFileExtension(const std::string& cgiFileExtension);
	
private:
	int			m_port;
	int			m_maxBodySize;
	std::string	m_hostname;
	std::string	m_serverName;
	std::string	m_root;
	std::string m_cgiFileExtension;
	std::map<std::string, std::string>	m_errorPages;
	std::vector<LocationBlock>	m_LocationBlocks;

};

#endif
