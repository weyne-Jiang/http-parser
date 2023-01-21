#include "HttpRequest.hpp"
#define CR '\r'

HttpRequest::HttpRequest(std::string url)
{
	if (!decodeUrl(url))
		std::cerr <<"URL����ʧ��\n";
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

//�������󷽷�
void HttpRequest::setMethod(std::string method)
{
	_method = method;
}

void HttpRequest::addField(std::string key, std::string value)
{
	_headerLine[key] = value;
}

//��ȡ����ͷ
const std::string HttpRequest::getRequest()
{
	std::string requestStr;
	//�����һ����Ϣ
	requestStr += _method
		+ ' '
		+ '/'
		+ _path;
	//���жϲ����Ƿ���� 
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
	//ƴ��Э��
	requestStr += ' '
		+ _protocol
		+ '/'
		+ _verison
		+ "\r\n";
	//ƴ���ֶ�
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

//����URL
bool HttpRequest::decodeUrl(std::string url)
{
	StringBuffer buff;
	std::string queryKey;
	std::string queryValue;
	std::map<std::string, std::string> query;
	HttpUrlDecodeState decodeState = HttpUrlDecodeState::START;
	//��ӿո�Ϊ������
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
		//Э��״̬
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
		//��һ��б��
		case HttpUrlDecodeState::FIRSTSLAP:
			if ('/' == ch)
				decodeState = HttpUrlDecodeState::SECONDSLAP;
			else
				decodeState = HttpUrlDecodeState::INVALID;
			break;
		//�ڶ���б��
		case HttpUrlDecodeState::SECONDSLAP:
			if ('/' == ch)
			{
				buff.begin = ++pCur;
				decodeState = HttpUrlDecodeState::IP;
			}
			else
				decodeState = HttpUrlDecodeState::INVALID;
			break;
		//IP״̬
		case HttpUrlDecodeState::IP:
			//�˴�ip�ж�Ҳ���Ǻ��Ͻ�����˼��˼��
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
		//�˿�״̬
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
		//·��״̬
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
		//������״̬
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
		//����ֵ״̬
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
		//ê��
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

