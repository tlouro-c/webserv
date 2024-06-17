#include "webserv.hpp"

std::vector<byte>*	getBufferNextLine(char* buffer, const  ssize_t& bytesRead, unsigned int& offset, const bool& isBody)
{
	std::vector<byte>* line = NULL;

	if (offset < bytesRead) {
		line = new std::vector<byte>;
	}
	while (offset < bytesRead) {

		line->push_back(buffer[offset]);
		if (!isBody && buffer[offset] == '\n') {
			offset++;
			break;
		}
		offset++;
	}
	return line;
}
