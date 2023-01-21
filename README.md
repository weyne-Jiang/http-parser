目录

* 1. HTTP协议详解
  
  * 1.1 HTTP协议介绍
  
  * 1.2 HTTP报文格式
  
  * 1.3 HTTP工作过程
  
  * 1.4 HTTP请求
  
  * 1.5 HTTP响应
  
* 2. 有限状态自动机解析实例
  
  * 2.1 解析URL
  
  * 2.2 解析HTTP响应
  
* 3. 网络缓冲区
  
  * 3.1 网络缓冲区的目的
  
  * 3.2 网络缓冲区选型与设计
  
  * 3.3 缓冲区实现代码
  
  * 3.4 性能对比

# 1. HTTP协议详解
## 1.1 HTTP协议介绍
HTTP（Hyper Text Transfer Protocol）： 全称超文本传输协议，是用于从万维网（WWW:World Wide Web ）服务器传输超文本到本地浏览器的传送协议。

HTTP 是一种应用层协议，是基于 TCP/IP 通信协议来传递数据的，其中 HTTP1.0、HTTP1.1、HTTP2.0 均为 TCP 实现，HTTP3.0 基于 UDP 实现。现主流使用 HTTP1.0 和 HTTP3.0

协议： 为了使数据在网络上从源头到达目的，网络通信的参与方必须遵循相同的规则，这套规则称为协议，它最终体现为在网络上传输的数据包的格式

## 1.2 HTTP报文格式
 HTTP请求报文主要由四部分组成，分别为请求行、请求头、空行、请求体。每一个字段结束为回车符+换行符，即/r/n；请求头和请求体的结束为两个连续的/r/n。

