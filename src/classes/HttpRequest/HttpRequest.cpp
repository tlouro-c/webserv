#include "HttpRequest.hpp"
#include "../Clients/Clients.hpp"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_BLUE		"\x1b[34m"

using namespace http;

HttpRequest::HttpRequest(const FileDescriptor& targetSocketFileDescriptor) :
	m_domain(""),
	m_version("HTTP/1.1"),
	m_requestMethod(""),
	m_URL(""),
	m_filePath(""),
	m_queryString(""),
	m_responseBody (""),
	m_contentType(""),
	m_requestContentType(""),
	m_requestConnection(""),
	m_contentLength("0"), 
	m_response(""),
	m_statusCode(http::OK_200),
	m_maxBodySize(1000000),
	m_requestStatus(REQUEST_NOT_READ),
	m_parseState(FIRST_LINE),
	m_cgiStatus(CGI_NOT_RUNNING),
	m_targetSocketFileDescriptor(targetSocketFileDescriptor),
	m_cgiFileExtension(".py"),
	m_CgiOutputFd(-1),
	m_cgiPid(-1),
	m_keepAlive(true),
	m_isChunked(false),
	m_currentRequestBodySize(0),
	m_cgiInputFile(NULL)
	{
		gettimeofday(&m_startTimeRequest, NULL);
	}

bool HttpRequest::getKeepAlive() {
	return (m_keepAlive);
}

HttpRequest::~HttpRequest() {
	if (m_CgiOutputFd != -1) {
		close(m_CgiOutputFd);
		m_CgiOutputFd = -1;
	}
	if (m_cgiPid != -1) {
		kill(m_cgiPid, SIGKILL);
		m_cgiPid = -1;
	}
}

static std::vector<byte>::iterator find_crlf(std::vector<byte>::iterator begin, std::vector<byte>::iterator end) {
    std::vector<byte>::iterator it = begin;

    while (it != end) {
        if (*it == '\r' && (it + 1) != end && *(it + 1) == '\n') {
            return it;
        }
        it++;
    }
    return end;
}


