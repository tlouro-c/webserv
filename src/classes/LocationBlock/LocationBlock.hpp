#ifndef LOCATIONBLOCK_HPP
# define LOCATIONBLOCK_HPP

# include "stdlibraries.hpp"

class LocationBlock {
public:
	LocationBlock();
	~LocationBlock();
	LocationBlock(const LocationBlock& other);
	LocationBlock& operator=(const LocationBlock& other);
	const std::string&	getRoutePath() const;
	void				setRoutePath(const std::string& routePath);
	const std::string&	getRoot() const;
	void				setRoot(const std::string& root);
	const std::string&	getIndexFile() const;
	void				setIndexFile(const std::string& indexFile);
	const StrPair&		getRedirection() const;
	void				setRedirection(const StrPair& redirection);
	const StrVector&	getAllowedMethods() const;
	void				setAllowedMethods(const StrVector& allowedMethods);
	const bool&			getDirectoryListing() const;
	void				setDirectoryListing(const bool&	directoryListing);
	const std::string&	getUploadFilesPath() const;
	void				setUploadFilesPath(const std::string& uploadFilesPath);

private:
	std::string	m_routePath;
	std::string	m_root;
	std::string	m_indexFile;
	std::string	m_uploadFilesPath;
	StrPair		m_redirection;
	StrVector	m_allowedMethods;
	bool		m_directoryListing;
	
};

#endif
