#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <set>
#include <memory>

template<typename A, typename V>
class FunctionMaxima {
public:
    class PointType;

    using point_type = PointType;

    FunctionMaxima() = default;

private:
    PointType create_point();

    std::set<point_type> function_points;
    std::set<point_type> local_maxima;
};

template<typename A, typename V>
class FunctionMaxima<A, V>::PointType {
public:
    PointType(const PointType &other) noexcept = default;

    PointType &operator=(PointType other) noexcept;

private:
    PointType(const A &arg, const V &val);

    friend PointType FunctionMaxima::create_point(const A &arg, const V &val);

    A const &arg() const;

    V const &val() const;

    std::shared_ptr<A> argument;
    std::shared_ptr<V> value;
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
A const &FunctionMaxima<A, V>::PointType::arg() const {
    return *argument.get();
}

template<typename A, typename V>
V const &FunctionMaxima<A, V>::PointType::val() const {
    return *value.get();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::PointType create_point(const A &arg, const V &val) {
    return FunctionMaxima<A, V>::PointType::PointType(arg, val);
}

#endif // FUNCTION_MAXIMA_H
