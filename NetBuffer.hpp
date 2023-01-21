#pragma once 

#include <iostream>
#include <cstring>

class NetBuffer {
public:
	//���캯�����������ߴ� ��չ����
	NetBuffer(uint32_t maxsize = 256, uint32_t expendfac = 2);
	~NetBuffer();		

	//��ʼ��
	bool init();

	//д������
	void writeBuffer(char* pSrc, uint32_t size);		

	//��������
	uint32_t readBuffer(char* pDst, int size);	

	//��ջ�����
	inline void cleanBuffer()
	{
		_readIndex = _writeIndex;
	}

	//��ȡ����������
	inline uint32_t getSize()
	{
		return _writeIndex - _readIndex;
	}

private:
	//��������λ����������ͷ
	void adjustBuffer();	

	//��չ����������
	void expandBuffer(uint32_t need_size);		

private:
	char* _pBuff; //������ָ��
	uint32_t _readIndex; //��ָ��λ��
	uint32_t _writeIndex; //дָ��λ��
	uint32_t _maxSize; //��������󳤶�
	uint32_t _expandFac; //��չ����
};
