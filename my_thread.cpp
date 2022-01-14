#include "my_thread.h"

int counter;
int N;   // #2. parameterize the number of threads
OllieThread* kurazy_threads = NULL;
boost::asio::io_service io;
boost::asio::io_service io_result;
boost::thread_group threadpool;
boost::thread_group threadpool_result;
boost::asio::io_service::work* work;
boost::asio::io_service::work* work_result;

void set_thread_number(int n) { N = n; }
void initialize_counter() { counter = 0; }
void print_counter() { printf("counter: %d\n", counter); }

void print_threads()
{
    //printf("check:\n");
    if(kurazy_threads == NULL) {
        printf("not yet generated.\n");
        return;
    }
	/*
    for(int i = 0; i < N; ++i) {
        SnapValue sv = kurazy_threads[i].read();
        printf("label: %d, value: %d, snap: ", sv.label, sv.value);
        std::cout << sv.snap << "\n";
    }
	*/
}

void gen_threads()
{
    if(kurazy_threads) {
        delete[] kurazy_threads;
        kurazy_threads = NULL;
    }
    kurazy_threads = new OllieThread[N];
    for(int i = 0; i < N; ++i) {
        kurazy_threads[i].set_tid(i);
        SnapValue sv(i+1, rand()%1000, NULL);
        kurazy_threads[i].write(sv);
    }

    //print_threads();
}

void set_boost_asio_io_services()
{
    work = new boost::asio::io_service::work(io);
    work_result = new boost::asio::io_service::work(io_result);
}

void threadpool_create()
{
    for (int i = 0; i < N; i++) {
        threadpool.create_thread(boost::bind(
                    &boost::asio::io_service::run, &io));
    }

    for (int i = 0; i < N; i++) {
        threadpool_result.create_thread(boost::bind(
                    &boost::asio::io_service::run, &io_result));
    }
	printf("threadpool creating is done.\n");
}

void io_post_update_thread_value(int tid)
{
    io.post(boost::bind(&update_thread_value, tid));
}

void delete_threadpool()
{
    delete work;
    threadpool.join_all();
    io.stop();

    delete work_result;
    threadpool_result.join_all();
    io_result.stop();
}

void goodbye_ollie()
{
    if(kurazy_threads) {
        delete[] kurazy_threads;
    }
    kurazy_threads = NULL;
}


// ====== SnapValue implementation =========//

SnapValue& SnapValue::operator=(const SnapValue& sv)
{
    this->label = sv.label;
    this->value = sv.value;
    if(this->snap) {
		delete[] snap;
	}
	if(sv.snap == NULL)	{
		this->snap = NULL;
		return *this;
	}
    this->snap = new int[N];
    for(int i = 0; i < N; ++i) {
        this->snap[i] = sv.snap[i];
    }
    return *this;
}

// Constructor 1
SnapValue::SnapValue(int l, int v, int* s) {
    label = l;
    value = v;
	if(s) {
		snap = new int[N];
		for(int i = 0; i < N; ++i) {
			snap[i] = s[i];
		}
	} else {
		snap = NULL;
	}
}

// Constructor 2
SnapValue::SnapValue(const SnapValue& sv) {
    label = sv.label;
    value = sv.value;
	if(sv.snap) {
		snap = new int[N];
		for(int i = 0; i < N; ++i) {
			snap[i] = sv.snap[i];
		}
	} else { snap = NULL; }
}

// == end == SnapValue implementation ======//

// ========= Ollie's implementation ======= //
SnapValue OllieThread::read()
{
	SnapValue sv(_sv_);
    return sv;
}

void OllieThread::write(const SnapValue& sv)
{
	_sv_.label = sv.label;
	_sv_.value = sv.value;
	if(sv.snap) {
		_sv_.snap = new int[N];
		for(int i = 0; i < N; ++i) {
			_sv_.snap[i] = sv.snap[i];
		}
	} else {
		_sv_.snap = NULL;
	}
}

// == end == Ollie's implementation ======= //

// ====== general functions =============== //
SnapValue* collect()                        // collect snapshots of all threads
{
    SnapValue* sv = new SnapValue[N];
    for(int i = 0; i < N; ++i) {
        sv[i] = kurazy_threads[i].read();   // operator= is actually available.
    }
    return sv;
}

int* get_values(const SnapValue* copy)
{
    int* vs = new int[N];
    for(int i = 0; i < N; ++i) {
        vs[i] = copy[i].value;
    }
    return vs;
}

int* scan()     // guarantee linearizability
{
    SnapValue* old_copy;
    SnapValue* new_copy;
    bool* moved = new bool[N];
    for(int i = 0; i < N; ++i)  moved[i] = false;
    old_copy = collect();
    while(1) {
LOOP_COLLECT:
        new_copy = collect();
        for (int i = 0; i < N; ++i) {
            if (old_copy[i].label != new_copy[i].label) { // detected change
                if (moved[i]) {     // already other thread passed.
                    break;
                } else {
                    moved[i] = true;
					if (old_copy)	delete[] old_copy;
                    old_copy = new_copy;
                    goto LOOP_COLLECT;
                }
            }
        }
        break;
    }

    int* ret = get_values(new_copy);
	if(moved) {
		delete[] moved;
		moved = NULL;
	}
	if(old_copy) {
		delete[] old_copy;
		old_copy = NULL;
	}
	if(new_copy) {
		delete[] new_copy;
		new_copy = NULL;
	}
    return ret;
}

void PrintOllie(int tid)
{
	printf("tid: %d\n", tid);
	if(kurazy_threads == NULL) {
		printf("???\n");
		return;
	}
	SnapValue sv = kurazy_threads[tid].read();
	if(sv.snap == NULL) {
		printf("snap is NULL.\n");
		return;
	}
	printf("label: %d, value: %d, snaps: ", sv.label, sv.value);
	for(int i = 0; i < N; ++i) {
		printf("%d ", sv.snap[i]);
	}
}

void update_thread_value(int tid)
{
    int value = rand() % 1000; // #1. random value for updating
    int* snap = scan();
	//printf("snap scanning is done.\n");
    SnapValue old_value = kurazy_threads[tid].read();
	//printf("old value initialization is done.\n");
    SnapValue new_value(old_value.label+1, value, snap);
	//printf("new value initialization is done.\n");
    kurazy_threads[tid].write(new_value);
	//printf("Writing new value to thread is done.\n");
    ++counter;	// #3. each thread increases counter variable.

	if(snap)	delete[] snap;
	snap = NULL;

	/* If you want to see the progress, remove the footprint symbol */
	//PrintOllie(tid);
	//io_result.post(boost::bind(&PrintOllie, tid));
}
