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