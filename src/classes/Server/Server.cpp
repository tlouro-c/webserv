# include "Server.hpp"
# include "webserv.hpp"

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_BLUE    "\x1b[34m"

volatile sig_atomic_t g_signalStatus = 0;

Server::Server() {}

Server::~Server() {
	for (std::vector<ServerSocket>::iterator it = m_listeningSockets.begin();
			it != m_listeningSockets.end(); std::advance(it, 1)) {
				(*it).closeSocket();
			}
}


/**
 * @brief Server Core
*/
void	Server::run() {
	Event				newEvent;
	http::RequestStatus	status;
	http::HttpRequest	*activeRequest;

	setupListeningSockets();
	setupEpoll();
	while (true && g_signalStatus == 0) {
		try {
			if (!m_eventsManager.waitForEvents()) {
				m_clientsMap.removeClosedConnections(m_eventsManager);
			}
		}
		catch (const std::exception&) {
			m_clientsMap.closeOpenConnections();
			return;
		}
		while (NEW_EVENTS && g_signalStatus == 0) {
			try {
				newEvent = m_eventsManager.getNextEvent();
				if (newEvent.isNewConnection(m_listeningSockets)) {
					acceptNewConnection(newEvent);
				} else if (newEvent.isReadable()) {
					try {
						activeRequest = m_clientsMap[newEvent.fd()];
					}
					catch (...) {
						m_eventsManager.remove(newEvent.fd());
						continue;
					}
					if (activeRequest->getRequestStatus() == http::CLOSE)
						continue;
					status = activeRequest->performReadOperations(m_serverBlocks);
					activeRequest->setRequestStatus(status);
					if (status == http::CGI) {
						http::CGIStatus cgiStatus;
					
						cgiStatus = activeRequest->performCgi();
						if (cgiStatus == http::CGI_RUNNING) {
							m_eventsManager.add(activeRequest->getCgiOutputFd(), READ_OPERATIONS);
							m_clientsMap.addToPipeMap(activeRequest->getCgiOutputFd(), activeRequest);
						} else if (cgiStatus == http::CGI_READ) {
							m_clientsMap.delFromPipeMap(activeRequest->getCgiOutputFd());
							m_eventsManager.remove(activeRequest->getCgiOutputFd());
							activeRequest->setCgiOutputFd(-1);
							status = http::CGI_COMPLETE;
						}
					}
					if (status != http::REQUEST_NOT_READ && status != http::CLOSE && status != http::CGI) {
						m_eventsManager.mod(activeRequest->getTargetSocketFileDescriptor(), WRITE_OPERATIONS);
					}
				} else if (newEvent.isWritable()) {
					activeRequest = m_clientsMap[newEvent.fd()];
					if (activeRequest->getRequestStatus() == http::CLOSE)
						continue;
					activeRequest->sendResponse();
					if (activeRequest->getRequestStatus() == http::REQUEST_NOT_READ) {
						m_eventsManager.mod(activeRequest->getTargetSocketFileDescriptor(), READ_OPERATIONS);
					}
				}
			}
			catch (const std::exception&) {
				m_clientsMap.removeClosedConnections(m_eventsManager);
				break;
			}
		}
	}
}


/**
 * @brief Accepts a new connection
 * 
 * @param event The event containing the file descriptor that triggered the new connection
*/
void	Server::acceptNewConnection(const Event& event) {

	FileDescriptor	newConnectionFd;

	newConnectionFd = accept(event.fd(), NULL, NULL);
	newConnectionFd.setNonBlocking();

	std::cout << ANSI_COLOR_GREEN << " ◉ " << ANSI_COLOR_RESET
		<< event.getHostname() << ":" << event.getPort() << "\n" << std::endl;

	http::HttpRequest* newRequest = new http::HttpRequest(newConnectionFd);
	newRequest->setPort(event.getPort());
	newRequest->setHostname(event.getHostname());
	m_clientsMap.addToSocketMap(newConnectionFd, newRequest);
	m_eventsManager.add(newConnectionFd, READ_OPERATIONS);
}