/**
 * @brief Reads the HTTP request from the client
 * 
 * @return If everything goes well returns true and in case of an error returns false
 * 
*/
RequestStatus	HttpRequest::readRequest(const std::vector<ServerBlock>& serverBlocks) {

	gettimeofday(&m_startTimeRequest, NULL);

	ssize_t				bytesRead = 0;
	unsigned int		offset = 0;
	char				buffer[BUFFER_SIZE + 1];
	std::vector<byte>*	line(0);
	int					nextChunkSize = 0;

	bytesRead = read(m_targetSocketFileDescriptor, buffer, BUFFER_SIZE);
	if (bytesRead <= 0) {
		return (CLOSE);
	}
	while ((line = getBufferNextLine(buffer, bytesRead, offset, m_parseState == BODY)) != NULL) {

		if (m_parseState == FIRST_LINE) {
			ByteStreamIt firstSpace = std::find(line->begin(), line->end(), ' ');
			m_requestMethod = strip(std::string(line->begin(), firstSpace));
			ByteStreamIt questionMark = std::find(std_next(firstSpace), line->end(), '?');
			ByteStreamIt secondSpace = std::find(std_next(firstSpace), line->end(), ' ');
			if (questionMark != line->end()) {
				m_URL = decodeUrl(strip(std::string(std_next(firstSpace), questionMark)));
				m_queryString = strip(std::string(std_next(questionMark), secondSpace));
			} else {
				m_URL = decodeUrl(strip(std::string(std_next(firstSpace), secondSpace)));
			}
			m_parseState = HEADERS;
		} else if (m_parseState == HEADERS) {
			std::string lineStr = std::string(line->begin(), line->end());
			if (startsWith("Host:", lineStr)) {
				m_domain = strip(lineStr.substr(lineStr.find(":") + 2));
			} else if (startsWith("Content-Length:", lineStr)) {
				m_contentLength = lineStr.substr(lineStr.find(":") + 2);
			} else if (startsWith("Content-Type:", lineStr)) {
				m_requestContentType = strip(lineStr.substr(lineStr.find(":") + 2));
			} else if (startsWith("Connection:", lineStr)) {
				m_requestConnection = strip(lineStr.substr(lineStr.find(":") + 2));
			} else if (startsWith("Transfer-Encoding:", lineStr)) {
				m_isChunked = (strip(lineStr.substr(lineStr.find(":") + 2)) == "chunked");
			} else if (lineStr == "\n" || lineStr == "\r\n") {
				assignSettings(serverBlocks);
				m_filePath = m_settings.getRoot() + m_URL;
				m_parseState = BODY;
			}
		} else {
			if (!m_cgiFileExtension.empty() && endsWith(m_cgiFileExtension, m_filePath) && m_requestMethod == "POST" && m_cgiInputFile == NULL) {
				m_cgiInputFile = std::tmpfile();
			}
			if (m_isChunked) {
				do {
					ByteStreamIt sizeEnd = find_crlf(line->begin(), line->end());
					std::string chunkSizeStr(line->begin(), sizeEnd);
					nextChunkSize = std::strtol(chunkSizeStr.c_str(), NULL, 16);
					if (nextChunkSize == 0) {
						if (chunkSizeStr == "0" && nextChunkSize == 0) {
							delete (line);
							return (REQUEST_READ);
						}
						delete (line);
						return (REQUEST_NOT_READ);
					}
					std::vector<byte>::iterator chunkStart = std_next(sizeEnd, 2);
					std::vector<byte>::iterator chunkEnd = std_next(chunkStart, nextChunkSize);
					chunkEnd = find_crlf(chunkStart, line->end());
					std::vector<byte> chunk(chunkStart, chunkEnd);

					m_currentRequestBodySize += chunk.size();
					if (!m_cgiFileExtension.empty() && endsWith(m_cgiFileExtension, m_filePath) && m_requestMethod == "POST") {
						std::fwrite(&chunk[0], sizeof(byte), chunk.size(), m_cgiInputFile);
					}
					m_requestBody.insert(m_requestBody.end(), chunk.begin(), chunk.end());

					if (std::distance(chunkEnd, line->end()) > 2) {
						line->erase(line->begin(), std_next(chunkEnd, 2));
					} else {
						delete (line);
						if (m_currentRequestBodySize > m_maxBodySize) {
							buildErrorPage(http::CONTENT_TOO_LARGE_413);
							return (ERROR);
						}
						return (REQUEST_NOT_READ);
					}
				} while (nextChunkSize != 0);
			} else {
				m_currentRequestBodySize += line->size();
				if (m_currentRequestBodySize > m_maxBodySize) {
					delete (line);
					buildErrorPage(http::CONTENT_TOO_LARGE_413);
					return (ERROR);
				}
				if (!m_cgiFileExtension.empty() && endsWith(m_cgiFileExtension, m_filePath) && m_requestMethod == "POST") {
					std::fwrite(&(*line)[0], sizeof(byte), line->size(), m_cgiInputFile);
				}
				m_requestBody.insert(m_requestBody.end(), line->begin(), line->end());
			}
		}
		delete (line);
	}
	if (static_cast<int>(m_requestBody.size()) >= std::atoi(m_contentLength.c_str()) && !m_isChunked) {
		return (REQUEST_READ);
	}
	return (REQUEST_NOT_READ);
}


