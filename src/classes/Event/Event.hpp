#ifndef EVENT_HPP
# define EVENT_HPP

# include "stdlibraries.hpp"

# include "../ServerSocket/ServerSocket.hpp"

class ServerSocket;

class Event : public FileDescriptor {
public:
					Event();
					Event(const FileDescriptor& fd, const uint32_t& events);
	uint32_t		getEvents() const;
	void			setEvents(const uint32_t& events);
	int				fd() const;
	bool			isNewConnection(const std::vector<ServerSocket>& listeningSockets);
	bool			isReadable() const;
	bool			isWritable() const;
	int				getPort() const;
	std::string		getHostname() const;
private:
	uint32_t		m_events;
	std::string		m_hostname;
	int				m_port;
};

bool operator==(const ServerSocket* serverSocket, const Event event);

#endif
