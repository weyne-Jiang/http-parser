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
		std::cerr << "�������ڴ����ʧ��\n";
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
		//ʣ���ܿռ䲻��
		adjustBuffer();
		expandBuffer(size);
	}
	else if (size > writableSize)
	{
		//ʣ���ܿռ��㹻��ʣ���д�ռ䲻��
		adjustBuffer();
	}

	//д������
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
		//ȫ����ȡ
		memcpy(pDst, _pBuff + _readIndex, readableSize);
		_readIndex += readableSize;
		return readableSize;
	}
	else if (size > 0)
	{
		//��ָ�����ȶ�ȡ
		if ((uint32_t)size > readableSize)
			size = readableSize; //ָ�����ȴ��ڿɶ�����
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
		//��ʱ��дָ���غϣ�����������
		_readIndex = 0;
		_writeIndex = 0;
	}
	else
	{
		//�������ݵ�������ͷ��(ͬһ�������ڴ�ռ俽�������������غ��Ƿ�����)
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
			//�������ߴ磬���ݵ�����ߴ�
			expendSize = needSize;
			break;
		}
		else
		{
			// ����ʣ��ߴ磬maxSize * ��������
			expendSize *= _expandFac;
			remaindSize = expendSize - usedSize;
		}
	} 

	//�����¿ռ�
	char* newBuff = new char[expendSize]();
	memcpy(newBuff, _pBuff + _readIndex, usedSize);
	delete[] _pBuff;
	_pBuff = newBuff;
	_maxSize = expendSize;
}