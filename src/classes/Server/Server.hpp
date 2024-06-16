#ifndef SERVER_HPP
# define SERVER_HPP

# include "webserv.hpp"

class Server {
public:
	Server();
	~Server();
	void	run();
	void	loadConfig(const char* filename);
	
	//* tmp 
	void	printSettings();
	//* end of tmp
private:
	std::vector<ServerBlock>	m_serverBlocks;
	std::vector<ServerSocket>	m_listeningSockets;
	EventPoll					m_eventsManager;
	Clients						m_clientsMap;
	
	void	assignServerBlockSetting(const std::string& line, ServerBlock& serverBlock, int line_number);
	void	assignLocationBlockSetting(const std::string& line, LocationBlock& serverBlock, int line_number);
	void	setupListeningSockets();
	void	setupEpoll();
	void	acceptNewConnection(const Event& event);

	class BadOpenFile : public std::exception {
		const char* what() const throw();
	};

	class UnknownDirective : public std::exception {
		const char* what() const throw();
	};

	class BadConfig : public std::exception {
		const char* what() const throw();
	};
};

#endif