//* Setup


/**
 * @brief Sets up the listening sockets by creating them and adding them to the listening sockets vector
*/
void	Server::setupListeningSockets() {

	for (std::vector<ServerBlock>::iterator current = m_serverBlocks.begin();
		current != m_serverBlocks.end(); std::advance(current, 1)) {
			try {
				ServerSocket	newSocket((*current).getHostname(), (*current).getPort());
				if (newSocket.isOpen()) {
                	std::cout << "Listening on " << std::setw(20) << std::left 
					<< (*current).getHostname() << ":" << (*current).getPort()
					<< ANSI_COLOR_GREEN << "  [SUCCESS]" << ANSI_COLOR_RESET << std::endl;
					m_listeningSockets.push_back(newSocket);
				} else {
					throw std::exception();
				}
			}
			catch (const std::exception& exception) {
            	std::cerr << "Listening on " << std::setw(20) << std::left 
					<< (*current).getHostname() << ":" << (*current).getPort()
					<< ANSI_COLOR_RED << "  [FAILURE: " << exception.what() << "]" << ANSI_COLOR_RESET << std::endl;
			}
		}
	std::cout << std::endl;
}


/**
 * @brief Sets up the epoll event manager by adding the listening sockets
*/
void Server::setupEpoll() {

	for (std::vector<ServerSocket>::iterator current = m_listeningSockets.begin();
		current != m_listeningSockets.end(); std::advance(current, 1)) {
			m_eventsManager.add((*current).getFileDescriptor(), READ_OPERATIONS);
		}
}




//* Parsing

static bool incrementer(int& i) {
	
	i++;
	return (true);
}

static bool	isDirective(const std::string& line) {
	
	std::stringstream	lineStream(line);
	std::string			directive = "";

	lineStream >> directive;

	if (directive == "listen" || directive == "server_name" || directive == "root"
		|| directive == "error_page" || directive == "client_max_body_size"
		|| directive == "location" || directive == "scripts_path" || directive == "server:"
		|| directive == "upload_dir" || directive == "location") {
			return (true);
	}
	return (false);
}

/**
 * @brief Loads the server configuration from a file
 * 
 * @param filename The name of the file to load the configuration from
*/
void	Server::loadConfig(const char* filename) {

	std::ifstream						configFile(filename);
	std::map<std::string, std::string>	ConfigMap;
	std::string							line = "";
	std::string							errorLine = "";
	int									line_number = 0;

	if (configFile.fail()) {
		throw BadOpenFile();
	}

	while (!line.empty() || (std::getline(configFile, line) && incrementer(line_number))) {

		if (line == "server:") {
			ServerBlock newServerBlock;
			line.clear();
			while (!line.empty() || (std::getline(configFile, line) && incrementer(line_number) && !std::isalpha(line[0]) && (line[0] == '\t' || isOnlySpaces(line)))){
				int i = 0;
				while (isspace(line[i])) {i++;}
				if (isOnlySpaces(line) || line[i] == '#') {
					line.clear();
					continue;
				}
				if (startsWith(std::string("location "), line)) {
					LocationBlock 		newLocationBlock;
					newLocationBlock.setRoot(newServerBlock.getRoot());
					std::stringstream	tmpStream(line);
					std::string			tmpStr;

					tmpStream >> tmpStr;
					tmpStream >> tmpStr;
					newLocationBlock.setRoutePath(tmpStr);
					while (std::getline(configFile, line) && incrementer(line_number) && ((line[0] == '\t' && line[1] == '\t') || line[0] == '#')){
						int i = 0;
						while (isspace(line[i])) {i++;}
						if (isOnlySpaces(line) || line[i] == '#') {
							continue;
						}
						assignLocationBlockSetting(line, newLocationBlock, line_number);
					}
					if (!isDirective(line) && !line.empty() && line[0] != '#') {
						std::cout << ANSI_COLOR_RED << "[" << line_number << "] " << line << ANSI_COLOR_RESET << std::endl;
						throw BadConfig();
					}
					newServerBlock.addLocationBlock(newLocationBlock);
				}
				else {
					assignServerBlockSetting(line, newServerBlock, line_number);
					line.clear();
				}
			}
			if (line != "server:" && !line.empty() && line[0] != '#') {
				std::cout << ANSI_COLOR_RED << "[" << line_number << "] " << line << ANSI_COLOR_RESET << std::endl;
				throw BadConfig();
			}
			m_serverBlocks.push_back(newServerBlock);
		} else if (line.empty() || line[0] == '#') {
			continue; 
		} else {
			std::cout << ANSI_COLOR_RED << "[" << line_number << "] " << line << ANSI_COLOR_RESET << std::endl;
			throw BadConfig();
		}
	}
}


