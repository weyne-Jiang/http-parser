#include "HttpController.hpp"
//#include "RingBuffer.hpp"
#include <assert.h>
#include <chrono>
using namespace std::chrono;
using namespace std;
void httpTest()
{
	HttpController* phttp = HttpController::getInstence();
	
	HttpRequest req("http://127.0.0.1:80/");
	req.addField("Connection", "close");

	for (size_t i = 0; i < 100*100*5; i++)
	{
		HttpResponse resp;
		printf("**********************这是第 %d 次请求*************************\n", i);
		phttp->sendHttpRequest(req, resp);
	}
}

int main(void)
{
	auto start = system_clock::now();
	httpTest();
	auto end = system_clock::now();
	auto cost = duration_cast<milliseconds>(end - start);
	cout << "用时 ： " << double(cost.count()) << "毫秒" << endl;
	system("pause");
	return 0;
}