/**
 * @brief Performs read operations for the client HTTP request
 * 
 * @param serverBlocks The server blocks of the server
 * 
 * @return The status of the request
 * 
*/
RequestStatus	HttpRequest::performReadOperations(const std::vector<ServerBlock>& serverBlocks) {

	if (m_requestStatus != REQUEST_NOT_READ) {
		return (m_requestStatus);
	}

	switch (readRequest(serverBlocks)) {

		case CLOSE:
			return (CLOSE);
		case REQUEST_READ:
			m_requestStatus = REQUEST_READ;
			break;
		case ERROR:
			return (ERROR);
		default:
			return (REQUEST_NOT_READ);
	}
	if (unknownMethod()) {
		buildErrorPage(http::NOT_IMPLEMENTED_501);
		return (ERROR);
	}
	StrVector allowedMethods = m_settings.getAllowedMethods();
	if (std::find(allowedMethods.begin(), allowedMethods.end(),
		m_requestMethod) == allowedMethods.end()) {
			buildErrorPage(http::METHOD_NOT_ALLOWED_405);
			return (ERROR);
	}
	if (m_settings.getRedirection().first != "" && m_settings.getRedirection().second != "") {
		m_statusCode = m_settings.getRedirection().first;
		m_URL = m_settings.getRedirection().second;
	}

	if (m_requestMethod == "DELETE") {
		if (std::remove(m_filePath.c_str()) != 0) {
			buildErrorPage(NO_CONTENT_204);
			return (http::ERROR);
		}
		m_statusCode = OK_200;
		m_responseBody = basicHtml("Success Deleting File", "<h2>Success Deleting File</h2>");
		return (http::OK);
	}
	if (!m_cgiFileExtension.empty() && endsWith(m_cgiFileExtension, m_filePath)) {
		return (http::CGI);
	}
	if (isDirectory(m_filePath)) {
		m_filePath += m_settings.getIndexFile();
		if (!isFile(m_filePath) && m_settings.getDirectoryListing()) {
			performDirectoryListing();
			return (http::OK);
		}
	}
	std::ifstream requestedFile(m_filePath.c_str());
	if (requestedFile.fail() || isDirectory(m_filePath)) {
		buildErrorPage(NOT_FOUND_404);
		return (http::ERROR);
	}
	ifstreamToString(requestedFile, m_responseBody);
	requestedFile.close();
	m_statusCode = OK_200;
	return (http::OK);
}


/**
 * @brief Assigns the settings for the client HTTP request
 * 
 * @param serverBlocks The server blocks of the server
 * 
*/
void	HttpRequest::assignSettings(const std::vector<ServerBlock>& serverBlocks) {

	const ServerBlock*	defaultServerBlock = NULL;
	const ServerBlock*	selectedServerBlock = NULL;
	int				i = 0;

	for (std::vector<ServerBlock>::const_iterator it = serverBlocks.begin();
		it != serverBlocks.end(); std::advance(it, 1)) {

			if (m_port != it->getPort()
				|| (normalizeHostname(it->getHostname()) != "0.0.0.0" 
				&& normalizeHostname(m_hostname) != normalizeHostname(it->getHostname()))) {
					continue;
			}
			if (i == 0) {
				defaultServerBlock = &(*it);
				i++;
			}
			std::stringstream	strStream;
			strStream << it->getPort();
			std::string	domain(it->getServerName() + ":" + strStream.str());
			if (m_domain == domain) {
				selectedServerBlock = &(*it);
				break;
			}
		}
	if (!selectedServerBlock) {
		selectedServerBlock = defaultServerBlock;
	}

	std::string lastMatch = "";
	for (std::vector<LocationBlock>::const_iterator it = selectedServerBlock->getLocationBlocks().begin();
		it != selectedServerBlock->getLocationBlocks().end(); std::advance(it, 1)) {

			std::string routePath = (*it).getRoutePath();
			if (startsWith(routePath, m_URL) && routePath.length() > lastMatch.length()) {
				lastMatch = routePath;
				m_settings = *it;
			}
		}
	if (lastMatch == "") {
		m_settings.setRoot(selectedServerBlock->getRoot());
	}
	m_cgiFileExtension = selectedServerBlock->getCgiFileExtension();
	m_errorPages = selectedServerBlock->getErrorPages();
	m_maxBodySize = selectedServerBlock->getMaxBodySize();
}


/**
 * @brief Builds an error page for the client HTTP request
 * 
 * @param errorCode The error code of the response
 * 
*/
void	HttpRequest::buildErrorPage(const std::string& errorCode) {

	std::ifstream errorFile((m_errorPages[errorCode]).c_str());

	if (errorFile.fail()) {
		std::cout << strerror(errno) << std::endl;
		m_responseBody = basicHtml("500", "<h2>500 Internal Server Error</h2>");
		m_statusCode = "500";
		return;
	}

	m_filePath = m_errorPages[errorCode];
	ifstreamToString(errorFile, m_responseBody);
	errorFile.close();
	m_statusCode = errorCode;
}

std::string	HttpRequest::getHostname() {
	return (m_hostname);
}

