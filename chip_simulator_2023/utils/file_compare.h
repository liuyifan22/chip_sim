#pragma once

#include <string>

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)
#include <windows.h>
#define PATH_SEPARATOR std::string("\\")
#elif defined(__APPLE__)
#define PATH_SEPARATOR std::string("/")
#elif defined(__linux__) || defined(__linux)
#define PATH_SEPARATOR std::string("/")
#else
#endif

/// @brief Execute a command line and return the output
/// @param cmdLine Command line to be executed
/// @return (std::string) Command output
std::string cmdPopen(const std::string& cmdLine) {
	char buffer[1024] = { '\0' };
	FILE* pf = NULL;
	pf = _popen(cmdLine.c_str(), "r");
	if (NULL == pf) {
		printf("open pipe failed\n");
		return std::string("");
	}
	std::string ret;
	while (fgets(buffer, sizeof(buffer), pf)) {
		ret += buffer;
	}
	_pclose(pf);
	return ret;
}


/// @brief Use fc command to compare two files
/// @param file1 File 1 path and name
/// @param file2 File 2 path and name
/// @return true if two files are the same, false otherwise
/// Example: fileCompare(".\\test1.txt", ".\\test2.txt")
bool fileCompare(const std::string& file1, const std::string& file2) {
	std::string cmdLine = "fc " + file1 + " " + file2;
	std::string ret = cmdPopen(cmdLine);
	//cout << ret << endl;
	if (ret.find("FC: no differences encountered") != std::string::npos) {
		return true;
	}
	else {
		return false;
	}
}




