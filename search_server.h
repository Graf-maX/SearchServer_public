#pragma once

#include "document.h"
#include "Lib/concurrent_map.h"

#include <algorithm>
#include <cmath>
#include <execution>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <iterator>

#define N_BUCKETS 100

class SearchServer {
public:
    /// Конструкторы класса
    SearchServer() = default;

    explicit SearchServer(const std::string& stop_words);
    explicit SearchServer(const std::string_view stop_words);

    template <typename StringCollection>
    explicit SearchServer(const StringCollection& collection);

    // Добавление документа на сервер
    template<typename Str>
    void AddDocument(const int document_id, const Str& document,
                     DocumentStatus status, const std::vector<int>& ratings);
    void AddDocument(const int document_id, std::string_view document,
                     DocumentStatus status, const std::vector<int>& ratings);

    // Поиск наиболее релевантных документов
    template <typename StatusFilter>
    std::vector<Document> FindTopDocuments(std::string_view raw_query,
                                           StatusFilter status) const;
    std::vector<Document> FindTopDocuments(std::string_view raw_query,
                          const DocumentStatus document_status = DocumentStatus::ACTUAL) const;
    template <typename ExPol, typename StatusFilter>
    std::vector<Document> FindTopDocuments(ExPol&& ex_po, std::string_view raw_query,
                                           StatusFilter status) const;
    template <typename ExPol>
    std::vector<Document> FindTopDocuments(ExPol&& ex_po,std::string_view raw_query,
                          const DocumentStatus document_status = DocumentStatus::ACTUAL) const;

    // Количество документов на сервере
    int GetDocumentCount() const;
    // Итераторы указывающие на первый и на последний id документов на сервере соответственно
    std::set<int>::iterator begin() noexcept;
    std::set<int>::iterator end() noexcept;

    // Возврат частоты слов в документе
    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;
    // Удаление документа
    void RemoveDocument(int document_id);
    template<class ExPol>
    void RemoveDocument(ExPol&& ex_po, int document_id);

    // Матчинг документов
    std::tuple<std::vector<std::string_view>, DocumentStatus>
    MatchDocument(const std::string_view raw_query, int document_id) const;
    template <typename ExPol>
    std::tuple<std::vector<std::string_view>, DocumentStatus>
    MatchDocument(ExPol&& ex_po, const std::string_view raw_query, int document_id) const;

private:
    // Максимальное количество документов, выводимых во время поиска
    const int MAX_RESULT_DOCUMENT_COUNT = 5;

    /// Контейнеры для хранения необработанных строковых данных
    std::set<std::string> stop_words_str_collect_;
    std::map<int, std::string> originals_documents_;

    /// Основные рабочие контейнеры для хранения обработанных данных
    using MapKeyStrView     = std::map<std::string_view, std::map<int, double>>;
    using MapKeyInt         = std::map<int, std::map<std::string_view, double>>;
    std::set<std::string_view> stop_words_;
    MapKeyStrView word_to_document_freqs_;
    MapKeyInt word_to_document_freqs_id_key_;
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    std::map<int, DocumentData> document_ratings_;
    std::set<int> document_ids_;

    // Приватные методы класса
    void StringViewConstructor(std::string_view in_str);
    void CollectionParse(const std::string& in_str);
    void CollectionParse(const std::string_view in_str);
    void CheckId(const int document_id) const;
    void AddDocumentWithoutCheckId(const int document_id,
                                   const std::string_view document,
                                   const DocumentStatus status,
                                   const std::vector<int>& ratings);
    bool IsStopWord(std::string_view word) const;
    static void WordCheckOnValid(const std::string_view word);

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view text) const;
    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        QueryWord() = default;
        QueryWord(std::string_view dt, bool is_min, bool is_stp) : data(dt),
                                                         is_minus(is_min),
                                                         is_stop(is_stp){
        }
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string_view text) const;

    struct Query {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };

    Query ParseQuery(const std::string_view text) const;

    template <typename StatusFilter>
    std::vector<Document> FindAllDocuments(const Query& query, StatusFilter status) const;
    template <typename ExPol, typename StatusFilter>
    std::vector<Document> FindAllDocuments(ExPol&& ex_po, const Query& query, StatusFilter status) const;
    template <typename ExPol, typename StatusFilter, typename Container>
    void FindAllDocumentsImpl(ExPol&& ex_po, const Query& query, StatusFilter status, Container& document_to_relevance) const;
};

// Конструктор класса SearchServer
template <typename StringCollection>
SearchServer::SearchServer(const StringCollection& collection) {
    for(const auto& word : collection) {
        WordCheckOnValid(word);
        if (!word.empty()) {
            CollectionParse(word);
        }
    }
}

template <typename Str>
void SearchServer::AddDocument(const int document_id,
                               const Str& document,
                               const DocumentStatus status,
                               const std::vector<int>& ratings) {
    CheckId(document_id);
    auto& ref_original_doc = originals_documents_[document_id];
    ref_original_doc = document;
    AddDocumentWithoutCheckId(document_id,
                              std::string_view{ref_original_doc},
                              status,
                              ratings);
}