int	HttpRequest::getPort() {
	return (m_port);
}


/**
 * @brief Expands the status code of the response
 * 
 * @return The status code of the response in string format
 * 
*/
std::string	HttpRequest::expandStatusCode() {

	std::string	expandedCode;

	if (m_statusCode == "404") {
		expandedCode = "404 Not Found";
	} else if (m_statusCode == "413") {
		expandedCode = "413 Request Entity Too Large";
	} else if (m_statusCode == "501") {
		expandedCode = "501 Not Implemented";
	} else if (m_statusCode == "500") {
		expandedCode = "500 Internal Server Error";
	} else if (m_statusCode == "200") {
		expandedCode = "200 OK";
	} else if (m_statusCode == "302") {
		expandedCode = "302 Found";
	} else if (m_statusCode == "403") {
		expandedCode = "403 Forbidden";
	} else if (m_statusCode == "301") {
		expandedCode = "301 Moved Permanently";
	} else if (m_statusCode == "204") {
		expandedCode = "204 No Content";
	} else {
		return (m_statusCode);
	}
	return (expandedCode);
}


/**
 * @brief Expands the content type of the response
 * 
 * @return The content type of the response in string format
 * 
*/
std::string	HttpRequest::expandContentType() {

	std::string			expandedType;
	std::string			fileExtension;

	if (m_contentType != "") {
		return (m_contentType);
	}
	std::string::size_type dot = m_filePath.find_last_of('.');
	if (dot != std::string::npos) {
		fileExtension = m_filePath.substr(dot + 1);
	} 

	if (fileExtension == "html" || fileExtension == "htm") {
		expandedType = "text/html";
	} else if (fileExtension == "css") {
		expandedType = "text/css";
	} else if (fileExtension == "js") {
		expandedType = "text/javascript";
	} else if (fileExtension == "jpeg" || fileExtension == "jpg") {
		expandedType = "image/jpeg";
	} else if (fileExtension == "png") {
		expandedType = "image/png";
	} else if (fileExtension == "gif") {
		expandedType = "image/gif";
	} else if (fileExtension == "bmp") {
		expandedType = "image/bmp";
	} else if (fileExtension == "ico") {
		expandedType = "image/x-icon";
	} else if (fileExtension == "svg") {
		expandedType = "image/svg+xml";
	} else if (fileExtension == "json") {
		expandedType = "application/json";
	} else if (fileExtension == "pdf") {
		expandedType = "application/pdf";
	} else if (fileExtension == "txt") {
		expandedType = "text/plain";
	} else {
		expandedType = "application/octet-stream";
	}
	return (expandedType);
}


/**
 * @brief Expands the content length of the response
 * 
 * @return The content length of the response in string format
 * 
*/
std::string HttpRequest::expandContentLength() {

	std::stringstream	tmp;
	
	tmp << m_responseBody.length();
	m_contentLength = tmp.str();
	return (m_contentLength);
}


/**
 * @brief Creates a response body with Directory Listing
 * 
 * @return If everything goes well returns true and in case of an error returns false
 * 
*/
bool		HttpRequest::performDirectoryListing() {

	DIR*		directory;
	dirent*		entry = NULL;
	std::string	htmlBody = "";

	directory = opendir(m_filePath.c_str());
	if (!directory) {
		return (false);
	}
	htmlBody += "\t<div class=\"container py-5 text-center\">\n";
	htmlBody += "\t<ul class=\"list-group\">\n";
	do {
		entry = readdir(directory);
		if (!entry) {
			break;
		}
		std::string entryName = entry->d_name;
		std::string path = m_URL;
		if (*path.rbegin() != '/') {
        	path += '/';
    	}
		path += entryName;
		htmlBody +=	"\t\t<li class=\"list-group-item\">\n"
						"\t\t\t<a href=\"" + path + "\" >" + entryName + "</a>\n"
					"\t\t</li>\n";
	} while (entry);
	closedir(directory);
	htmlBody += "\t</ul>\n";
	htmlBody += "\t</div>\n";
	m_responseBody = basicHtml("Directory Listing", htmlBody);
	m_contentType = "text/html";
	m_statusCode = OK_200;
	return (true);
}


