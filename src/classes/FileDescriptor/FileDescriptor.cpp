#include "FileDescriptor.hpp"
#include "../FileDescriptor/FileDescriptor.hpp"


namespace unistd {
	/**
     *  @brief Closes a file descriptor.
     * 
     * @param fd The file descriptor to close.
     * @return 0 on success, -1 on failure.
     */
	int close (int fd) {
		return (::close(fd));
	}
};

FileDescriptor::FileDescriptor() : m_fd(-1) {}


FileDescriptor::FileDescriptor(const int& fd) : m_fd(fd){

	if (m_fd == -1) {
		return;
	}
}


FileDescriptor::FileDescriptor(const FileDescriptor& other) {

	if (this != &other) {
		*this = other;
	}
}


FileDescriptor& FileDescriptor::operator=(const FileDescriptor& other) {

	if (this != &other) {
		this->m_fd = other.m_fd;
	}
	return (*this);
}


FileDescriptor::~FileDescriptor() {}


const int&	FileDescriptor::get() const {
	return (m_fd);
}


void	FileDescriptor::set(const int& fd) {
	m_fd = fd;
}


void	FileDescriptor::close() const {
	if (m_fd != -1) {
		unistd::close(m_fd);
	}
}

void	FileDescriptor::setNonBlocking() const {
	fcntl(m_fd, F_SETFL, O_NONBLOCK);
}

bool		FileDescriptor::badFileDescriptor() const {

	if (m_fd == -1) {
		return (true);
	}
	return (false);
}

FileDescriptor::operator int() const {
	return (m_fd);
}

bool FileDescriptor::operator<(int fd) const { return (m_fd < fd); }
bool FileDescriptor::operator>(int fd) const { return (m_fd > fd); }
bool FileDescriptor::operator<=(int fd) const { return (m_fd <= fd); }
bool FileDescriptor::operator>=(int fd) const { return (m_fd >= fd); }
bool FileDescriptor::operator==(int fd) const { return (m_fd == fd); }
bool FileDescriptor::operator<(const FileDescriptor& other) const { return (m_fd < other.m_fd); }
bool FileDescriptor::operator>(const FileDescriptor& other) const { return (m_fd > other.m_fd); }
bool FileDescriptor::operator<=(const FileDescriptor& other) const { return (m_fd <= other.m_fd); }
bool FileDescriptor::operator>=(const FileDescriptor& other) const { return (m_fd >= other.m_fd); }
bool FileDescriptor::operator==(const FileDescriptor& other) const { return (m_fd == other.m_fd); }
