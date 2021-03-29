#include "search_server.h"
#include "string_processing.h"

#include <cmath>
#include <execution>

using namespace std;

SearchServer::SearchServer(const string& text) {
    StringViewConstructor(*stop_words_str_collect_.insert(text).first);
}

SearchServer::SearchServer(string_view text) {
    StringViewConstructor(text);
}

void SearchServer::AddDocument(const int document_id,
                               string_view document,
                               const DocumentStatus status,
                               const vector<int>& ratings) {
    CheckId(document_id);
    AddDocumentWithoutCheckId(document_id,
                              document,
                              status,
                              ratings);
}

void SearchServer::AddDocumentWithoutCheckId(const int document_id,
                                             const std::string_view document,
                                             const DocumentStatus status,
                                             const std::vector<int>& ratings) {
    auto words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    for (std::string_view word : words) {
        WordCheckOnValid(word);
        word_to_document_freqs_[word][document_id] += inv_word_count;
        word_to_document_freqs_id_key_[document_id][word] += inv_word_count;
    }
    document_ratings_[document_id] = {ComputeAverageRating(ratings), status};
    document_ids_.insert(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const string_view raw_query,
                                                const DocumentStatus document_status) const {
    return FindTopDocuments(execution::seq, raw_query, document_status);
}

int SearchServer::GetDocumentCount() const {
    return static_cast<int>(document_ratings_.size());
}

void SearchServer::StringViewConstructor(std::string_view text) {
    const vector<string_view> words = SplitIntoWords(text);
    for (string_view word : words) {
        WordCheckOnValid(word);
        stop_words_.insert(word);
    }
}

void SearchServer::CollectionParse(const std::string& in_str) {
    auto it = stop_words_str_collect_.insert(in_str);
    CollectionParse(std::string_view(*it.first));
}

void SearchServer::CollectionParse(const std::string_view in_str) {
    stop_words_.insert(in_str);
}

// Проверка на отрицательный и повторяющийся id
void SearchServer::CheckId(const int document_id) const {   
    if (document_id < 0 || document_ratings_.count(document_id) > 0) {
        throw invalid_argument("invalid id"s);
    }
}

std::set<int>::iterator SearchServer::begin() noexcept {
    return document_ids_.begin();
}

std::set<int>::iterator SearchServer::end() noexcept {
    return document_ids_.end();
}

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static const std::map<std::string_view, double> default_empty_map;
    return word_to_document_freqs_id_key_.count(document_id) > 0 ? word_to_document_freqs_id_key_.at(document_id) : default_empty_map;
}

void SearchServer::RemoveDocument(int document_id) {
    RemoveDocument(execution::seq, document_id);
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const string_view raw_query,
                                                                  int document_id) const {
    return MatchDocument(execution::seq, raw_query, document_id);
}

bool SearchServer::IsStopWord(string_view word) const {
    return stop_words_.count(word);
}

void SearchServer::WordCheckOnValid(const std::string_view word) {
    if(!std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    })) {
        throw std::invalid_argument("Invalid symbol!");
    }
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(const string_view text) const {
    vector<string_view> words_no_stop;
    auto words = SplitIntoWords(text);
    words_no_stop.reserve(words.size());
    for (string_view word : words) {
        if (!IsStopWord(word)) {
            words_no_stop.push_back(word);
        }
    }
    return words_no_stop;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {
    bool is_minus = false;
    // Проверка на пустое слово
    if (text.empty()) {
        throw invalid_argument("empty word"s);
    }
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
        // Проверка на пустое минус слово
        if (text.empty()) {
            throw invalid_argument("empty minus word"s);
        }
        // Проверка на двойной минус перед словом
        if (text.front() == '-') {
            throw invalid_argument("double minus"s);
        }
    }
    // Проверка на наличие недопустимых символов
    WordCheckOnValid(text);
    return QueryWord(text, is_minus, IsStopWord(text));
}

SearchServer::Query SearchServer::ParseQuery(const string_view text) const {
    const auto words = SplitIntoWords(text);
    Query query;
    for (const string_view word : words) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            } else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}