/**
 * @brief Assigns the server block settings
 * 
 * @param line The line to parse
 * @param serverBlock The server block to assign the settings to
*/
void	Server::assignServerBlockSetting(const std::string& line, ServerBlock& serverBlock, int line_number) {

	std::stringstream	lineStream(line);
	std::string			directive = "";
	std::string			value = "";
	std::string			tmp = "";

	lineStream >> directive;
	std::getline(lineStream, value, ';');
	std::stringstream	valueStream(value);

	if (directive == "listen") {
		if (value.find(':') || std::isalpha(value[0])) {
			std::getline(valueStream >> std::ws, tmp, ':');
			serverBlock.setHostname(tmp);
		}
		if (value.find(':') || std::isdigit(value[0])) {
			valueStream >> tmp;
			serverBlock.setPort(std::atoi(tmp.c_str()));
		}
		if (valueStream.fail())
			throw BadConfig();
	}
	else if (directive == "server_name") {
		valueStream >> tmp;
		if (valueStream.fail())
			throw BadConfig();
		serverBlock.setServerName(tmp);
	} else if (directive == "cgi_file_extension") {
		valueStream >> tmp;
		if (valueStream.fail())
			throw BadConfig();
		serverBlock.setCgiFileExtension(tmp);
	} else if (directive == "root") {
		valueStream >> tmp;
		if (valueStream.fail())
			throw BadConfig();
		serverBlock.setRoot(tmp);
	} else if (directive == "error_page") {
		if (countWords(value) != 2) {
			throw BadConfig();
		}
		std::string errorCode;
		valueStream >> errorCode;
		valueStream >> tmp;
		if (valueStream.fail())
			throw BadConfig();
		if (isFile(tmp))
			serverBlock.addErrorPage(errorCode, tmp);
	}
	else if (directive == "client_max_body_size") {
		valueStream >> tmp;
		if (valueStream.fail())
			throw BadConfig();
		serverBlock.setMaxBodySize(std::atoi(tmp.c_str()));
	}
	else {
		std::cout << ANSI_COLOR_RED << "[" << line_number << "] " << line << ANSI_COLOR_RESET << std::endl;
		throw UnknownDirective();
	}
}


