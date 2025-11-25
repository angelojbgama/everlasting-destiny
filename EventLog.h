#ifndef EVENTLOG_H
#define EVENTLOG_H

#include <deque>
#include <string>
#include <vector>

class EventLog {
public:
    explicit EventLog(size_t capacity = 12);

    void addEntry(const std::string& entry);
    std::vector<std::string> getEntries() const;

private:
    size_t maxEntries;
    std::deque<std::string> entries;
};

#endif
