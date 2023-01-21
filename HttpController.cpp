#pragma once 
#include "HttpController.hpp"
#define CR '\r'
#define LF '\n'
#define getDecodeState(state) (state != HttpResponseDecodeState::INVALID\
			&& state != HttpResponseDecodeState::BODY\
			&& state != HttpResponseDecodeState::CHUNK_SIZE\
			&& state != HttpResponseDecodeState::COMPLETE\
			&& state != HttpResponseDecodeState::NOT_COMPLETE)

#define addEndChar(pStart, len) {char end = '\0';\
			memcpy(pStart + len - 1, &end, 1);}

	HttpController* HttpController::_pInstance = new HttpController();
	HttpController::AutoRelease HttpController::_autoRelease;

	HttpController* HttpController::getInstence()
	{
		if (nullptr == _pInstance)
		{
			_pInstance = new HttpController();
		}
		return _pInstance;
	}

	HttpController::HttpController() : _ip("unknown"), _port(-1)
	{
#ifdef _WIN32
		WSADATA wData;
		::WSAStartup(MAKEWORD(2, 2), &wData);
#endif // _WIN32
		_recvBuffer = new NetBuffer();
		_recvBuffer->init();
	}

	HttpController::~HttpController()
	{
#ifdef _WIN32
		::closesocket(_clientSocket);
		::WSACleanup();
#else
		close(_clientSocket);
#endif // _WIN32
		delete _recvBuffer;
	}

	//建立TCP连接linux
	bool HttpController::connectServerL(std::string ip, int port)
	{
		_ip = ip;
		_port = port;

		_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		struct sockaddr_in ServerAddr { 0 };
		ServerAddr.sin_addr.s_addr = inet_addr(ip.c_str());
		ServerAddr.sin_port = htons(port);
		ServerAddr.sin_family = AF_INET;
		int errNo = connect(_clientSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr));
		//判断是否连接成功
		if (0 != errNo)
			return false;
		return true;
	}

	//建立TCP连接windows
	bool HttpController::connectServerW(std::string ip, int port)
	{
		_ip = ip;
		_port = port;

		struct sockaddr_in ServerAddr { 0 };
		ServerAddr.sin_addr.s_addr = inet_addr(ip.c_str());
		ServerAddr.sin_port = htons(port);
		ServerAddr.sin_family = AF_INET;

		SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		int errNo = connect(clientSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr));
		_clientSocket = (int)clientSocket;
#ifdef _WIN32
		//判断是否连接成功
		if (0 != errNo)
		{
			::WSACleanup();
			return false;
		}
