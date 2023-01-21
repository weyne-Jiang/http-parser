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
