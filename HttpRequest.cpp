#include "HttpRequest.hpp"
#define CR '\r'

HttpRequest::HttpRequest(std::string url)
{
	if (!decodeUrl(url))
		std::cerr <<"URL解析失败\n";
	else
	{
		std::string value = _ip + ':' + std::to_string(_port);
		addField("Host", value);
	}
}

HttpRequest::HttpRequest(std::string ip,
	int port,
	std::string path,
	std::optional<std::map<std::string, std::string>> params,
	std::string method)
{
	_ip = ip;
	_port = port;
	_path = path.substr(1);
	if (std::nullopt != params)
		_params = params;
	_method = "GET";
	std::string value = _ip + ':' + std::to_string(_port);
	addField("Host", value);
}

//设置请求方法
void HttpRequest::setMethod(std::string method)
{
	_method = method;
}

void HttpRequest::addField(std::string key, std::string value)
{
	_headerLine[key] = value;
}

//获取请求头
const std::string HttpRequest::getRequest()
{
	std::string requestStr;
	//构造第一行信息
	requestStr += _method
		+ ' '
		+ '/'
		+ _path;
	//需判断参数是否存在 
	if (_params != std::nullopt)
	{
		requestStr += '?';
		for (auto iter = _params->begin(); iter != _params->end();)
		{
			requestStr += iter->first
				+ '='
				+ iter->second;
			if (++iter != _params->end())
				requestStr += '&';
			else
				requestStr += ' ';
		}
	}
	//拼接协议
	requestStr += ' '
		+ _protocol
		+ '/'
		+ _verison
		+ "\r\n";
	//拼接字段
	for (auto& field : _headerLine)
	{
		requestStr += field.first
			+ ':'
			+ field.second
			+ "\r\n";
	}
	requestStr += "\r\n";
	return requestStr;
}

//解析URL
bool HttpRequest::decodeUrl(std::string url)
{
	StringBuffer buff;
	std::string queryKey;
	std::string queryValue;
	std::map<std::string, std::string> query;
	HttpUrlDecodeState decodeState = HttpUrlDecodeState::START;
	//添加空格为结束符
	url += CR;
	char* pUrl = const_cast<char*>(url.c_str());

	while (decodeState != HttpUrlDecodeState::INVALID
		&& decodeState != HttpUrlDecodeState::COMPLETE)
	{
		char ch = *pUrl;
		char* pCur = pUrl++;
		switch (decodeState)
		{
		case HttpUrlDecodeState::START:
			if (isalpha(ch))
			{
				buff.begin = pCur;
				*pCur -= 32;
				decodeState = HttpUrlDecodeState::PROTOCOL;
			}
			else
				decodeState = HttpUrlDecodeState::INVALID;
			break;
		//协议状态
		case HttpUrlDecodeState::PROTOCOL:
			if (isalpha(ch)) 
			{
				*pCur -= 32;
			}
			else if (':' == ch)
			{
				buff.end = pCur;
				_protocol = buff;
				decodeState = HttpUrlDecodeState::FIRSTSLAP;
			}
			else
				decodeState = HttpUrlDecodeState::INVALID;
			break;
		//第一个斜杠
		case HttpUrlDecodeState::FIRSTSLAP:
			if ('/' == ch)
				decodeState = HttpUrlDecodeState::SECONDSLAP;
			else
				decodeState = HttpUrlDecodeState::INVALID;
			break;
		//第二个斜杠
		case HttpUrlDecodeState::SECONDSLAP:
			if ('/' == ch)
			{
				buff.begin = ++pCur;
				decodeState = HttpUrlDecodeState::IP;
			}
			else
				decodeState = HttpUrlDecodeState::INVALID;
			break;
		//IP状态
		case HttpUrlDecodeState::IP:
			//此处ip判断也不是很严谨，意思意思吧
			if ('.' == ch || isdigit(ch)) {}
			else if (':' == ch)
			{
				buff.end = pCur;
				_ip = buff;
				buff.begin = ++pCur;
				decodeState = HttpUrlDecodeState::PORT;
			}
			else
				decodeState = HttpUrlDecodeState::INVALID;
			break;
		//端口状态
		case HttpUrlDecodeState::PORT:
			if (isdigit(ch)) {}
			else if ('/' == ch)
			{
				buff.end = pCur;
				_port = atoi(std::string(buff).c_str());
				buff.begin = ++pCur;
				decodeState = HttpUrlDecodeState::PATH;
			}
			else if (CR == ch)
			{
				buff.end = pCur;
				_port = atoi(std::string(buff).c_str());
				buff.begin = ++pCur;
				decodeState = HttpUrlDecodeState::COMPLETE;
			}
			else
				decodeState = HttpUrlDecodeState::INVALID;
			break;
		//路径状态
		case HttpUrlDecodeState::PATH:
			if (isalnum(ch) || '_' == ch || '/' == ch) {}
			else if ('?' == ch)
			{
				buff.end = pCur;
				_path = buff;
				buff.begin = ++pCur;
				decodeState = HttpUrlDecodeState::QUERY_KEY;
			}
			else if (CR == ch)
			{
				buff.end = pCur;
				_path = buff;
				buff.begin = ++pCur;
				decodeState = HttpUrlDecodeState::COMPLETE;
			}
			else
				decodeState = HttpUrlDecodeState::INVALID;
			break;
		//参数键状态
		case HttpUrlDecodeState::QUERY_KEY:
			if (isalnum(ch) || '_' == ch) {}
			else if ('=' == ch)
			{
				buff.end = pCur;
				queryKey = buff;
				buff.begin = ++pCur;
				decodeState = HttpUrlDecodeState::QUERY_VALUE;
			}
			else
				decodeState = HttpUrlDecodeState::INVALID;
			break;
		//参数值状态
		case HttpUrlDecodeState::QUERY_VALUE: 
			if ('&' == ch)
			{
				buff.end = pCur;
				queryValue = buff;
				query.insert({ queryKey, queryValue });
				buff.begin = ++pCur;
				decodeState = HttpUrlDecodeState::QUERY_KEY;
			}
			else if ('#' == ch)
			{
				buff.end = pCur;
				queryValue = buff;
				query.insert({ queryKey, queryValue });
				_params = query;

				buff.begin = ++pCur;
				buff.end = pCur;
				decodeState = HttpUrlDecodeState::FRAGMENG;
			}
			else if (CR == ch)
			{
				buff.end = pCur;
				queryValue = buff;
				query.insert({ queryKey, queryValue });
				_params = query;
				decodeState = HttpUrlDecodeState::COMPLETE;
			}
			else if(!isblank(ch)) {}
			else
				decodeState = HttpUrlDecodeState::INVALID;
			break;
		//锚点
		case HttpUrlDecodeState::FRAGMENG:
			if (CR == ch)
			{
				buff.end = pCur;
				_fragment = buff;
				decodeState = HttpUrlDecodeState::COMPLETE;
			}
			else if (isblank(ch))
				decodeState = HttpUrlDecodeState::INVALID;
			else {}
			break;
		default:
			break;
		}
	}
	return decodeState == HttpUrlDecodeState::COMPLETE ? true : false;
}

