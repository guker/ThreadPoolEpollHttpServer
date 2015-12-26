/*************************************************************************
    > File Name: thread_pool.h
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: 2015年12月23日 星期三 16时12分58秒
 ************************************************************************/
#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__
#include <string>
#include <set>
#include <map>
using namespace std;
#include <tcp_server.h>
#define MAXTHREAD 20
#define MAXTASK 1000
#include <sys/epoll.h>
#include <debug.h>
class Msg
{
public:
	enum MsgType{NewConn, Stop}type;
	union Data
	{
		int fd;
		void *ptr;
	}data;
};

class Epoll
{
public:
	Epoll();
	void addfd(epoll_event &e);
	void delfd(epoll_event &e);
	void poll();
	virtual void handle(epoll_event &e) = 0;
private:
	set<int> rfds;
	set<int> wfds;
	int epollfd;
	epoll_event events[MAXTASK];

};
template<typename TWorker>
class ThreadPool:public Epoll
{
private:
	static void* run_child(void *arg);

public:
	~ThreadPool();

	static ThreadPool *thread_pool_create(unsigned short port, int nthread = 8, string ip = "0.0.0.0");
private:
	virtual void handle(epoll_event &e);
	ThreadPool(int nthread, unsigned short port, string ip /*= "0.0.0.0"*/);
	int pipefd[MAXTHREAD][2];
	static ThreadPool *tp;
	int maxthread;
	TcpServer ts;
	int cur_worker;
};
template<typename TWorker>
ThreadPool<TWorker>::ThreadPool(int nthread, unsigned short port, string ip):ts(ip.c_str(), port), maxthread(nthread), cur_worker(0)
{
	epoll_event e;
	e.data.fd = ts.listen_sock.fd;
	e.events = EPOLLIN;
	addfd(e);

	for(int i = 0; i < maxthread; i++)
	{
		CHECK(pipe(pipefd[i]));
		pthread_t tid;
		printf("create thread %d.\n", i);
		TWorker *w = new TWorker(pipefd[i][0]);
		CHECK2(pthread_create(&tid, NULL, ThreadPool::run_child, w));
		pthread_detach(tid);
	}


}

template<typename TWorker>
ThreadPool<TWorker>* ThreadPool<TWorker>::thread_pool_create(unsigned short port, int nthread /* = 8 */, string ip /*= "0.0.0.0"*/)

{
	if(tp != NULL)
		return tp;
	if(nthread <= 0)
		return NULL;
	return tp = new ThreadPool<TWorker>(nthread, port, ip);

}

template<typename TWorker>
ThreadPool<TWorker> *ThreadPool<TWorker>::tp = NULL;

template<typename TWorker>
ThreadPool<TWorker>::~ThreadPool()
{
	if(tp != NULL)
		delete tp;
}

template<typename TWorker>
void* ThreadPool<TWorker>::run_child(void *arg)
{
	TWorker *w = (TWorker*)arg;
	w->run();
	return NULL;
}
template<typename TWorker>
void ThreadPool<TWorker>::handle(epoll_event &e)
{
	if(e.data.fd == ts.listen_sock.fd)
	{
		Msg msg;
		msg.type = Msg::NewConn;
		CHECK(msg.data.fd = ts.accept().fd);
		DEBUGMSG("accept a client:%d\n", msg.data.fd);
		CHECK(send(pipefd[cur_worker][0], &msg, sizeof(msg), 0) == sizeof(Msg));
	}
}
class Worker:public Epoll
{
public:
	Worker(int ppfd);
	void run();
private:
	virtual void handle(epoll_event &e);
	int pipefd;
};


#endif /*__THREAD_POOL_H__ */
