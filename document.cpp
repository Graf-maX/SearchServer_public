#include "document.h"

using namespace std;

Document::Document() : id(0),
             relevance(0),
             rating(0) {
}

Document::Document(int id_doc, double relevance_doc, int rating_doc) : id(id_doc),
                                                             relevance(relevance_doc),
                                                             rating(rating_doc) {
}

ostream& operator<<(ostream& out, const Document& doc) {
    out << "{ document_id = "s << doc.id
        << ", relevance = "s << doc.relevance
        << ", rating = "s << doc.rating << " }"s;
    return out;
}