#endif
		return true;
	}

	void HttpController::cleanSocket()
	{
#ifdef _WIN32
		::closesocket(_clientSocket);
#else
		close(_clientSocket);
#endif // _WIN32
		_ip = "unknow";
		_port = -1;
		_recvBuffer->cleanBuffer();
	}

	// 循环写通道层接收缓冲区（全部写入）
	uint32_t HttpController::writeNetBuffer()
	{
		//消息是否到达判断

		uint32_t totalLen = 0;
		int res = 2048;
		uint32_t pageSize = 2048;

		//每次从内核接收缓冲区读取若干字节
		while (res == pageSize)
		{
			char* buff = new char[pageSize]();
			res = recvn(buff, (int)pageSize);
			_recvBuffer->writeBuffer(buff, res);
			totalLen += res;
			delete[] buff;
		}

		//返回写入总长度
		return totalLen;
	}

	// 发送HTTP请求 修改设置回调、重试
	bool HttpController::sendHttpRequest(HttpRequest& req, HttpResponse& resp)
	{
		//判断是否已经连接（目前默认只建立一个连接）
		if (!(req.getIp() == _ip && req.getPort() == _port))
			//未连接
			connectServer(req.getIp(), req.getPort());

		//发送HTTP请求
		const std::string requestStr = req.getRequest();

		if (sendn(requestStr.c_str()) <= 0)
		{
			std::cerr << "发送HTTP请求失败\n";
			cleanSocket();
			return false;
		}

		//写接收网络缓冲区数据
		writeNetBuffer();

		//解析HTTP响应
		if (!decodeHeader(resp))
		{
			std::cerr << "解析响应头失败\n";
			cleanSocket();
			return false;
		}

		//接收响应体
		if (!recvResponse(resp))
		{
			std::cerr << "解析响应体未完成\n";
			cleanSocket();
			return false;
		}

		std::cout << resp.body << std::endl;

		return checkDecodeState(resp);
	}

	// 响应状态处理
	bool HttpController::checkDecodeState(HttpResponse& resp)
	{
		//状态码是否有效
		int code = atoi(resp.code.c_str());
		if (code >= 400)
		{
			printf("响应状态码：%d\n", code);
			cleanSocket();
			return false;
		}

		//是否keep-alive
		auto it = resp.responseLine.find("Connection");
		if (!(it != resp.responseLine.end() && it->second == "keep-alive"))
			cleanSocket();

		return true;
	}

	// 解析HTTP响应头
	bool HttpController::decodeHeader(HttpResponse& resp)
	{
		std::string responseKey;
		std::string responseValue;

		while (getDecodeState(resp.decodeState))
		{
			char ch = 0;
			_recvBuffer->readBuffer(&ch, 1);

			//状态判断
			switch (resp.decodeState)
			{
			//开始前状态
			case HttpResponseDecodeState::START:
				if (CR == ch || LF == ch || isblank(ch)) {}//不符开始条件，跳过
				else if (isupper(ch))
				{
					resp.protocol += ch;
					resp.decodeState = HttpResponseDecodeState::PROTOCOL;
				}
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
			//协议字段状态
			case HttpResponseDecodeState::PROTOCOL:
				if (isupper(ch))
					resp.protocol += ch;
				else if ('/' == ch)
					resp.decodeState = HttpResponseDecodeState::VERSION;
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
			//协议版本状态
			case HttpResponseDecodeState::VERSION:
				if (isdigit(ch) || '.' == ch)
					resp.version += ch;
				else if (isblank(ch))
					resp.decodeState = HttpResponseDecodeState::BEFORE_CODE;
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
			//状态码之前状态
			case HttpResponseDecodeState::BEFORE_CODE:
				if (isblank(ch)) {}
				else if (isdigit(ch))
				{
					resp.code += ch;
					resp.decodeState = HttpResponseDecodeState::CODE;
				}
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
			//状态码字段状态
			case HttpResponseDecodeState::CODE:
				if (isdigit(ch))
					resp.code += ch;
				else if (CR == ch)
					resp.decodeState = HttpResponseDecodeState::WHEN_CR;
				else if (isblank(ch))
					resp.decodeState = HttpResponseDecodeState::AFTER_CODE;
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
			//状态码结束后状态
			case HttpResponseDecodeState::AFTER_CODE:
				if (isblank(ch) || isalpha(ch)) {}
				else if (CR == ch)
					resp.decodeState = HttpResponseDecodeState::WHEN_CR;
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
			//请求头字段键状态
			case HttpResponseDecodeState::HEADER_KEY:
				if ('-' == ch || isalnum(ch))
					responseKey += ch;
				else if (isblank(ch))
					resp.decodeState = HttpResponseDecodeState::HEADER_BEFORE_COLON;
				else if (':' == ch)
					resp.decodeState = HttpResponseDecodeState::HEADER_AFTER_COLON;
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
			//请求头字段冒号前状态
			case HttpResponseDecodeState::HEADER_BEFORE_COLON:
				if (isblank(ch)) {}
				else if (':' == ch)
					resp.decodeState = HttpResponseDecodeState::HEADER_AFTER_COLON;
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
			//请求头字段冒号后状态
			case HttpResponseDecodeState::HEADER_AFTER_COLON:
				if (isblank(ch)) {}
				else
				{
					responseValue += ch;
					resp.decodeState = HttpResponseDecodeState::HEADER_VALUE;
				}
				break;
			//请求头字段值状态
			case HttpResponseDecodeState::HEADER_VALUE:
				if (CR == ch)
				{
					resp.responseLine.insert({ responseKey, responseValue });
					//清空
					responseKey.clear();
					responseValue.clear();
					resp.decodeState = HttpResponseDecodeState::WHEN_CR;
				}
				else
					responseValue += ch;
				break;
			//第一个\r状态
			case HttpResponseDecodeState::WHEN_CR:
				if (LF == ch)
					resp.decodeState = HttpResponseDecodeState::CR_LF;
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
			// \r\n状态
			case HttpResponseDecodeState::CR_LF:
				if (CR == ch)
					resp.decodeState = HttpResponseDecodeState::CR_LF_CR;
				else if (isblank(ch))
					resp.decodeState = HttpResponseDecodeState::INVALID;
				else
				{
					responseKey += ch;
					resp.decodeState = HttpResponseDecodeState::HEADER_KEY;
				}
				break;
			// \r\n\r状态
			case HttpResponseDecodeState::CR_LF_CR:
				if (LF == ch)
				{
					//判断Transfer-Encoding字段是否存在，字段值是否为chunked。
					auto it = resp.responseLine.find("Transfer-Encoding");
					auto it1 = resp.responseLine.find("Content-Length");
					if (it != resp.responseLine.end())
					{
						if ("chunked" == it->second)
							//swithch to chunk transport
							resp.decodeState = HttpResponseDecodeState::CHUNK_SIZE;
					}
					else if (it1 != resp.responseLine.end())
					{
						//swithch to normal transport
						resp.bodyLength = atoi(it1->second.c_str());
						if (resp.bodyLength > 0)
							resp.decodeState = HttpResponseDecodeState::BODY;
						else
							resp.decodeState = HttpResponseDecodeState::COMPLETE;
					}
					else
						//其他情况 未处理
						resp.decodeState = HttpResponseDecodeState::NOT_COMPLETE;
				}
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;

			default:
				break;
			}
		}
		return resp.decodeState == HttpResponseDecodeState::INVALID ? false : true;
	}

	// 接收HTTP响应体
	bool HttpController::recvResponse(HttpResponse& resp)
	{
		std::string chunkSize;
		std::string chunkBlock;

		if (HttpResponseDecodeState::BODY == resp.decodeState)
		{
			//读取缓冲区
			char* buff = new char[resp.bodyLength + 1]();
			_recvBuffer->readBuffer(buff, resp.bodyLength);
			addEndChar(buff, resp.bodyLength + 1);
			resp.body = buff;
			delete[] buff;

			resp.decodeState = HttpResponseDecodeState::COMPLETE;
			return true;
		}

		char ch;
		int len = -1;
		while (resp.decodeState != HttpResponseDecodeState::INVALID
			&& resp.decodeState != HttpResponseDecodeState::COMPLETE)
		{
			_recvBuffer->readBuffer(&ch, 1);

			switch (resp.decodeState)
			{
				//分块尺寸状态
			case HttpResponseDecodeState::CHUNK_SIZE:
				if (isalnum(ch))
					chunkSize += ch;
				else if (CR == ch)
					resp.decodeState = HttpResponseDecodeState::AFTER_SIZE;
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
			//chunksize之后
			case HttpResponseDecodeState::AFTER_SIZE:
				if (LF == ch)
				{
					len = std::stoi(chunkSize, 0, 16);
					chunkSize.clear();
					if (0 == len)
					{
						resp.body += chunkBlock;
						resp.decodeState = HttpResponseDecodeState::COMPLETE;
					}
					else
						resp.decodeState = HttpResponseDecodeState::CHUNK_BLOCK;
				}
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
				//分块读取状态
			case HttpResponseDecodeState::CHUNK_BLOCK:
			{
				chunkBlock += ch;

				//按分块长度读取(此处是第二个字符)
				char* buff = new char[len]();
				_recvBuffer->readBuffer(buff, len-1);
				addEndChar(buff, len);
				chunkBlock += buff;
				delete[] buff;
				resp.decodeState = HttpResponseDecodeState::AFTER_BLOCK;
				break;
			}
			//chunkblock之后/r
			case HttpResponseDecodeState::AFTER_BLOCK:
				if (CR == ch)
					resp.decodeState = HttpResponseDecodeState::AFTER_BLOCK_LF;
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;
			//chunkblock之后/r/n
			case HttpResponseDecodeState::AFTER_BLOCK_LF:
				if (LF == ch)
					resp.decodeState = HttpResponseDecodeState::CHUNK_SIZE;
				else
					resp.decodeState = HttpResponseDecodeState::INVALID;
				break;

			default:
				break;
			}
		}
		return resp.decodeState == HttpResponseDecodeState::COMPLETE ? true : false;
	}

	// 循环读接收缓冲区
	int HttpController::recvn(char* buf, int len)
	{
		int left = len;
		int ret = 0;

		while (left > 0)
		{
			ret = recv(_clientSocket, buf, left, MSG_WAITALL);
			//线程中断
			if (-1 == ret && errno == EINTR)
				continue;
			//抛出错误
			else if (-1 == ret)
			{
				std::cerr << "读取套接字接收缓冲区错误\n";
				return ret;
			}
			//读取完毕或者对端关闭
			else if (0 == ret)
				break;
			//正常情况读取
			else
			{
				buf += ret;
				left -= ret;
			}
		}

		//返回已读取长度。
		return len - left;
	}

	// 循环写发送缓冲区
	int HttpController::sendn(const char* buf, int len)
	{
		int left = len;
		int ret = 0;

		while (left > 0)
		{
			ret = send(_clientSocket, buf, left, 0);
			if (-1 == ret && errno == EINTR)
				continue;
			else if (-1 == ret)
			{
				std::cerr << "写入套接字发送缓冲区错误\n";
				return ret;
			}
			//读取完毕或者对端关闭
			else if (0 == ret)
				break;
			else
			{
				buf += ret;
				left -= ret;
			}
		}
		//返回已写长度。
		return len - left;
	}

	int HttpController::sendn(const char* buf)
	{
		int len = strlen(buf) + 1;
		return sendn(buf, len);
	}

