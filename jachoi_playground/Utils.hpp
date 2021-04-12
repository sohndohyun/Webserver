#pragma once
#include <string>
#include <algorithm>
#include <map>
#ifdef htons 
#undef htons
#endif
#ifdef htonl
#undef htonl
#endif

namespace jachoi
{
	std::string ltrim(std::string s);
	std::string rtrim(std::string s);
	std::string trim(std::string s);
	void	*memset(void *s, int c, unsigned long n);
	short htons(short n);
	long htonl(long n);
	std::string makeGMT(const char *time_zone, time_t tv_sec);
	int htoi(const std::string& num);
	std::string to_string(long num);
	std::map<std::string, std::string> make_envp(char** envp);
	char** get_envp();
	void set_env(const std::string key, const std::string value);
}