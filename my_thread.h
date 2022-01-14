#ifndef MY_THREAD_H_INCLUDED
#define MY_THREAD_H_INCLUDED

#include <iostream>
#include <pthread.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <memory>

void set_thread_number(int);
void initialize_counter();
void print_threads();
void gen_threads();
void print_counter();
void set_boost_asio_io_services();
void threadpool_create();
void io_post_update_thread_value(int);
void delete_threadpool();
void goodbye_ollie();   // free memory allocation of threads

struct SnapValue {
public:
    SnapValue() { snap = NULL; }
    SnapValue(int l, int v, int* s);    
	SnapValue(const SnapValue& sv);
    ~SnapValue() {
        if(snap)	delete[] snap;
        snap = NULL;
    }
    SnapValue& operator=(const SnapValue&);
    int label;
    int value;
    int* snap;
};

class OllieThread {    // just name of thread
    // methods
public:
    OllieThread() {}
    OllieThread(int tid) : _tid_(tid) {}
    SnapValue read();
    void write(const SnapValue&);
	void set_tid(int tid) { _tid_ = tid; }
	int get_tid()	{ return _tid_; }

    // member instances
private:
    SnapValue _sv_;
    int _tid_;
};

SnapValue* collect();               // collect snapshots of all threads
int* get_values(const SnapValue*);
int* scan();
void update_thread_value(int tid);  // #1. update each thread's local variable

#endif  // MY_THREAD_H_INCLUDED
