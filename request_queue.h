#pragma once
#include <queue>
#include <vector>
#include "document.h"
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer &search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status = DocumentStatus::ACTUAL);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        std::string query;
        std::vector<Document> res;
        bool is_empty;
    };

    // Приватные параметры класса
    std::deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    const SearchServer &server_;
    int count_query_ = 0;
    int count_empty_query_ = 0;

    // Удаление устаревшего запроса
    void DelElement();
    // Добавление нового запроса
    void AddElement(const std::string& raw_query, std::vector<Document>& result_search);
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    // Удаление старого запроса
    DelElement();
    // Добавление нового запроса
    std::vector<Document> result = server_.FindTopDocuments(raw_query, document_predicate);
    AddElement(raw_query, result);
    // Возвращение результатов поиска
    return result;
}
