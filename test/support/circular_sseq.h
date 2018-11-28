#pragma once
#include <initializer_list>

class circular_sseq {
private:
    inline static const std::initializer_list<unsigned int> vals = {1U, 2U, 3U, 4U};
    const unsigned int *_b = &*vals.begin();
    size_t _size = vals.size();
    int index = 0;
public:
    using result_type = unsigned int;
    circular_sseq() {}
    circular_sseq(std::initializer_list<unsigned int> v) : _b(&*v.begin()), _size(v.size()) {}
    
    template <typename InputIter>
    circular_sseq(InputIter, InputIter) : index(1) {}
    
    template <typename RandIt>
    void generate(RandIt b, RandIt e) {
        while(b != e) {
            *b = _b[index];
            index = (index + 1) % _size;
            ++b;
        }
    }
    
    size_t size() const {return _size;}
    
    template <typename OutIt>
    void param(OutIt out) {
        for(int i = 0; i < _size; ++i) {
            *out = _b[ (index + i) % _size ];
        }
    }
};
