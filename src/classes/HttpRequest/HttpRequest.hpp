#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include "stdlibraries.hpp"

# include "../ServerBlock/ServerBlock.hpp"
# include "../FileDescriptor/FileDescriptor.hpp"

class Clients;

namespace http {

	const char OK_200[] = "200";
	const char REDIRECT_302[] = "302";
	const char NOT_FOUND_404[] = "404";
	const char NO_CONTENT_204[] = "204";
	const char NOT_IMPLEMENTED_501[] = "501";
	const char CONTENT_TOO_LARGE_413[] = "413";
	const char INTERNAL_ERROR_500[] = "500";
	const char FORBIDDEN_403[] = "403";

	enum RequestStatus {
		REQUEST_NOT_READ,
		REQUEST_READ,
		ERROR,
		OK,
		CGI,
		CGI_COMPLETE,
		CLOSE
	};

	enum ParseState {
		FIRST_LINE,
		HEADERS,
		BODY
	};

	enum CGIStatus {
		CGI_NOT_RUNNING,
		CGI_RUNNING,
		CGI_DONE,
		CGI_READ,
		CGI_ERROR
	};

	class HttpRequest {
	public:
								HttpRequest(const FileDescriptor& targetSocketFileDescriptor);
								~HttpRequest();
		RequestStatus			readRequest();
		RequestStatus			performReadOperations(const std::vector<ServerBlock>& serverBlocks);
		void					sendResponse();
		void					assignSettings(const std::vector<ServerBlock>& serverBlocks);
		void					setHostname(const std::string& hostname);
		void					setPort(const int& port);
		void					setDomain(const std::string& domain);
		void					setRequestStatus(const RequestStatus& status);
		const RequestStatus&	getRequestStatus();
		const CGIStatus&		getCgiStatus();
		void					buildErrorPage(const std::string& errorCode);
		CGIStatus				monitorCgiRunTime();
		void					reset();
		timeval					getStartTimeRequest();
		CGIStatus				performCgi();
		int						getCgiOutputFd();
		void					setCgiOutputFd(int fd);
		pid_t					getCgiPid();
		FileDescriptor			getTargetSocketFileDescriptor();
		bool					getKeepAlive();
		std::string				getHostname();
		int						getPort();

	private:
		std::string							m_hostname;
		int									m_port;
		std::string							m_domain;
		std::string							m_version;
		std::string							m_requestMethod;
		std::string							m_URL;
		std::string							m_filePath;
		std::string							m_queryString;
		std::string							m_responseBody;
		std::vector<char>					m_requestBody;
		std::string							m_contentType;
		std::string							m_requestContentType;
		std::string							m_requestConnection;
		std::string							m_contentLength;
		std::string							m_response;
		std::string							m_statusCode;
		std::map<std::string, std::string>	m_errorPages;
		int									m_maxBodySize;
		LocationBlock						m_settings;
		enum http::RequestStatus			m_requestStatus;
		enum http::ParseState				m_parseState;
		timeval								m_startTimeRequest;
		enum http::CGIStatus				m_cgiStatus;
		const FileDescriptor				m_targetSocketFileDescriptor;
		std::string							m_cgiFileExtension;
		int									m_CgiOutputFd;
		pid_t								m_cgiPid;
		int									m_requestId;
		bool								m_keepAlive;
		bool								m_isChunked;

		
		//* Private methods
						HttpRequest();
		std::string		expandStatusCode();
		std::string		expandContentType();
		std::string		expandContentLength();
		bool			performDirectoryListing();
		void			childProccess(int inputPipe[2], int outputPipe[2]);
		char**			createEnvironment();
		bool			unknownMethod();

		//* Exceptions

		class CgiError : public std::exception {
		public:
			const char* what() const throw();
		};

		class CgiNotFound : public std::exception {
		public:
			const char* what() const throw();
		};
	};

	typedef std::pair<const FileDescriptor, HttpRequest*> FdReqPair;
	typedef std::map<const FileDescriptor, HttpRequest*> FdReqMap;
	typedef std::map<const FileDescriptor, HttpRequest*>::iterator Iterator;

};


#endif
