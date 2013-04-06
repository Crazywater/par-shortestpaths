#ifndef REQENTRY_H
#define REQENTRY_H
struct ReqEntry {
    vertex_idx_t prev;
    vertex_idx_t v;
    weight_t x;
};
#endif // REQENTRY_H
