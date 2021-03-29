#include "proc_queries.h"

#include "log_duration.h"
#include "process_queries.h"
#include "search_server.h"

#include <execution>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

void TestProcQueries1();
void TestProcQueries2();

void TestProcQueriesJoined1();
void TestProcQueriesJoined2();

void TestsProcessQueries() {
    cout << "TestsProcessQueries"s << endl;
    TestProcQueries1();
    TestProcQueries2();
    cout << endl;
}

void TestsProcessJoined() {
    cout << "TestsProcessJoined"s << endl;
    TestProcQueriesJoined1();
    TestProcQueriesJoined2();
    cout << endl;
}

void TestProcQueries1() {
    SearchServer search_server("and with"s);

    int id = 0;
    for (
        const string& text : {
            "funny pet and nasty rat"s,
            "funny pet with curly hair"s,
            "funny pet and not very nasty rat"s,
            "pet with rat and rat and rat"s,
            "nasty rat with curly hair"s,
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }

    const vector<string> queries = {
        "nasty rat -not"s,
        "not very funny nasty pet"s,
        "curly hair"s
    };
    id = 0;
    for (
        const auto& documents : ProcessQueries(search_server, queries)
    ) {
        cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << endl;
    }
}

string GenerateWordProc(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution('a', 'z')(generator));
    }
    return word;
}

vector<string> GenerateDictionaryProc(mt19937& generator, int word_count, int max_length) {
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWordProc(generator, max_length));
    }
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

string GenerateQueryProc(mt19937& generator, const vector<string>& dictionary, int max_word_count) {
    const int word_count = uniform_int_distribution(1, max_word_count)(generator);
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

vector<string> GenerateQueriesProc(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQueryProc(generator, dictionary, max_word_count));
    }
    return queries;
}

template <typename QueriesProcessor>
void TestProc(string_view mark, QueriesProcessor processor, const SearchServer& search_server, const vector<string>& queries) {
    LOG_DURATION(mark);
    const auto documents_lists = processor(search_server, queries);
}

#define TEST_PROC(processor) TestProc(#processor, processor, search_server, queries)

void TestProcQueries2() {
    mt19937 generator;
    const auto dictionary = GenerateDictionaryProc(generator, 10000, 25);
    const auto documents = GenerateQueriesProc(generator, dictionary, 100'000, 10);

    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) {
        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
    }

    const auto queries = GenerateQueriesProc(generator, dictionary, 10'000, 7);
    TEST_PROC(ProcessQueries);
}



void TestProcQueriesJoined1() {
    SearchServer search_server("and with"s);

    int id = 0;
    for (
        const string& text : {
            "funny pet and nasty rat"s,
            "funny pet with curly hair"s,
            "funny pet and not very nasty rat"s,
            "pet with rat and rat and rat"s,
            "nasty rat with curly hair"s,
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }

    const vector<string> queries = {
        "nasty rat -not"s,
        "not very funny nasty pet"s,
        "curly hair"s
    };
    for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
        cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
    }
}

template <typename QueriesProcessor>
void TestProcJoined(string_view mark, QueriesProcessor processor, const SearchServer& search_server, const vector<string>& queries) {
    LOG_DURATION(mark);
    const auto documents = processor(search_server, queries);
    cout << documents.size() << endl;
}

#define TEST_PROC_JOINED(processor) TestProcJoined(#processor, processor, search_server, queries)

void TestProcQueriesJoined2() {
    mt19937 generator;
    const auto dictionary = GenerateDictionaryProc(generator, 10000, 25);
    const auto documents = GenerateQueriesProc(generator, dictionary, 100'000, 10);

    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) {
        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
    }

    const auto queries = GenerateQueriesProc(generator, dictionary, 10'000, 7);
    TEST_PROC_JOINED(ProcessQueries);
}
