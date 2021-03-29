#include "string_processing.h"

#include <execution>

using namespace std;

vector<string_view> SplitIntoWords(string_view text) {
    vector<string_view> result;
    const int64_t pos_end = text.npos;
    while (true) {
        int64_t space = text.find(' ', 0);
        result.push_back(space == pos_end ? text.substr(0) : text.substr(0, space));
        if (space == pos_end) {
            break;
        } else {
            text.remove_prefix(space + 1);
        }
    }

    return result;
}
