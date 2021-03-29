#pragma once
#include <vector>
#include <iostream>

template <typename It>
class IteratorRange {
public:
    // Конструкторы
    IteratorRange() = default;
    explicit IteratorRange(const It it_begin, const It it_end);

    It begin() const; // Возвращает итератор начала диапазона
    It end() const; // Возвращает итератор конца диапазона
    size_t size() const; // Возвращает количество элементов

private:
    It _begin;
    It _end;
};

// Конструктор класса
template <typename It>
IteratorRange<It>::IteratorRange(const It it_begin, const It it_end) : _begin(it_begin),
                                                             _end(it_end){
}

// Возвращает итератор начала диапазона
template <typename It>
It IteratorRange<It>::begin() const  {
    return _begin;
}

// Возвращает итератор конца диапазона
template <typename It>
It IteratorRange<It>::end() const  {
    return _end;
}

// Возвращает количество элементов
template <typename It>
size_t IteratorRange<It>::size() const {
    return distance(begin(), end());
}

// Вывод класса IteratorRange
template <typename It>
std::ostream& operator<<(std::ostream& out, const IteratorRange<It>& con) {
    for (auto it = con.begin(); it != con.end(); ++it) {
        out << *it;
    }
    return out;
}


template <typename It>
class Paginator {
public:
    // Конструктор
    explicit Paginator(const It it_begin, const It it_end, const size_t page_size);

    auto begin() const; // Возвращает итератор на начало диапазона страниц
    auto end() const; // Возвращает итератор на конец диапазона страниц
    size_t size() const; // Возвращает количество страниц

private:
    std::vector<IteratorRange<It>> _pages;
};

// Конструктор Paginator
template <typename It>
Paginator<It>::Paginator(const It it_begin, const It it_end, const size_t page_size) {
    auto it = it_begin;
    auto n_iteration = distance(it_begin, it_end) / page_size;
    for (int i = 0; i < static_cast<int>(n_iteration); ++i) {
        auto it_b = it;
        advance(it, page_size);
        _pages.push_back(IteratorRange(it_b, it));
    }
    if (it != it_end) {
        _pages.push_back(IteratorRange(it, it_end));
    }
}

// Возвращает итератор на начало диапазона страниц
template <typename It>
auto Paginator<It>::begin() const {
    return _pages.begin();
}

// Возвращает итератор на конец диапазона страниц
template <typename It>
auto Paginator<It>::end() const {
    return _pages.end();
}

// Возвращает количество страниц
template <typename It>
size_t Paginator<It>::size() const {
    return _pages.size();
}

// Функция создания элемента класса Paginator
template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
