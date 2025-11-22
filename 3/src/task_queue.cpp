#include "task_queue.h"

TaskQueue* TaskQueue::instance = nullptr;
std::mutex TaskQueue::instance_mutex;
