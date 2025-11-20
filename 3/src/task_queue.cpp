#include "task_queue.h"

// Define the static member variables
TaskQueue* TaskQueue::instance = nullptr;
std::mutex TaskQueue::instance_mutex;
