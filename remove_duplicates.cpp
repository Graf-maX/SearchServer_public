#include <iostream>
#include "remove_duplicates.h"

using namespace std;

// Функция поиска и удаления повторяющихся элементов (откорректирована)
void RemoveDuplicates(SearchServer& search_server) {
    map<set<string_view>, set<int>> words_in_documents;

    for (auto it = search_server.begin(); it != search_server.end(); ++it) {
        set<string_view> words_in_document;
        for (const auto& [word, freq] : search_server.GetWordFrequencies(*it)) {
            words_in_document.insert(word);
        }
        words_in_documents[words_in_document].insert(*it);
    }

    for (const auto& [wrds, ids] : words_in_documents) {
        if (ids.size() == 1) {
            continue;
        }

        auto it = ids.begin();
        ++it;
        for (; it != ids.end(); ++it) {
            //cout << "Found duplicate document id "s << *it << endl;
            search_server.RemoveDocument(*it);
        }
    }
}
