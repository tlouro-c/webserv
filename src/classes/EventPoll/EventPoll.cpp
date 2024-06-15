#include "EventPoll.hpp"
#include "../ServerSocket/ServerSocket.hpp"
#include "../FileDescriptor/FileDescriptor.hpp"


/**
 * @brief Creates a new instance of the EventPoll Class, by initializing it's file descriptor with the epoll_create() function
 * 
 * @throw In case of an error throws and exception of type `EventPollException` (std::exception)
*/
EventPoll::EventPoll() : m_fileDescriptor(epoll_create(MAX_EVENTS)) {

	if (m_fileDescriptor < 0) {
		throw EventPollException("epoll_create()" + std::string (std::strerror(errno)));
	} 
}


EventPoll::~EventPoll() {
	m_fileDescriptor.close();
}


/**
 * @brief Add a new file descriptor to the `EventPoll`
 * 
 * @param fileDescriptor the file descriptor to be added
 * @param eventsToNotify the type of events to be notified for
 *
 * @throw  In case of an error throws and exception of type `EventPollException` (std::exception)
 * 
*/ 
void	EventPoll::add(const FileDescriptor& fileDescriptor, uint32_t eventsToNotify) const {

	epoll_event eventSettings;

	std::memset(&eventSettings, 0, sizeof(eventSettings));
	eventSettings.data.fd = fileDescriptor;
	eventSettings.events = eventsToNotify;

	if (epoll_ctl(m_fileDescriptor,
		EPOLL_CTL_ADD, fileDescriptor, &eventSettings) == -1) {
			throw EventPollException("epoll_ctl(ADD) " + std::string (std::strerror(errno)));
		}
}


/**
 * @brief Modify the events for a file descriptor inside the `EventPoll`
 * 
 * @param fileDescriptor the file descriptor to be modified
 * @param eventsToNotify the new type of events to be notified for
 * 
 * @throw  In case of an error throws and exception of type `EventPollException` (std::exception)
 * 
*/
void	EventPoll::mod(const FileDescriptor& fileDescriptor, uint32_t eventsToNotify) const {

	epoll_event eventSettings;

	std::memset(&eventSettings, 0, sizeof(eventSettings));
	eventSettings.data.fd = fileDescriptor;
	eventSettings.events = eventsToNotify;

	if (epoll_ctl(m_fileDescriptor,
		EPOLL_CTL_MOD, fileDescriptor, &eventSettings) == -1) {
			throw EventPollException("epoll_ctl(MOD) " + std::string (std::strerror(errno)));
		}
}

/**
 * @brief Remove a file descriptor inside the `EventPoll`
 * 
 * @param fileDescriptor the file descriptor to be removed
 * 
 * @throw  In case of an error throws and exception of type `EventPollException` (std::exception)
 * 
*/
void	EventPoll::remove(const FileDescriptor& fileDescriptor) const {


	epoll_ctl(m_fileDescriptor, EPOLL_CTL_DEL, fileDescriptor, NULL);
	fileDescriptor.close();
}


/**
 * @brief Starts waiting for new event notification
 * 
 * @param controlSockets A vector of the sockets to listen for new events on
 * 
 * @throw  In case of an error throws and exception of type `EventPollException` (std::exception)
 * 
*/
bool	EventPoll::waitForEvents() const {

	epoll_event newEvents[MAX_EVENTS / 4];

	int newEventsNum = epoll_wait(m_fileDescriptor, newEvents, MAX_EVENTS / 4, 1);
	if (newEventsNum == -1) {
		throw EventPollException("epoll_wait() " + std::string (std::strerror(errno)));
	} else if (newEventsNum == 0) {
		return (false);
	}
	

	for (int i = 0; i < newEventsNum; i++) {
		if (newEvents[i].data.fd != STDIN_FILENO && newEvents[i].data.fd != STDOUT_FILENO) {
			m_newEvents.push_back(Event(newEvents[i].data.fd, newEvents[i].events));
		}
	}
	return (true);
}


/**
 * @return Returns the next file descriptor where there is a notification
 * 
 * @throw  Whenever there's no more events, throws and exception of type `NoMoreNewEvents`
 * 
*/
Event	EventPoll::getNextEvent() const {
	
	if (m_newEvents.isEmpty()) {
		throw NoMoreNewEvents();
	}

	Event nextEvent = m_newEvents.front();
	m_newEvents.pop_front();

	return (nextEvent);
}


//* Exceptions


EventPoll::EventPollException::EventPollException
	(const std::string& errorMessage) : m_errorMessage(
		"EventPollException: " + errorMessage) {}


const char* EventPoll::EventPollException::what() const throw() {
	return (m_errorMessage.c_str());
}


EventPoll::EventPollException::~EventPollException() throw() {}


const char* EventPoll::NoMoreNewEvents::what() const throw() {
	return ("No more new events");
}