template <typename ExPol,typename StatusFilter>
std::vector<Document> SearchServer::FindTopDocuments(ExPol&& ex_po, const std::string_view raw_query,
                                       StatusFilter status) const {

    Query query = ParseQuery(raw_query);

    std::vector<Document> matched_documents = FindAllDocuments(ex_po, query, status);
    sort(ex_po, matched_documents.begin(), matched_documents.end(),
         [](const Document& lhs, const Document& rhs) {
        const double about_zero = 1e-6;
        if (std::abs(lhs.relevance - rhs.relevance) < about_zero){
            return lhs.rating > rhs.rating;
        } else {
            return lhs.relevance > rhs.relevance;
        }
    });

    if (static_cast<int>(matched_documents.size()) > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename ExPol>
std::vector<Document> SearchServer::FindTopDocuments(ExPol&& ex_po, const std::string_view raw_query,
                                                const DocumentStatus document_status) const {
    return FindTopDocuments(ex_po, raw_query,
           [document_status]([[maybe_unused]] int document_id,
                             [[maybe_unused]] DocumentStatus status,
                             [[maybe_unused]] int rating) {
                             return status == document_status;
                        });
}

template <typename StatusFilter>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query,
                                       StatusFilter status) const {
    return FindTopDocuments(std::execution::seq, raw_query, status);
}

template<class ExPol>
void SearchServer::RemoveDocument(ExPol&& ex_po, int document_id) {
    auto it_doc_id = document_ids_.find(document_id);
    // Проверка наличия документа
    if (it_doc_id == document_ids_.end()) {
        return;
    }

    document_ids_.erase(it_doc_id);       // Удаление из вектора id
    document_ratings_.erase(document_id);   // Удаление из documents ratings
    auto it_wrd_to_doc_id = word_to_document_freqs_id_key_.find(document_id);

    std::vector<const std::string_view*> words;
    words.reserve((it_wrd_to_doc_id->second.size()));

    for_each(
        ex_po,
        it_wrd_to_doc_id->second.begin(),
        it_wrd_to_doc_id->second.end(),
        [&words](const auto& doc){
            words.push_back(&doc.first);
        }
    );

    auto& wrd_to_docs = word_to_document_freqs_;

    // Удаление из word_to_document_freqs_
    for_each(
        ex_po,
        words.begin(), words.end(),
        [document_id, &wrd_to_docs](const std::string_view* wrd) {
            wrd_to_docs.at(*wrd).erase(document_id);
        }
    );

    // Удаление из word_to_document_freqs_id_key_
    word_to_document_freqs_id_key_.erase(document_id);
}

template <typename ExPol>
std::tuple<std::vector<std::string_view>, DocumentStatus>
SearchServer::MatchDocument(ExPol&& ex_po, const std::string_view raw_query, int document_id) const {
    if (!document_ids_.count(document_id)) {
        throw std::out_of_range("Invalid document ID!");
    }

    const auto query = ParseQuery(raw_query);
    std::set<std::string_view> wrd_p;

    const auto& wrd_to_doc_id = word_to_document_freqs_id_key_.at(document_id);
    for (const std::string_view word : query.minus_words) {
        if (wrd_to_doc_id.count(word)) {
            return std::tuple(std::vector<std::string_view>{}, document_ratings_.at(document_id).status);
        }
    }

    std::for_each(
        ex_po,
        query.plus_words.begin(),
        query.plus_words.end(),
        [&wrd_p, &wrd_to_doc_id, document_id](const std::string_view word){
            auto it = wrd_to_doc_id.find(word);
            if (it != wrd_to_doc_id.end()) {
                wrd_p.insert(it->first);
            }
        }
    );

    std::vector<std::string_view> temp;
    for (const std::string_view wrd : wrd_p) {
        temp.push_back(wrd);
    }

    return std::tuple(temp, document_ratings_.at(document_id).status);
}

template <typename ExPol, typename StatusFilter>
std::vector<Document> SearchServer::FindAllDocuments(ExPol&& ex_po, const Query& query, StatusFilter status) const {
    std::map<int, double> ordinary_map;
    if (std::is_same_v<ExPol, std::execution::parallel_policy>) {
        ConcurrentMap<int, double> document_to_relevance(N_BUCKETS);
        FindAllDocumentsImpl(ex_po, query, status, document_to_relevance);
        ordinary_map = document_to_relevance.BuildOrdinaryMap();
    } else {
        FindAllDocumentsImpl(ex_po, query, status, ordinary_map);
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : ordinary_map) {
        matched_documents.push_back({
                                        document_id,
                                        relevance,
                                        document_ratings_.at(document_id).rating
                                    });
    }

    return matched_documents;
}

template <typename ExPol, typename StatusFilter, typename Container>
void SearchServer::FindAllDocumentsImpl(ExPol&& ex_po, const Query& query, StatusFilter status, Container& document_to_relevance) const {
    std::set<int> set_ids;

    /// Поиск документов содержащих плюс слова
    auto search_plus_words_func = [this, &document_to_relevance, &set_ids]
                                  (std::string_view word){
        auto it_docs_collection = word_to_document_freqs_.find(word);
        if (it_docs_collection != word_to_document_freqs_.end()) {
            const double inverse_document_freq = std::log(document_ratings_.size() * 1.0
                                                          / it_docs_collection->second.size());
            for (const auto [document_id, term_freq] : it_docs_collection->second) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                    set_ids.insert(document_id);
            }
        }
    };

    std::for_each(ex_po,
                  query.plus_words.begin(), query.plus_words.end(),
                  search_plus_words_func);

    std::vector<int> vec_ids(set_ids.begin(), set_ids.end());

    /// Исключение документов содержащих минус слова
    /// или если функция фильтрации status возвращает false
    auto& ref_minus_words = query.minus_words;
    auto exclude_docs_with_minus_words_func = [this, &document_to_relevance, &ref_minus_words, status]
                                    (const int id){
        if (!status(id,
                    document_ratings_.at(id).status,
                    document_ratings_.at(id).rating)) {
             document_to_relevance.erase(id);
        } else {
            for (auto word : ref_minus_words) {
                if (word_to_document_freqs_id_key_.at(id).count(word)) {
                    document_to_relevance.erase(id);
                }
            }
        }
    };

    std::for_each(ex_po,
                  set_ids.begin(), set_ids.end(),
                  exclude_docs_with_minus_words_func);
}
