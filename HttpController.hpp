#pragma once

#ifdef _WIN32
#include <WinSock.h>
#pragma comment(lib, "ws2_32.lib")
#define MSG_WAITALL 0
#undef max
#undef ERROR
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

//#ifdef _WIN32
//#ifdef DeltaEngine_Build
//#define Dll_Export_Import __declspec(dllexport)
//#elif defined(Static_Build)
//#define Dll_Export_Import
//#else
//#define Dll_Export_Import __declspec(dllimport)
//#endif
//#else
//#define Dll_Export_Import
//#endif

#include "NetBuffer.hpp"
#include "HttpRequest.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <map>
#include <functional>
#include <optional>

#ifdef _WIN32
#define connectServer connectServerW
#else
#define connectServer connectServerL
#endif // _WIN32

//Http响应解析的状态
enum class HttpResponseDecodeState
	{
		INVALID,//无效
		START,//响应行开始
		PROTOCOL,//协议
		VERSION, //协议版本
		BEFORE_CODE, //状态码开始前
		CODE, // 状态码
		AFTER_CODE, //状态码完成后
		HEADER_KEY,//键
		HEADER_BEFORE_COLON,//冒号之前
		HEADER_AFTER_COLON,//冒号之后
		HEADER_VALUE,//值
		WHEN_CR,//遇到一个回车之后
		CR_LF,//回车换行
		CR_LF_CR,//回车换行之后的状态
		CHUNK_SIZE, //分块尺寸
		CHUNK_BLOCK, //分块内容
		AFTER_BLOCK, //chunkblcok后/r
		AFTER_BLOCK_LF, //chunkblcok后/n
		AFTER_SIZE, //chunksize后/r
		AFTER_SIZE_LF, //chunksize后/n
		BODY,//响应体
		COMPLETE,//完成
		NOT_COMPLETE,//长度信息不足未完成
	};

struct HttpResponse
	{
		std::string protocol;//协议
		std::string version; //协议版本
		std::string code; //状态码
		std::multimap<std::string, std::string> responseLine;//响应行内容
		//std::string chunked; //分块信息 delete
		std::string body;//响应体
		uint32_t bodyLength = 0; //响应体长度
		HttpResponseDecodeState decodeState = HttpResponseDecodeState::START;//解析状态

	};

class HttpController
{

public:
	//获取实例指针
	static HttpController* getInstence();

	// 发送HTTP请求
	bool sendHttpRequest(HttpRequest& req, HttpResponse& resp);

private:
	HttpController();
	~HttpController();
	HttpController(const HttpController& cls) = delete;
	HttpController& operator=(const HttpController& my) = delete;
	//用于自动销毁
	class AutoRelease
	{
	public:
		AutoRelease() { };
		~AutoRelease()
		{
			if (nullptr != _pInstance)
				delete _pInstance;
		}
	};

	// 建立TCP连接
	bool connectServerL(std::string ip, int port);

	// 建立TCP连接
	bool connectServerW(std::string ip, int port);

	// 清理socket
	void cleanSocket();

	// 解析HTTP响应头
	bool decodeHeader(HttpResponse& resp);

	// 接收HTTP响应体
	bool recvResponse(HttpResponse& resp);

	// 解析状态处理
	bool checkDecodeState(HttpResponse& res);

	// 读套接字内核接收缓冲区
	int recvn(char* buf, int len);

	// 读通道层接收缓冲区
	uint32_t readNetBuffer(char* pDst, int size);

	// 循环写通道层接收缓冲区（全部写入）
	uint32_t writeNetBuffer();

	// 写套接字内核发送缓冲区
	int sendn(const char* buf, int len);

	// 写套接字内核发送缓冲区，自主获取缓冲区数据长度
	int sendn(const char* buf);

private:
	static HttpController* _pInstance;
	static AutoRelease _autoRelease;

	std::string _ip;
	int _port;
	int _clientSocket;
	NetBuffer *_recvBuffer;
	//NetBuffer* _sendBuffer;
};