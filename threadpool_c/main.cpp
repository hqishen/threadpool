
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

typedef struct task_t {
    void (*function)(void *arg);
    void *arg;
} Task;

struct ThreadPool {
    Task * task;
    int queueCapacity;
    int queueSize;
    int queueFront;
    int queueRear;

    pthread_t managerTid;
    pthread_t *workerTIDs;

    int minNum;
    int maxNum;
    int busyNum;
    int liveNum;
    int exitNum;

    pthread_cond_t notFull;
    pthread_cond_t notEmpty;
    pthread_mutex_t mutexPool;
    pthread_mutex_t mutexBusy;

    bool isShutdown = false;
};

ThreadPool* threadpoolCreate(int min, int max, int queueSize);

bool threadpoolAddTask(ThreadPool *tp, void (*func)(void *), void *argc);

int getThreadPoolBusyNum(ThreadPool *tp);

int getThreadPoolAliveNum(ThreadPool *tp);

void threadpoolDestroy(ThreadPool *tp);

void *workerTask(void *arg);

void *managerTask(void *arg);

void threadExit(ThreadPool *tp);

ThreadPool * threadpoolCreate(int min, int max, int queueSize) {
    if (max > queueSize || min > max) {
        perror("init queue error");
        return NULL;
    }

    ThreadPool * tp = (ThreadPool *)malloc(sizeof(ThreadPool));
    do {
        assert(tp);

        tp->queueFront = 0;
        tp->queueRear = 0;
        tp->queueSize = 0;
        tp->queueCapacity = queueSize;
        tp->minNum = min;
        tp->maxNum = max;
        tp->busyNum = 0;
        tp->liveNum = min;
        tp->exitNum = 0;

        tp->task = (Task*)malloc(sizeof(Task) * queueSize);
        if (tp->task == nullptr) {
            break;
        }
        tp->workerTIDs = (pthread_t*)malloc(sizeof(pthread_t) * max);
        if(tp->workerTIDs == nullptr) {
            break;
        }

        if (pthread_mutex_init(&tp->mutexBusy, NULL) ||
        pthread_mutex_init(&tp->mutexPool, NULL) ||
        pthread_cond_init(&tp->notEmpty, NULL) ||
        pthread_cond_init(&tp->notFull, NULL) ) {
            break;
        }

        pthread_create(&tp->managerTid, NULL, managerTask, tp);
        for (int i = 0; i < max; ++i) {
            pthread_create(&tp->workerTIDs[i], NULL, workerTask, tp);
        }

        return tp;

    } while(0);

    if (tp && tp->workerTIDs) {
        free(tp->workerTIDs);
    }
    if (tp && tp->task) {
        free(tp->task);
    }
    if (tp)
        free(tp);

    return nullptr;
}

void threadpoolDestroy(ThreadPool *tp) {
    if (tp == nullptr) {
        return;
    }

    tp->isShutdown = true;
    pthread_join(tp->managerTid, NULL);
    for (int i = 0; i < tp->liveNum; ++i) {
        pthread_cond_signal(&tp->notEmpty);
    }

    if (tp->workerTIDs)  {
        free(tp->workerTIDs);
    }
    if (tp->task) {
        free(tp->task);
    }
    pthread_mutex_destroy(&tp->mutexPool);
    pthread_mutex_destroy(&tp->mutexBusy);
    pthread_cond_destroy(&tp->notEmpty);
    pthread_cond_destroy(&tp->notFull);
    free(tp);
    tp = nullptr;
}

int getThreadPoolBusyNum(ThreadPool *tp) {
    pthread_mutex_lock(&tp->mutexBusy);
    int num = tp->busyNum;
    pthread_mutex_unlock(&tp->mutexBusy);
    return num;
}

int getThreadPoolAliveNum(ThreadPool *tp) {
    pthread_mutex_lock(&tp->mutexPool);
    int num= tp->liveNum;
    pthread_mutex_unlock(&tp->mutexPool);
    return num;
}

bool threadpoolAddTask(ThreadPool *tp, void(*func)(void *), void *arg) {

    pthread_mutex_lock(&tp->mutexPool);
    while(tp->queueSize >= tp->queueCapacity && !tp->isShutdown) {
        std::cout << "add task ????????????????????????" << std::endl;
        pthread_cond_wait(&tp->notFull, &tp->mutexPool);
    }
    if (tp->isShutdown) {
        pthread_mutex_unlock(&tp->mutexPool);
        return true;
    }
    tp->task[tp->queueRear].function = func;
    tp->task[tp->queueRear].arg = arg;
    tp->queueRear = (tp->queueRear + 1 ) % tp->queueCapacity;
    tp->queueSize++;
    pthread_cond_signal(&tp->notEmpty); //??????worker??????????????????
    pthread_mutex_unlock(&tp->mutexPool);

    return true;
}

