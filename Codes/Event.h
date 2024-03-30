#ifndef EVENT_H
#define EVENT_H

#include <string>

using namespace std;

struct Event {
    int time;
    string signal;
    int value;
    Event(int t, string s, int v) : time(t), signal(s), value(v) {}

    // For priority_queue to sort events in ascending order of time
    bool operator>(const Event& other) const {
        return time > other.time;
    }
};

#endif // EVENT_H
