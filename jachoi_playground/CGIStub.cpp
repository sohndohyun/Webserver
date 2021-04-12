#include "CGIStub.hpp"
#include "Exception.hpp"
#include "RequestParser.hpp"
#include "ChunkParser.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include "Utils.hpp"
#include "FileIO.hpp"
#include <signal.h>


#define debug
#ifdef debug
	#include "errno.h"
	using namespace std;
#endif
CGIStub::CGIStub(const std::string& req, const std::string& cgipath): cgipath(cgipath)
{

	int wpipe[2];
	int rpipe[2];
	RequestParser r(req);
	fcntl(rpipe[1], F_SETFL, O_NONBLOCK);
	fcntl(wpipe[0], F_SETFL, O_NONBLOCK);

	if (pipe(wpipe) == -1 || pipe(rpipe) == -1)
		throw Exception("pipe error");
	pid_t pid = fork();
	switch (pid)
	{
		case -1:
		{
			throw Exception("cgi Fork error");
			break;
		}
		case 0: // 자식 프로세스
		{
			char* const argv[] = {const_cast<char*>(cgipath.c_str()), 0};
			jachoi::set_env("REQUEST_METHOD", r.method);
			jachoi::set_env("SERVER_PROTOCOL", "HTTP/1.1");
			jachoi::set_env("PATH_INFO", r.pathparser->path);
				//TODO  Utils 에 HTTP_로 변환하는거 만들기
			if (r.header.find("X-Secret-Header-For-Test") != r.header.end())
			{
				jachoi::set_env("HTTP_X_SECRET_HEADER_FOR_TEST", r.header["X-Secret-Header-For-Test"]);
			}
			char ** envp = jachoi::get_envp();

			// cerr << "writing..." << body.size() << endl;
			// jachoi::FileIO(".tmp_cgi").write(body);

			dup2(rpipe[0], 0);
			dup2(wpipe[1], 1);
			execve(argv[0], argv, envp);
			cerr << "Exec failed : " << errno << endl;
			exit(1);
		}
		default: // 부모 프로세스
		{
			char bufs[1000000] = {0};
			// const std::string& body =  r.header["Transfer-Encoding"] == "chunked" ?
			// 	ChunkParser(const_cast<std::string&>(r.body)).getData() : r.body;
			ChunkParser chunk(r.body);
			int rdbytes = -1;
			size_t wrbytes = 0;
			while (true)
			{
				if (wrbytes == r.body.size())
					break;
				if (wrbytes + 60000 < r.body.size())
					wrbytes += write(rpipe[1], r.body.c_str() + wrbytes, 60000);
				else if (r.body.size() - wrbytes > 0)
					wrbytes += write(rpipe[1], r.body.c_str() + wrbytes, r.body.size() - wrbytes);
				rdbytes = read(wpipe[0], bufs, sizeof(bufs));
				result.append(bufs, rdbytes);
			}
			close(wpipe[0]);
			close(wpipe[1]);
			close(rpipe[0]);
			close(rpipe[1]);
			kill(pid, 9);
			// cout << "result : " <<  result.substr(0, 100) << endl;
		}
	}
}

const std::string CGIStub::getCGIResult()
{
	return result.substr(result.find("\r\n\r\n") + 4);
}

CGIStub::~CGIStub()
{
}