/**
 * @brief Assigns the location block settings
 * 
 * @param line The line to parse
 * @param locationBlock The location block to assign the settings to
*/
void	Server::assignLocationBlockSetting(const std::string& line, LocationBlock& locationBlock, int line_number) {

	std::stringstream	lineStream(line);
	std::string			directive = "";
	std::string			value = "";
	std::string			tmp = "";

	lineStream >> directive;
	std::getline(lineStream, value, ';');
	std::stringstream	valueStream(value);

	if (directive == "root") {
		valueStream >> tmp;
		if (valueStream.fail())
			throw BadConfig();
		locationBlock.setRoot(tmp);
	}
	else if (directive == "index") {
		valueStream >> tmp;
		if (valueStream.fail())
			throw BadConfig();
		locationBlock.setIndexFile(tmp);
	}
	else if (directive == "return") {
		std::string statusCode;
		valueStream >> statusCode;
		valueStream >> tmp;
		if (valueStream.fail())
			throw BadConfig();
		locationBlock.setRedirection(std::make_pair(statusCode, tmp));
	}
	else if (directive == "allow") {
		StrVector	tmpVector;
		while (valueStream >> tmp) {
			tmpVector.push_back(tmp);
		}
		locationBlock.setAllowedMethods(tmpVector);
	} else if (directive == "upload_dir") {
		valueStream >> tmp;
		if (valueStream.fail())
			throw BadConfig();
		locationBlock.setUploadFilesPath(tmp);
	} else if (directive == "autoindex") {
		valueStream >> tmp;
		if (valueStream.fail() || (tmp != "on" && tmp != "off"))
			throw BadConfig();
		locationBlock.setDirectoryListing(tmp == "on" ? true: false);
	}
	else {
		std::cout << ANSI_COLOR_RED << "[" << line_number << "] " << line << ANSI_COLOR_RESET << std::endl;
		throw UnknownDirective();
	}
}


//* Exceptions

const char* Server::BadOpenFile::what() const throw() {
	
	return ("Error opening file");
}

const char* Server::UnknownDirective::what() const throw() {
	
	return ("Unknown directive");
}

const char* Server::BadConfig::what() const throw() {

	return ("Error reading config");
}


//* Debugging

/**
 * @brief Prints the server settings
*/
void	Server::printSettings() {
	
	std::cout << "Printing settings" << std::endl;
	for (std::vector<ServerBlock>::iterator it = m_serverBlocks.begin();
			it != m_serverBlocks.end(); std::advance(it, 1)) {
				std::cout << "SERVER BLOCK:" << std::endl;
				std::cout << "port: " << (*it).getPort() << std::endl;
				std::cout << "hostname: " << (*it).getHostname() << std::endl;
				std::cout << "server name: " << (*it).getServerName() << std::endl;
				std::cout << "root: " << (*it).getRoot() << std::endl;
				std::cout << "max body size: "<< (*it).getMaxBodySize() << std::endl;
				std::cout << "cgi file extension: " << (*it).getCgiFileExtension() << std::endl;
				if (!(*it).getErrorPages().empty())
					std::cout << "error pages: " << std::endl;
				for (std::map<std::string, std::string>::const_iterator it2 = (*it).getErrorPages().begin();
						it2 != (*it).getErrorPages().end(); std::advance(it2, 1)) {
							std::cout << "  " << it2->first << " - ";
							std::cout << it2->second << std::endl;
						}
				std::vector<LocationBlock> tmp = (*it).getLocationBlocks();
				for (std::vector<LocationBlock>::iterator it = tmp.begin(); it != tmp.end(); std::advance(it, 1)) {
					std::cout << "\nLOCATION BLOCK:" << std::endl;
					std::cout << '\t' << "route to: " << (*it).getRoutePath() << std::endl;
					std::cout << '\t' << "root: " << (*it).getRoot() << std::endl;
					std::cout << '\t' << "index: " << (*it).getIndexFile() << std::endl;
					std::pair<std::string, std::string> tmpPair = (*it).getRedirection();
					std::cout << '\t' << tmpPair.first << std::endl;
					std::cout << '\t' << tmpPair.second << std::endl;
					std::vector<std::string> tmpVector = (*it).getAllowedMethods();
					std::cout << '\t' << "Allowed Methods" << std::endl; 
					for (std::vector<std::string>::iterator it = tmpVector.begin(); it != tmpVector.end(); std::advance(it, 1)) {
						std::cout << '\t' << *it << std::endl;
					}
					std::cout << '\t' << (*it).getDirectoryListing() << std::endl;;
			}
	}
}