![http报文格式](https://img-blog.csdnimg.cn/3a7965c578ae4c728b1405b0401250b3.png#pic_center)
* 请求行：用于描述客户端的请求方式（GET/POST等），请求的资源名称(URL)以及使用的HTTP协议的版本号。
它们用空格分隔。例如，GET /index.html HTTP/1.1。
HTTP协议的请求方法包括GET、HEAD、PUT、POST、TRACE、OPTIONS、DELETE等，详见[官方文档](https://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html)。
GET是最常见的一种请求方式，当客户端要从服务器中读取文档时，当点击网页上的链接或者通过在浏览器的地址栏输入网址来浏览网页的，使用的都是GET方式。GET方法要求服务器将URL定位的资源放在响应报文的数据部分，回送给客户端。 POST方法可以允许客户端给服务器提供信息较多。POST方法将请求参数封装在HTTP请求数据中，以名称/值的形式出现，可以传输大量数据，这样POST方式对传送的数据大小没有限制，而且也不会显示在URL中，对隐私数据保密性更好。 HEAD就像GET，只不过服务端接受到HEAD请求后只返回响应头，而不会发送响应内容。当我们只需要查看某个页面的状态的时候，使用HEAD是非常高效的，因为在传输的过程中省去了页面内容。

* 请求头：请求头部由关键字/值对组成，每行一对，关键字和值用英文冒号“:”分隔。请求头部通知服务器有关于客户端请求的信息。

* 空行：最后一个请求头之后是一个空行，发送回车符和换行符，通知服务器以下不再有请求头。

* 请求体：当使用POST等方法时，通常需要客户端向服务器传递数据。这些数据就储存在请求正文中（GET方式是保存在url地址后面，不会放到这里）。POST方法适用于需要客户填写表单的场合。与请求数据相关的最常使用的请求头是Content-Type和Content-Length。
## 1.3 HTTP工作过程
当我们在浏览器输入一个网址，此时浏览器就会给对应的服务器发送一个 HTTP 请求，对应的服务器收到这个请求之后，经过计算处理，就会返回一个 HTTP 响应。并且当我们访问一个网站时，可能涉及不止一次的 HTTP 请求和响应的交互过程。具体过程如下
1. 浏览器进行DNS域名解析，得到对应的IP地址
2. 根据这个IP，找到对应的服务器建立连接（三次握手）
3. 建立TCP连接后发起HTTP请求（一个完整的http请求报文）
4. 服务器响应HTTP请求，浏览器得到html代码（服务器如何响应）
5. 浏览器解析html代码，并请求html代码中的资源（如js、css、图片等）
6. 浏览器对页面进行渲染呈现给用户
7. 服务器关闭TCP连接（四次挥手）

## 1.4 HTTP请求
对于HTTP请求的解析，主要是对URL的解析，然后通过解析内容构建请求报文。

![](https://img-blog.csdnimg.cn/f3a1d7bd47d04c79bbacacacd1ca829b.png#pic_center)
请求流程大致如下
1. 解析URL，获取协议、IP、端口、路径、参数、锚点。
2. 通过IP与端口，和服务端建立TCP连接。
3. 构建并发送HTTP请求报文。
4. 等待响应，待响应解析后决定后续操作。

url格式大致如下
protocol://hostname:[port]/path?[query_key1]=[query_value1]&[query_key2]=[query_value2]#fragment

常用字段如下
请求头字段 | 说明
----   |--------
Host | 请求的目标IP地址端口或者域名
Accept	|告诉服务端客户端接受什么类型的响应
Content-Length	|表示请求体的长度
Cookie	|传递的cookie
User-Agent	|发送请求的应用程序信息
Connection|keep-alive或者closed，表示是否保持连接
## 1.5 HTTP响应
HTTP响应报文跟请求报文相似，也由四部分组成，分别为响应行、响应头、空行、响应体。

![在这里插入图片描述](https://img-blog.csdnimg.cn/a74d4c2efcca4863a909a6fd84f5a772.png#pic_center)
响应解析流程大致如下
1. 接受响应，并解析响应行，确认协议和状态码。
2. 解析响应头和响应体。（需根据Transfer-Encoding确定响应体具体解析方式）
3. 根据Connection状态确定后续操作。

常用字段如下
响应头字段 | 说明
----   |--------
Accept	|告诉服务端客户端接受什么类型的响应
Content-Length	|表示响应体的长度
Content-Encoding | 内容编码
Transfer-Encoding | 传输编码
Connection|keep-alive或者closed，表示是否保持连接

# 2. 有限状态自动机解析实例
有限状态自动机拥有有限数量的状态，每个状态可以迁移到零个或多个状态，输入字符决定执行哪个状态的迁移。有限状态自动机可以表示为一个有向图，非常适合用来做协议的解析。
项目完整代码   [weyne-Jiang/http-parser](https://github.com/weyne-Jiang/http-parser)
## 2.1 解析URL
URL解析状态迁移图大致如下
![C++使用有限状态自动机编程解析HTTP协议](https://img-blog.csdnimg.cn/20210219220539591.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM5NTE5MDE0,size_16,color_FFFFFF,t_70#pic_center)

以下为项目中URL解析的自动机部分
```cpp
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
			if (isalnum(ch) || '_' == ch) {}
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
```
## 2.2 解析HTTP响应
相较于请求头的解析，响应体的解析略微繁琐。需要判断Transfer-Encoding传输编码形式。  

1. 如果响应头中已有content-length，即已经声明了响应体长度，直接读取网络缓冲区中对应长度数据即可。
2. 如果未发现content-length，且Transfer-Encoding为chunked，即为分块传输形式。需要交替读取chunk长度块和数据块。

对于chunked编码，是以16进制长度+数据内容为一个单元，多个单元组合，最后以长度为0的单元结束。形式如下

```cpp
[chunk块长度][CRLF][chunk块数据][CRLF]......[chunk块长度][CRLF][chunk块数据][CRLF][长度0][CRLF][footer内容][CRLF]
```
chunked编码具体详解可见[HTTP 进阶之 chunked 编码](http://www.paradeto.com/2020/01/01/http-advance-chunked/)

以下为项目中HTTP响应体解析的自动机部分
```cpp
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
```

# 3. 网络缓冲区
## 3.1网络缓冲区的目的
具体来说网络缓冲区包括读（接收）缓冲区和写（发送）缓冲区。设计缓冲区的目的
1. 设计读缓冲区的目的：当从TCP中读数据时，不能确定读到的是一个完整的数据包，如果是不完整的数据包，需要先放入缓冲区中进行缓存，直到数据包完整才进行业务处理。
设计写缓冲区的目的：向TCP写数据不能保证所有数据写成功，如果TCP写缓冲区已满，则会丢弃数据包，所以需要一个写缓冲区暂时存储需要写的数据。
2. 由于在进行HTTP响应解析时，并不确定需要读TCP内核缓冲区的具体长度，故比较稳妥的方式是一个一个字符地读取，完成响应头解析后，根据传输编码采用不同方式读取响应体内容。但此方式会出现用户态和内核态的频繁转换，上下文切换的开销巨大。故可加入网络缓冲区进行数据暂存，解析时从用户态的网络缓冲区中读取数据，避免频繁切换到内核态。

![网络缓冲区和内核中TCP缓冲区的关系](https://img-blog.csdnimg.cn/86901e4720fa49b5bd69c9065a0694fc.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBA5piv6Zi_5q-b5ZWK,size_20,color_FFFFFF,t_70,g_se,x_16)
## 3.2 网络缓冲区选型与设计
考虑可使用环形缓冲区和线性可扩展缓冲区。由于环形缓冲区满时空间扩展繁琐，故使用线性可扩展缓冲区。
下图设计了一种线性可扩展的读写缓冲区，初始化时预分配了一片连续的内存空间作为缓冲区区域。缓冲区分为数据空间和预留空间。相较于环形缓冲区，扩容简单，但也为了最大化利用空间，会将数据移动至头部，故移动操作会损耗一定性能。

![可扩展缓冲区设计](https://img-blog.csdnimg.cn/f564c6e78959495aa05909d88aed77c2.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBA5piv6Zi_5q-b5ZWK,size_20,color_FFFFFF,t_70,g_se,x_16)

## 3.3 缓冲区实现代码

NetBuffer.hpp
```cpp
#pragma once 

#include <iostream>
#include <cstring>

class NetBuffer {
public:
	//构造函数，参数最大尺寸 扩展因子
	NetBuffer(uint32_t maxsize = 256, uint32_t expendfac = 2);
	~NetBuffer();		

	//初始化
	bool init();

	//写缓冲区
	void writeBuffer(char* pSrc, uint32_t size);		

	//读缓冲区
	uint32_t readBuffer(char* pDst, int size);	

	//清空缓冲区
	inline void cleanBuffer()
	{
		_readIndex = _writeIndex;
	}

	//获取缓冲区长度
	inline uint32_t getSize()
	{
		return _writeIndex - _readIndex;
	}

private:
	//调整数据位置至缓冲区头
	void adjustBuffer();	

	//扩展缓冲区长度
	void expandBuffer(uint32_t need_size);		

private:
	char* _pBuff; //缓冲区指针
	uint32_t _readIndex; //读指针位置
	uint32_t _writeIndex; //写指针位置
	uint32_t _maxSize; //缓冲区最大长度
	uint32_t _expandFac; //扩展因子
};

```
NetBuffer.cpp
```cpp
#include "NetBuffer.hpp"

NetBuffer::NetBuffer(uint32_t maxsize, uint32_t expendfac) : _readIndex(0), _writeIndex(0), _expandFac(expendfac) , _maxSize(maxsize)
{
}

NetBuffer::~NetBuffer()
{
	delete[] _pBuff;
	_pBuff = nullptr;
}

bool NetBuffer::init()
{
	_pBuff = new char[_maxSize]();
	if (nullptr == _pBuff)
	{
		std::cerr << "缓冲区内存分配失败\n";
		return false;
	}
	return true;
}

void NetBuffer::writeBuffer(char* pSrc, uint32_t size)
{
	uint32_t usedSize = _writeIndex - _readIndex;
	uint32_t remainSize = _maxSize - usedSize;
	uint32_t writableSize = _maxSize - _writeIndex;
	if (size > remainSize)
	{
		//剩余总空间不足
		adjustBuffer();
		expandBuffer(size);
	}
	else if (size > writableSize)
	{
		//剩余总空间足够，剩余可写空间不足
		adjustBuffer();
	}

	//写入数据
	memcpy(_pBuff + _writeIndex, pSrc, size);
	_writeIndex += size;
}

uint32_t NetBuffer::readBuffer(char* pDst, int size)
{
	uint32_t readableSize = _writeIndex - _readIndex;
	if (0 == size || 0 == readableSize)
		return 0;
	if (size < 0)
	{
		//全部读取
		memcpy(pDst, _pBuff + _readIndex, readableSize);
		_readIndex += readableSize;
		return readableSize;
	}
	else if (size > 0)
	{
		//按指定长度读取
		if ((uint32_t)size > readableSize)
			size = readableSize; //指定长度大于可读长度
		memcpy(pDst, _pBuff + _readIndex, size);
		_readIndex += size;
	}
	return size;
}

void NetBuffer::adjustBuffer()
{
	if (0 == _readIndex)
		return;
	uint32_t usedSize = _writeIndex - _readIndex;

	if (0 == usedSize)
	{
		//此时读写指针重合，无已用数据
		_readIndex = 0;
		_writeIndex = 0;
	}
	else
	{
		//调整数据到缓冲区头部(同一块连续内存空间拷贝到自身，数据重合是否会出错？)
		memcpy(_pBuff, _pBuff + _readIndex, usedSize);
		_writeIndex -= _readIndex;
		_readIndex = 0;
	}
}

void NetBuffer::expandBuffer(uint32_t needSize)
{
	uint32_t usedSize = _writeIndex - _readIndex;
	uint32_t remaindSize = _maxSize - usedSize;
	uint32_t expendSize = _maxSize;

	while (needSize > remaindSize)
	{
		if (needSize > _maxSize)
		{
			//大于最大尺寸，扩容到需求尺寸
			expendSize = needSize;
			break;
		}
		else
		{
			// 大于剩余尺寸，maxSize * 扩容因子
			expendSize *= _expandFac;
			remaindSize = expendSize - usedSize;
		}
	} 

	//申请新空间
	char* newBuff = new char[expendSize]();
	memcpy(newBuff, _pBuff + _readIndex, usedSize);
	delete[] _pBuff;
	_pBuff = newBuff;
	_maxSize = expendSize;
}
```
## 3.4 性能对比
测试采用nginx案例服务器，在本地环回分别连续请求5000次，30000次，50000次请求。
忽略网络延迟因素（当然实际不能忽略，节目效果谢谢），相较于直接从内核缓冲区中读取并解析，加入了网络缓冲区的版本有了巨大性能提升。在50000次请求中耗时减少一半。

无缓冲区测试结果
![无缓冲区测试结果](https://img-blog.csdnimg.cn/aa17f70fa97e42f9a02b69b65e556239.jpeg#pic_center)
<br>
加入缓冲区测试结果
![加入缓冲区测试结果](https://img-blog.csdnimg.cn/4e5daec6b7334c3c8ac0ca6f1de8bdd2.jpeg#pic_center)

文章摘要
1. [【计算机网络】HTTP 协议详解](https://blog.csdn.net/weixin_51367845/article/details/123313047)
2. [C++使用有限状态自动机编程解析HTTP协议](https://blog.csdn.net/qq_39519014/article/details/112317112)
3. [一次完整的HTTP请求过程是怎么样的呢？【图文详解】](https://zhuanlan.zhihu.com/p/161560683)
4. [Day057--django](http://t.zoukankan.com/surasun-p-10022522.html)
5. [HTTP 进阶之 chunked 编码](http://www.paradeto.com/2020/01/01/http-advance-chunked/)
6. [c++实现一个简易的网络缓冲区](https://blog.csdn.net/shi_xiao_xuan/article/details/122054839)
7. [weyne-Jiang/http-parser](https://github.com/weyne-Jiang/http-parser)

