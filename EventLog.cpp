#include "EventLog.h"

EventLog::EventLog(size_t capacity) : maxEntries(capacity) {}

void EventLog::addEntry(const std::string& entry) {
    entries.push_front(entry);
    while (entries.size() > maxEntries) {
        entries.pop_back();
    }
}

std::vector<std::string> EventLog::getEntries() const {
    return std::vector<std::string>(entries.begin(), entries.end());
}
