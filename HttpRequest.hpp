#pragma once
#include <iostream>
#include <string>
#include <map>
#include <optional>

struct StringBuffer
{
	char* begin = NULL;//�ַ�����ʼλ��
	char* end = NULL;//�ַ�������λ��

	operator std::string() const
	{
		return std::string(begin, end);
	}
};

//Http���������״̬
enum class HttpUrlDecodeState
{
	COMPLETE = 0, //���
	INVALID,//��Ч
	START,//��Ӧ�п�ʼ
	PROTOCOL,//Э��
	FIRSTSLAP, 
	SECONDSLAP,
	IP, //IP��ַ
	PORT, //�˿ں�
	PATH, //·��
	QUERY_KEY, //������
	QUERY_VALUE, //����ֵ
	FRAGMENG //��ϢƬ��
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

	//�������󷽷�
	void setMethod(std::string method);

	//��������ֶΣ��ظ���ӻḲ��				
	void addField(std::string key, std::string value);

	//����URL
	bool decodeUrl(std::string url);

	//��ȡ����ͷ
	const std::string getRequest();

	//��ȡip
	inline std::string getIp()
	{
		return _ip;
	}

	//��ȡ�˿�
	inline int getPort()
	{
		return _port;
	}

private:
	std::string _ip; //ip
	int _port; //�˿�
	std::string _method = "GET"; // ���󷽷�
	std::string _path; // ·��
	std::optional<std::map<std::string, std::string>> _params ; // ����
	std::string _protocol; // Э��
	std::string _verison = "1.1"; //Э��汾
	std::map<std::string, std::string> _headerLine; // ����ͷ�ֶ���Ϣ
	std::string _fragment; //��Ϣ�ֶ�
};