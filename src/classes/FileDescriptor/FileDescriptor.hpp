#ifndef FILEDESCRIPTOR_HPP
# define FILEDESCRIPTOR_HPP

# include "stdlibraries.hpp"

class FileDescriptor {

public:
	FileDescriptor();
	FileDescriptor(const int& fd);
	FileDescriptor(const FileDescriptor& other);
	FileDescriptor& operator=(const FileDescriptor& other);
	operator int() const;
	bool operator<(int fd) const;
	bool operator>(int fd) const;
	bool operator<=(int fd) const;
	bool operator>=(int fd) const;
	bool operator==(int fd) const;
	bool operator<(const FileDescriptor& other) const;
	bool operator>(const FileDescriptor& other) const;
	bool operator<=(const FileDescriptor& other) const;
	bool operator>=(const FileDescriptor& other) const;
	bool operator==(const FileDescriptor& other) const;

	virtual ~FileDescriptor();
	const int& 	get() const;
	void		set(const int& fd);
	void		close() const;
	void		setNonBlocking() const;
	bool		badFileDescriptor() const;

private:
	int 		m_fd;
};

#endif