CGIStatus	HttpRequest::performCgi() {

	if (m_cgiStatus == CGI_NOT_RUNNING) {
		int		outputPipe[2];

		if (pipe(outputPipe) == -1) {
			throw CgiError();
		}
		m_CgiOutputFd = outputPipe[0];
		fcntl(outputPipe[0], F_SETFL, O_NONBLOCK);
		fcntl(outputPipe[1], F_SETFL, FD_CLOEXEC);

		m_cgiPid = fork();
		if (m_cgiPid == -1) {
			throw CgiError();
		} else if (m_cgiPid == 0) {
			childProccess(outputPipe);
		}
		close(outputPipe[1]);
		m_cgiStatus = CGI_RUNNING;
	} else if (m_cgiStatus == CGI_DONE) {
		char buffer[BUFFER_SIZE] = {0};

		ssize_t bytesRead = read(m_CgiOutputFd, buffer, BUFFER_SIZE - 1);
		if (bytesRead < 0) {
			throw CgiError();
		} else if (bytesRead == 0) {
			m_cgiStatus = CGI_READ;
			return (m_cgiStatus);
		}
		m_response += std::string(buffer, bytesRead);
	}
	return (m_cgiStatus);
}


void	HttpRequest::childProccess(int outputPipe[2]) {
	
	int inputFileFd = -1;
	close(outputPipe[0]);
	if (m_requestMethod == "POST") {
		std::rewind(m_cgiInputFile);
		inputFileFd = fileno(m_cgiInputFile);
	}
	if (dup2(outputPipe[1], STDOUT_FILENO) == -1 || (m_requestMethod == "POST"
			&& dup2(inputFileFd, STDIN_FILENO) == -1)) {
				std::exit(1);
			}
	if (m_requestMethod == "POST") {
		close(inputFileFd);
	}
	close(outputPipe[1]);
	
	StrVector	argvVector;

	argvVector.push_back(m_URL.substr(1));

	char** argv = vectorToCharPtrArr(argvVector);
	char** env = createEnvironment();

	chdir(m_settings.getRoot().c_str());
	execve(argv[0], argv, env);
	std::cerr << strerror(errno) << std::endl;
	int status = errno;
	cleanCharPtrArr(argv);
	cleanCharPtrArr(env);
	std::exit(status);
}

char**	HttpRequest::createEnvironment() {

	std::vector<std::string>	envVector;
	std::stringstream			contentLengthStream;

	contentLengthStream << m_requestBody.size();

	envVector.push_back("PATH_INFO=" + m_filePath);
	envVector.push_back("DOMAIN=" + m_domain);
	envVector.push_back("REQUEST_METHOD=" + m_requestMethod);
	envVector.push_back("QUERY_STRING=" + m_queryString);
	envVector.push_back("CONTENT_LENGTH=" + contentLengthStream.str());
	envVector.push_back("CONTENT_TYPE=" + m_requestContentType);
	envVector.push_back("UPLOAD_DIR=" + m_settings.getUploadFilesPath());

	return (vectorToCharPtrArr(envVector));
}

timeval	HttpRequest::getStartTimeRequest() {
	return (m_startTimeRequest);
}


static std::string StrToLower(std::string str) {
	
	std::string lowerStr = "";
	for (std::string::iterator it = str.begin(); it != str.end(); std::advance(it, 1)) {
		lowerStr += std::tolower(*it);
	}
	return (str);
}


/**
 * @brief Sends the HTTP response over a socket to a client
 *
*/
void	HttpRequest::sendResponse() {

	if (m_requestStatus == CGI) {
		write(m_targetSocketFileDescriptor, m_response.c_str(), m_response.length());
	} else {
		m_response =	m_version + " " + expandStatusCode() + "\r\n"
						"Connection: " + (StrToLower(m_requestConnection) == "close" ? "Close" : "Keep-alive") + "\r\n"
						"Content-Type: " + expandContentType() + "\r\n"
						"Content-Length: " + expandContentLength() + "\r\n"
						"\r\n" + m_responseBody;
	}
	if (write(m_targetSocketFileDescriptor, m_response.c_str(), m_response.length()) <= 0) {
		m_requestConnection = "close";
	}
	std::cout << (std::atoi(m_statusCode.c_str()) >= 400 ? ANSI_COLOR_RED : ANSI_COLOR_BLUE) 
		<< "  [" << m_hostname << ":" << m_port << "] " << m_statusCode << " "
		<< m_requestMethod << " " << m_domain + m_URL << " " << m_version << ANSI_COLOR_RESET << std::endl;
	if (StrToLower(m_requestConnection) == "close") {
		m_keepAlive = false;
		m_requestStatus = CLOSE;
	} else {
		reset();
	}
}