void *managerTask(void *arg){
    ThreadPool *tp = (ThreadPool*)arg;

    while(!tp->isShutdown) {
        sleep(1);
        //??????????????????????????????
        pthread_mutex_lock(&tp->mutexPool);
        int queueSize = tp->queueSize; // add task ??????????????????
        int liveNum = tp->liveNum; //???????????????????????????
        pthread_mutex_unlock(&tp->mutexPool);

        pthread_mutex_lock(&tp->mutexBusy);
        int busyNum = tp->busyNum;
        pthread_mutex_unlock(&tp->mutexBusy);

        // ?????????????????????????????????????????????????????????
        if (queueSize > liveNum && liveNum < tp->maxNum) {
            pthread_mutex_lock(&tp->mutexPool); //????????????
            for (int i = 0; i < tp->maxNum && tp->liveNum < tp->maxNum; ++i) {
                if (tp->workerTIDs[i] == 0) {
                    pthread_create(&tp->workerTIDs[i], NULL, workerTask, tp);
                    tp->liveNum++;
                }
            }
            pthread_mutex_unlock(&tp->mutexPool);
        }

        //?????????????????????
        if (busyNum * 2 < liveNum && liveNum > tp->minNum) {
            pthread_mutex_lock(&tp->mutexPool);
            tp->exitNum = 2;
            pthread_mutex_unlock(&tp->mutexPool);
            //???????????????????????????
            for (int i = 0; i < 2 ; ++i) {
                pthread_cond_signal(&tp->notEmpty);
            }
        }
    }

    return NULL;
}

void *workerTask(void *arg) { //?????????????????????????????????????????????????????????
    ThreadPool *tp = (ThreadPool*)arg;

    while(true) {
        pthread_mutex_lock(&tp->mutexPool);

        while(tp->queueSize == 0 && tp->isShutdown == false) {
            // ?????????????????????
            pthread_cond_wait(&tp->notEmpty, &tp->mutexPool); //??????????????????

            if (tp->exitNum) { //?????????????????????????????? todo
                tp->exitNum--;
                if (tp->exitNum > tp->minNum) {
                    tp->liveNum--;
                    pthread_mutex_unlock(&tp->mutexPool);
                    std::cout << "???????????????...." << std::endl;
                    threadExit(tp);

                }
            }
        }
        //???????????????????????????????????????
        if (tp->isShutdown == true) {
            pthread_mutex_unlock(&tp->mutexPool);
            threadExit(tp);
            break;
        }

        Task task;
        task.function = tp->task[tp->queueFront].function;
        task.arg = tp->task[tp->queueFront].arg;

        //???????????????
        tp->queueFront = (tp->queueFront + 1) % tp->queueCapacity;
        tp->queueSize--;

        pthread_cond_signal(&tp->notFull); // ??????addtask ?????????????????????????????????
        pthread_mutex_unlock(&tp->mutexPool);

        std::cout << "?????????????????? tid:" << pthread_self() << std::endl;

        pthread_mutex_lock(&tp->mutexBusy);
        tp->busyNum++;
        pthread_mutex_unlock(&tp->mutexBusy);
        task.function(task.arg);
        std::cout << "?????????????????? tid:" << pthread_self() << std::endl;
        free(task.arg);
        task.arg = NULL;

        pthread_mutex_lock(&tp->mutexBusy);
        tp->busyNum--;
        pthread_mutex_unlock(&tp->mutexBusy);

    }
    return NULL;
}


void threadExit(ThreadPool *tp) {
    pthread_t tid = pthread_self();
    for (int i = 0; i < tp->maxNum; ++i) {
        if (tp->workerTIDs[i] == tid) {
            tp->workerTIDs[i] = 0;
            //??????????????????
            break;
        }
    }
    pthread_exit(NULL);
}


// ??????
void testTask(void *arg) {
    int num = *(int*)arg;
    sleep(20);
    std::cout << "data value :" << num << " tid:" << pthread_self() << std::endl;

}

void test(void) {
    ThreadPool * tp = threadpoolCreate(5, 8, 100);
    for (int i = 0; i < 100; ++i) {
        int * data = (int *)malloc(sizeof(int));
        *data = i * 2;
        threadpoolAddTask(tp, testTask, (void *)data);
        sleep(1);
    }

    sleep(1000);
    threadpoolDestroy(tp);
}

int main() {
    test();

    return 0;
}
