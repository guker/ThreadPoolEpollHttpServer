/*************************************************************************
    > File Name: main.cpp
    > Author: wyg
    > Mail: wyg_0802@126.com
    > Created Time: 2015年12月23日 星期三 16时07分33秒
 ************************************************************************/
#include <iostream>
#include <thread_pool.h>
#include <inc.h>
#include <debug.h>
using namespace std;
int main()
{
	cout<<"Hello world!"<<endl;
	ThreadPool<Worker> *th = ThreadPool<Worker>::thread_pool_create(12345, 4);
	th->poll();
	return 0;
}
