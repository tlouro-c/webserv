#include "Event.hpp"
#include "../EventPoll/EventPoll.hpp"

Event::Event() : FileDescriptor(), m_events(0) {}


Event::Event(const FileDescriptor& fd, const uint32_t& events) :
	FileDescriptor(fd), m_events(events) {}


uint32_t Event::getEvents() const {
	return (m_events);
}


void		Event::setEvents(const uint32_t& events) {
	m_events = events;
}

int			Event::fd() const {
	return (this->get());
}

bool	Event::isNewConnection(const std::vector<ServerSocket>& listeningSockets) {

	for (std::vector<ServerSocket>::const_iterator current = listeningSockets.begin();
		current != listeningSockets.end(); std::advance(current, 1)) {
			if ((*current).getFileDescriptor() == fd()) {
				m_port = (*current).getBoundPort();
				m_hostname = (*current).getBoundIP();
				return (true);
			}
		}
	return (false);
}

bool	Event::isReadable() const {
	return (m_events & epoll::READ_OPERATIONS);
}

bool	Event::isWritable() const {
	return (m_events & epoll::WRITE_OPERATIONS);
}


int	Event::getPort() const {
	return (m_port);
}

std::string	Event::getHostname() const {
	return (m_hostname);
}


bool operator==(const ServerSocket* serverSocket, const Event event) {
	if (serverSocket) {
		return (serverSocket->getFileDescriptor() == event.get());
	}
	return (false);
}
