#ifndef TASK_H
#define TASK_H

#include <iostream>

class Task {
private:
    int id;
public:
    Task(int id) : id(id) {}
    virtual ~Task() = default;
    virtual void execute() = 0;
    int getId() const { return id; }
};

#endif // TASK_H
