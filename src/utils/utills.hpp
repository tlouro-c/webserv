#ifndef UTILS_HPP
# define UTILS_HPP

# include <exception>

typedef unsigned char byte;


class FileDescriptor;

std::vector<byte>*	getBufferNextLine(char* buffer, const  ssize_t& bytesRead,
						unsigned int& offset, const bool& isBody);
bool				startsWith(const std::string& toExpect, const std::string& text);
unsigned int		countWords(const std::string& str);
bool				isOnlySpaces(const std::string& str);
std::string			normalizeHostname(const std::string& hostname);
void				ifstreamToString(std::ifstream& source, std::string& destination);
bool				isFile(const std::string& filename);
bool				isDirectory(std::string& filename);
std::string			basicHtml(const std::string& title, const std::string& body);
char**				vectorToCharPtrArr(const std::vector<std::string>& vector);
void				cleanCharPtrArr(char**& toClean);
void				signalHandler(int signal_num);
void				sigpipe_handler(int signum);
std::string			strip(const std::string& str);
std::string			decodeUrl(const std::string& url);
bool endsWith(const std::string& toExpect, const std::string& text);

template<typename Iterator>
Iterator std_next(const Iterator& it, int x = 1) {

	Iterator returnIterator = it;
	std::advance(returnIterator, x);
	return (returnIterator);
}

class BadFileDescriptor : public std::exception {
public:
	const char* what() const throw() {
		return ("Bad file descriptor");
	}
};

class BadRead : public std::exception {
public:
	const char* what() const throw() {
		return ("Bad read");
	}
};

class UnexpectedDifference : public std::exception {
public:
	const char* what() const throw() {
		return ("Unexpected difference");
	}
};

#endif
