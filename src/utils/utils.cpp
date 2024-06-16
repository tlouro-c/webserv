#include "stdlibraries.hpp"

bool	startsWith(const std::string& toExpect, const std::string& text) {

	int start = 0;
	while (std::isspace(text[start])) {
		start++;
	}
	return (toExpect.length() > text.length() ?
		false : text.substr(start, toExpect.length()) == toExpect);
}

bool endsWith(const std::string& toExpect, const std::string& text) {

	int end = text.length() - 1;
	while (std::isspace(text[end])) {
		end--;
	}
	return (toExpect.length() > text.length() ?
		false : text.substr(end - toExpect.length() + 1, toExpect.length()) == toExpect);
}

unsigned int countWords(const std::string& str)
{
    std::stringstream stream(str);
    return std::distance(std::istream_iterator<std::string>(stream), std::istream_iterator<std::string>());
}

bool isOnlySpaces(const std::string& str) {

	std::size_t i = 0;
	std::size_t len = str.length();
	while (i < len) {
		if (!isspace(str[i])) {
			return false;
		}
		i++;
	}
	return true;
}


std::string normalizeHostname(const std::string& hostname) {
	if (hostname == "localhost" || hostname == "127.0.0.1") {
		return "localhost";
	}
	return hostname;
}


void	ifstreamToString(std::ifstream& source, std::string& destination) {

	destination.assign(std::istreambuf_iterator<char>(source), std::istreambuf_iterator<char>());
}


bool	isFile(const std::string& filename) {

	struct stat buffer;
	return (stat(filename.c_str(), &buffer) == 0 && !S_ISDIR(buffer.st_mode));
}


bool isDirectory(std::string& filename) {
	struct stat buffer;
	if (stat(filename.c_str(), &buffer) != 0) {
		return false;
	}
	if (S_ISDIR(buffer.st_mode)) {
		if (*(filename.end() - 1) != '/') {
			filename += '/';
		}
		return (true);
	} else {
		return (false);
	}
}

std::string basicHtml(const std::string& title, const std::string& body) {
    return 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<title>" + title + "</title>\n"
        "<link href='https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css' "
        "rel='stylesheet' integrity='sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH' "
        "crossorigin='anonymous'>\n"
        "</head>\n"
        "<body>\n"
        + body +
        "</body>\n"
        "</html>\n";
}


char**	vectorToCharPtrArr(const std::vector<std::string>& vector) {

	char**	result = new char*[vector.size() + 1];
	int				i = 0;

	for (std::vector<std::string>::const_iterator it = vector.begin();
			it != vector.end(); it++) {
				result[i] = new char[(*it).length() + 1];
				std::memset(result[i], 0, (*it).length() + 1);
				std::strcpy(result[i], (*it).c_str());
				i++;
			}
	result[i] = NULL;
	return (result);
}


void	cleanCharPtrArr(char**& toClean) {

	for (int i = 0; toClean[i]; i++) {
		delete[] toClean[i];
	}
	delete[] toClean;
}

void signalHandler(int signal_num) {
	(void)signal_num;
	g_signalStatus = 1;
}

void sigpipe_handler(int signum) {
    std::cerr << "SIGPIPE received: Broken pipe error" << std::endl;
	(void)signum;
}


std::string	strip(const std::string& str) {

	std::string::const_iterator firstLetterIt = str.begin();
	while (std::isspace(*firstLetterIt)) {
		firstLetterIt++;
	}
	std::string::const_iterator lastLetterIt = str.end() - 1;
	while (std::isspace(*lastLetterIt)) {
		lastLetterIt--;
	}
	return (std::string(firstLetterIt, lastLetterIt + 1));
}


std::string decodeUrl(const std::string& url) {

	std::ostringstream OStream;
	for (std::size_t i = 0; i < url.size(); i++) {
		if (url[i] == '%') {
			int value;
			std::stringstream stringStream(url.substr(i + 1, 2));
			stringStream >> std::hex >> value;
			OStream << static_cast<char>(value);
			i += 2;
		} else {
			OStream << url[i];
		}
	}
	return OStream.str();
}
