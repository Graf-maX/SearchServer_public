#include "request_queue.h"

using namespace std;

RequestQueue::RequestQueue(const SearchServer &search_server) : server_(search_server) {
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
    // Удаление старого запроса
    DelElement();
    // Добавление нового запроса
    vector<Document> result = server_.FindTopDocuments(raw_query, status);
    AddElement(raw_query, result);
    // Возвращение результатов поиска
    return result;
}

int RequestQueue::GetNoResultRequests() const {
    return count_empty_query_;
}

// Удаление устаревшего запроса
void RequestQueue::DelElement() {
    if (count_query_ == sec_in_day_) {
        if (requests_.front().is_empty) {
            --count_empty_query_;
        }
        requests_.pop_front();
    } else {
        ++count_query_;
    }
}

// Добавление нового запроса
void RequestQueue::AddElement(const std::string& raw_query, std::vector<Document>& result_search) {
    requests_.push_back({raw_query, result_search, result_search.empty()});
    if (result_search.empty()) {
        ++count_empty_query_;
    }
}