bool HttpRequest::unknownMethod() {
	return (m_requestMethod != "GET" && m_requestMethod != "POST"
				&& m_requestMethod != "DELETE");
}

FileDescriptor HttpRequest::getTargetSocketFileDescriptor() {
	return (m_targetSocketFileDescriptor);
}


CGIStatus	HttpRequest::monitorCgiRunTime() {

	if (m_cgiStatus != CGI_RUNNING) {
		return (m_cgiStatus);	
	}

	timeval		now;
	long long	elapsed;
	int			status;

	gettimeofday(&now, NULL);
	elapsed = (now.tv_sec - m_startTimeRequest.tv_sec) * 1000000LL + (now.tv_usec - m_startTimeRequest.tv_usec);
	if (elapsed >= 10000000) {
		kill(m_cgiPid, SIGKILL);
	}
	if (waitpid(m_cgiPid, &status, WNOHANG) > 0) {
		if (m_requestMethod == "POST" && m_cgiInputFile != NULL) {
			fclose(m_cgiInputFile);
			m_cgiInputFile = NULL;
		}
		if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
			m_cgiStatus = CGI_DONE;
		} else {
			m_cgiStatus = CGI_ERROR;
		}
		m_cgiPid = -1;
	}
	return (m_cgiStatus);
}

void HttpRequest::reset() {

	m_domain = "";
	m_version = "HTTP/1.1";
	m_requestMethod = "";
	m_URL = "";
	m_filePath = "";
	m_queryString = "";
	m_responseBody = "";
	m_contentType = "";
	m_requestContentType = "";
	m_requestConnection = "";
	m_contentLength = "0";
	m_response = "";
	m_statusCode = http::OK_200;
	m_maxBodySize = 1024;
	m_requestStatus = REQUEST_NOT_READ;
	m_parseState = FIRST_LINE;
	m_cgiStatus = CGI_NOT_RUNNING;
	m_requestBody.clear();
	m_cgiFileExtension = ".py";
	m_CgiOutputFd = -1;
	m_cgiPid = -1;
	m_keepAlive = true;
	m_isChunked = false;
	m_currentRequestBodySize = 0;
	m_cgiInputFile = NULL;
	gettimeofday(&m_startTimeRequest, NULL);
}

//* Setters and Getters

void	HttpRequest::setHostname(const std::string& hostname) {
	m_hostname = hostname;
}

void	HttpRequest::setPort(const int& port) {
	m_port = port;
}

void	HttpRequest::setDomain(const std::string& domain) {
	m_domain = domain;
}

void	HttpRequest::setRequestStatus(const http::RequestStatus& status) {
	m_requestStatus = status;
}

const RequestStatus&	HttpRequest::getRequestStatus() {
	return (m_requestStatus);
}

const CGIStatus&	HttpRequest::getCgiStatus() {
	return (m_cgiStatus);
}

int HttpRequest::getCgiOutputFd() {
	return (m_CgiOutputFd);
}

pid_t HttpRequest::getCgiPid() {
	return (m_cgiPid);
}

void	HttpRequest::setCgiOutputFd(int fd) {
	m_CgiOutputFd = fd;
}

std::FILE*	HttpRequest::getCgiInputFile() {
	return (m_cgiInputFile);
}

void	HttpRequest::setCgiInputFile(std::FILE* file) {
	m_cgiInputFile = file;
}

//* Exceptions


const char* HttpRequest::CgiError::what() const throw() {
	return ("Cgi Error");
}

const char* HttpRequest::CgiNotFound::what() const throw() {
	return ("Cgi Not Found");
}
