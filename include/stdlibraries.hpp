#ifndef STDLIBRARIES_HPP
# define STDLIBRARIES_HPP

# include <iostream>
# include <fstream>
# include <sstream>
# include <string>
# include <cstring>
# include <cerrno>
# include <cstdlib>
# include <exception>
# include <vector>
# include <map>
# include <algorithm>
# include <iterator>
# include <numeric>
# include <iomanip>

# include <sys/socket.h>
# include <sys/errno.h>
# include <unistd.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/epoll.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <sys/types.h>
# include <netdb.h>
# include <sys/stat.h>
# include <dirent.h>
# include <signal.h>
# include <sys/time.h>

# include "../src/utils/CircularBuffer.hpp"
# include "../src/utils/utills.hpp"
# include "../src/classes/FileDescriptor/FileDescriptor.hpp"

typedef unsigned char byte;
typedef struct sigaction s_sigaction;

typedef std::pair<std::string, std::string> StrPair;
typedef std::vector<std::string>			StrVector;
typedef std::string::iterator				StrIt;
typedef std::vector<byte>::iterator			ByteStreamIt;

extern volatile sig_atomic_t g_signalStatus;

const int BUFFER_SIZE = 4096;

#endif
