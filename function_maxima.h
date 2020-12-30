#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <set>
#include <memory>
#include <iostream>
#include <exception>
#include <type_traits>

template<typename A, typename V>
class FunctionMaxima {
public:
    class PointType;

    using point_type = PointType;

    FunctionMaxima() = default;

    FunctionMaxima(const FunctionMaxima<A, V> &other) noexcept = default;

    FunctionMaxima<A, V> &operator=(FunctionMaxima<A, V> other);

    V const &value_at(A const &a) const;

    ~FunctionMaxima();

private:
    PointType create_point(const A &arg, const V &val);

    class Comparator;

    std::set<point_type, Comparator> function_points;
    std::set<point_type, Comparator> local_maxima;
};

template<typename A, typename V>
class FunctionMaxima<A, V>::PointType {
public:
    PointType(const PointType &other) noexcept = default;

    PointType &operator=(PointType other) noexcept;

    A const &arg() const noexcept;

    V const &val() const noexcept;

    ~PointType();

private:
    PointType(const A &arg, const V &val);

    friend PointType FunctionMaxima::create_point(const A &arg, const V &val);

    std::shared_ptr<A> argument;
    std::shared_ptr<V> value;
};

template<typename A, typename V>
class FunctionMaxima<A, V>::Comparator {
public:
    using is_transparent = std::true_type;

    bool operator()(const typename FunctionMaxima<A, V>::PointType &lk, const A &fk) const {
        return lk.arg() < fk;
    }

    bool operator()(const A &fk, const typename FunctionMaxima<A, V>::PointType &lk) const {
        return fk < lk.arg();
    }

    bool operator()(const typename FunctionMaxima<A, V>::PointType &fk,
                    const typename FunctionMaxima<A, V>::PointType &lk) const {
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
    argument.swap(other.arg());
    value.swap(other.val());

    return *this;
}

template<typename A, typename V>
FunctionMaxima<A, V>::PointType::PointType(const A &arg, const V &val) {
    try {
        argument = std::make_shared<A>(new A(arg));
        Guard<A> point_construction_guard = Guard<A>(&argument);
        value = std::make_shared<A>(new V(val));
        point_construction_guard.done();
    } catch (...) {
        throw;
    }
}

template<typename A, typename V>
A const &FunctionMaxima<A, V>::PointType::arg() const noexcept {
    return *argument.get();
}

template<typename A, typename V>
V const &FunctionMaxima<A, V>::PointType::val() const noexcept {
    return *value.get();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::PointType
FunctionMaxima<A, V>::create_point(const A &arg, const V &val) {
    return FunctionMaxima<A, V>::PointType::PointType(arg, val);
}

template<typename A, typename V>
FunctionMaxima<A, V>::PointType::~PointType() {
    argument.reset();
    value.reset();
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
    return function_points.find(a) != function_points.end() ? (*function_points.find(a)).val()
                                                            : throw InvalidArg();
}

#endif // FUNCTION_MAXIMA_H
