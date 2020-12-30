#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <set>
#include <memory>
#include <iostream>
#include <exception>
#include <type_traits>

template<typename A, typename V>
class FunctionMaxima {
private:
    class Comparator;

public:
    class PointType;

    using point_type = PointType;

    FunctionMaxima() = default;

    FunctionMaxima(const FunctionMaxima<A, V> &other) noexcept = default;

    FunctionMaxima<A, V> &operator=(FunctionMaxima<A, V> other);

    V const &value_at(A const &a) const;

    void set_value(A const &a, V const &v);

    void erase(A const &a);

    using iterator = typename std::set<point_type, Comparator>::iterator;

    iterator begin() const noexcept;

    iterator end() const noexcept;

    iterator find(A const &a) const;

    using mx_iterator = iterator;

    mx_iterator mx_begin() const noexcept;

    mx_iterator mx_end() const noexcept;

    using size_type = size_t;

    size_type size() const noexcept;

    ~FunctionMaxima();

private:
    PointType create_point(const A &arg, const V &val);

    void delete_point(const A &a);

    std::set<point_type, Comparator> function_points;
    std::set<point_type, Comparator> local_maxima;
};

template<typename A, typename V>
class FunctionMaxima<A, V>::PointType {
public:
    PointType(const PointType &other) noexcept = default;

    PointType &operator=(PointType other) noexcept;

    A const &arg() const noexcept;

    V const &value() const noexcept;

    ~PointType();

private:
    PointType(const A &arg, const V &val);

    friend PointType FunctionMaxima::create_point(const A &arg, const V &val);

    std::shared_ptr<A> point_argument;
    std::shared_ptr<V> point_value;
};

template<typename A, typename V>
class FunctionMaxima<A, V>::Comparator {
public:
    using is_transparent = std::true_type;

    bool operator()(const FunctionMaxima<A, V>::PointType &lk,
                    const A &fk) const {
        return lk.arg() < fk;
    }

    bool operator()(const A &fk, const FunctionMaxima<A, V>::PointType &lk) const {
        return fk < lk.arg();
    }

    bool operator()(const FunctionMaxima<A, V>::PointType &fk,
                    const FunctionMaxima<A, V>::PointType &lk) const {
        return fk.arg() < lk.arg();
    }
};

namespace {
    template<typename A>
    class Guard {
    public:
        Guard(std::shared_ptr<A> *data);

        ~Guard() noexcept;

        void done() noexcept;

    private:
        std::shared_ptr<A> *m_data;
        bool reverse;
    };

    template<typename A>
    Guard<A>::Guard(std::shared_ptr<A> *data) : reverse(true), m_data(data) {}

    template<typename A>
    Guard<A>::~Guard() noexcept {
        if (reverse) {
            m_data->reset();
        }
    }

    template<typename A>
    void Guard<A>::done() noexcept {
        reverse = false;
    }

    class InvalidArg : public std::exception {
        const char *what() const throw() override {
            return "Invalid argument!";
        }
    };
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::PointType &
FunctionMaxima<A, V>::PointType::operator=(FunctionMaxima<A, V>::PointType other) noexcept {
    point_argument.swap(other.arg());
    point_value.swap(other.value());

    return *this;
}

template<typename A, typename V>
FunctionMaxima<A, V>::PointType::PointType(const A &arg, const V &val) {
    point_argument = std::make_shared<A>(A(arg));
    Guard<A> point_construction_guard = Guard<A>(&point_argument);
    point_value = std::make_shared<A>(V(val));
    point_construction_guard.done();
}

template<typename A, typename V>
A const &FunctionMaxima<A, V>::PointType::arg() const noexcept {
    return *point_argument.get();
}

template<typename A, typename V>
V const &FunctionMaxima<A, V>::PointType::value() const noexcept {
    return *point_value.get();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::PointType
FunctionMaxima<A, V>::create_point(const A &arg, const V &val) {
    return typename FunctionMaxima<A, V>::PointType::PointType(arg, val);
}

template<typename A, typename V>
FunctionMaxima<A, V>::PointType::~PointType() {
    point_argument.reset();
    point_value.reset();
}

template<typename A, typename V>
FunctionMaxima<A, V>::~FunctionMaxima() {
    function_points.clear();
    local_maxima.clear();
}

template<typename A, typename V>
FunctionMaxima<A, V> &FunctionMaxima<A, V>::operator=(FunctionMaxima<A, V> other) {
    function_points.swap(other.function_points);
    local_maxima.swap(other.local_maxima);

    return *this;
}

template<typename A, typename V>
V const &FunctionMaxima<A, V>::value_at(const A &a) const {
    return function_points.find(a) != function_points.end() ? (*function_points.find(a)).value()
                                                            : throw InvalidArg();
}

template<typename A, typename V>
void FunctionMaxima<A, V>::delete_point(const A &a) {
    if (function_points.find(a) != function_points.end()) {
        function_points.erase(function_points.find(a));
    }
}

template<typename A, typename V>
void FunctionMaxima<A, V>::set_value(const A &a, const V &v) {
    delete_point(a);
    point_type aux = FunctionMaxima<A, V>::point_type(create_point(a, v));
    function_points.insert(aux);
}

template<typename A, typename V>
void FunctionMaxima<A, V>::erase(const A &a) {
    delete_point(a);
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::begin() const noexcept {
    return function_points.begin();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::end() const noexcept {
    return function_points.end();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::find(const A &a) const {
    return function_points.find(a);
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::mx_iterator FunctionMaxima<A, V>::mx_begin() const noexcept {
    return local_maxima.begin();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::mx_iterator FunctionMaxima<A, V>::mx_end() const noexcept {
    return local_maxima.end();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::size_type FunctionMaxima<A, V>::size() const noexcept {
    return function_points.size();
}

#endif // FUNCTION_MAXIMA_H
