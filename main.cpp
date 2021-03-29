#include "process_queries.h"
#include "search_server.h"

#include "Tests/finde_top_docs_par.h"
#include "Tests/match_doc_par.h"
#include "Tests/proc_queries.h"
#include "Tests/removed_doc_par.h"

#include <execution>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

int main() {
    TestsProcessQueries();
    TestsProcessJoined();
    TestsRemovedDocPar();
    TestsMatchDocPar();
    TestFTDPar();

    return 0;
}
