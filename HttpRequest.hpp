#pragma once
#include <iostream>
#include <string>
#include <map>
#include <optional>

struct StringBuffer
{
	char* begin = NULL;//字符串开始位置
	char* end = NULL;//字符串结束位置

	operator std::string() const
	{
		return std::string(begin, end);
	}
};

//Http请求解析的状态
enum class HttpUrlDecodeState
{
	COMPLETE = 0, //完成
	INVALID,//无效
	START,//响应行开始
	PROTOCOL,//协议
	FIRSTSLAP, 
	SECONDSLAP,
	IP, //IP地址
	PORT, //端口号
	PATH, //路径
	QUERY_KEY, //参数键
	QUERY_VALUE, //参数值
	FRAGMENG //信息片段
};

class HttpRequest
{
public:
	HttpRequest(std::string ip,
		int port,
		std::string path,
		std::optional<std::map<std::string, std::string>> params = std::nullopt,
		std::string method = "GET");

	HttpRequest(std::string url);

	HttpRequest(const HttpRequest& cls) = default;
	HttpRequest& operator=(const HttpRequest& cls) = default;
	HttpRequest(HttpRequest&& cls) = default;
	HttpRequest& operator=(HttpRequest&& cls) = default;
	~HttpRequest() = default;

	//设置请求方法
	void setMethod(std::string method);

	//添加请求字段，重复添加会覆盖				
	void addField(std::string key, std::string value);

	//解析URL
	bool decodeUrl(std::string url);

	//获取请求头
	const std::string getRequest();

	//获取ip
	inline std::string getIp()
	{
		return _ip;
	}

	//获取端口
	inline int getPort()
	{
		return _port;
	}

private:
	std::string _ip; //ip
	int _port; //端口
	std::string _method = "GET"; // 请求方法
	std::string _path; // 路径
	std::optional<std::map<std::string, std::string>> _params ; // 参数
	std::string _protocol; // 协议
	std::string _verison = "1.1"; //协议版本
	std::map<std::string, std::string> _headerLine; // 请求头字段信息
	std::string _fragment; //信息字段
};