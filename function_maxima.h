#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <set>
#include <memory>
#include <iostream>
#include <exception>
#include <type_traits>
#include <tuple>

template<typename A, typename V>
class FunctionMaxima {
private:
    class FunctionPointsComparator;

    class LocalMaximaComparator;

    class PointInsertionGuard;

    class LocalMaximaUpdateGuard;

public:
    class PointType;

    using point_type = PointType;

    FunctionMaxima() = default;

    FunctionMaxima(const FunctionMaxima<A, V> &other) noexcept = default;

    FunctionMaxima<A, V> &operator=(FunctionMaxima<A, V> other);

    V const &value_at(A const &a) const;

    void set_value(A const &a, V const &v);

    void erase(A const &a);

    using iterator = typename std::set<point_type, FunctionPointsComparator>::iterator;

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

    using tpl = typename std::tuple<iterator, bool, bool, mx_iterator>;

    void get_info(tpl &p_info, tpl &ln_info, tpl &rn_info, const A &a, const V &v) const;

    bool check_whether_the_same(const A &a, const V &v) const;

    std::set<point_type, FunctionPointsComparator> function_points;
    std::set<point_type, LocalMaximaComparator> local_maxima;
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
class FunctionMaxima<A, V>::FunctionPointsComparator {
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
        if (fk.arg() > lk.arg() || fk.arg() < lk.arg()) {
            return fk.arg() < lk.arg();
        }

        return fk.value() < lk.value();
    }
};

template<typename A, typename V>
class FunctionMaxima<A, V>::LocalMaximaComparator {
public:
    using is_transparent = std::true_type;

    bool operator()(const FunctionMaxima<A, V>::PointType &fk,
                    const FunctionMaxima<A, V>::PointType &lk) const {
        if (fk.value() > lk.value() || fk.value() < lk.value()) {
            return fk.value() > lk.value();
        }

        return fk.arg() < lk.arg();
    }
};

namespace {
    template<typename A>
    class PointConstructionGuard {
    public:
        PointConstructionGuard(std::shared_ptr<A> *data);

        ~PointConstructionGuard() noexcept;

        void done() noexcept;

    private:
        std::shared_ptr<A> *m_data;
        bool reverse;
    };

    template<typename A>
    PointConstructionGuard<A>::PointConstructionGuard(std::shared_ptr<A> *data) : reverse(true),
                                                                                  m_data(data) {}

    template<typename A>
    PointConstructionGuard<A>::~PointConstructionGuard() noexcept {
        if (reverse) {
            m_data->reset();
        }
    }

    template<typename A>
    void PointConstructionGuard<A>::done() noexcept {
        reverse = false;
    }

    class InvalidArg : public std::exception {
        const char *what() const throw() override {
            return "Invalid argument!";
        }
    };
}

template<typename A, typename V>
class FunctionMaxima<A, V>::PointInsertionGuard {
public:
    PointInsertionGuard(iterator it, std::set<point_type, FunctionPointsComparator> *fun_points)
            : reverse(true), m_it(it), m_function_points(fun_points) {}

    ~PointInsertionGuard() noexcept {
        if (reverse) {
            (*m_function_points).erase(m_it);
        }
    }

    void done() noexcept {
        reverse = false;
    }

private:
    iterator m_it;
    std::set<point_type, FunctionPointsComparator> *m_function_points;
    bool reverse;
};

template<typename A, typename V>
class FunctionMaxima<A, V>::LocalMaximaUpdateGuard {
public:
    LocalMaximaUpdateGuard(std::set<point_type, LocalMaximaComparator> *loc_maxima)
            : reverse(true), m_local_maxima(loc_maxima),
              is_point_it_not_null(false), is_ln_it_not_null(false), is_rn_it_not_null(false) {}

    ~LocalMaximaUpdateGuard() noexcept {
        if (reverse) {
            if (is_point_it_not_null) {
                (*m_local_maxima).erase(point_it);
            }

            if (is_ln_it_not_null) {
                (*m_local_maxima).erase(ln_it);
            }

            if (is_rn_it_not_null) {
                (*m_local_maxima).erase(rn_it);
            }
        }
    }

    void set_point_it(mx_iterator it) noexcept {
        point_it = it;
        is_point_it_not_null = true;
    }

    void set_ln_it(mx_iterator it) noexcept {
        ln_it = it;
        is_ln_it_not_null = true;
    }

    void set_rn_it(mx_iterator it) noexcept {
        rn_it = it;
        is_rn_it_not_null = true;
    }

    void done() noexcept {
        reverse = false;
    }

private:
    mx_iterator point_it, ln_it, rn_it;
    std::set<point_type, LocalMaximaComparator> *m_local_maxima;
    bool reverse, is_point_it_not_null, is_ln_it_not_null, is_rn_it_not_null;
};

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
    PointConstructionGuard<A> point_construction_guard = PointConstructionGuard<A>(&point_argument);
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

// JESTEM TUTAJ :) :)

template<typename A, typename V>
void FunctionMaxima<A, V>::set_value(const A &a, const V &v) {
    if (check_whether_the_same(a, v))
        return;

    tpl point_info, left_neighbour_info, right_neighbour_info;
    get_info(point_info, left_neighbour_info, right_neighbour_info);

    auto new_point = create_point(a, v);
    auto point_insertion_g = PointInsertionGuard(function_points.insert(new_point),
                                                 &function_points);
    auto local_maxima_g = LocalMaximaUpdateGuard(&local_maxima);

    if (point_info.get(2))
        local_maxima_g.set_point_it(local_maxima.insert(new_point));

    if (!left_neighbour_info.get(1) && left_neighbour_info.get(2))
        local_maxima_g.set_point_it(local_maxima.insert(*left_neighbour_info.get(0)));

    if (!right_neighbour_info.get(1) && right_neighbour_info.get(2))
        local_maxima_g.set_point_it(local_maxima.insert(*right_neighbour_info.get(0)));

    if (point_info.get(0) != end())
        function_points.erase(point_info.get(0));

    if (point_info.get(1))
        local_maxima.erase(point_info.get(3));

    if (left_neighbour_info.get(1) && !left_neighbour_info.get(2))
        local_maxima.erase(left_neighbour_info.get(3));

    if (right_neighbour_info.get(1) && !right_neighbour_info.get(2))
        local_maxima.erase(right_neighbour_info.get(3));
}

template<typename A, typename V>
void FunctionMaxima<A, V>::erase(const A &a) {

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

template<typename A, typename V>
void FunctionMaxima<A, V>::get_info(tpl &p_info, tpl &ln_info,
                                    tpl &rn_info, const A &a, const V &v) const {
    p_info = std::make_tuple(end(), false, false, mx_end());
    ln_info = p_info, rn_info = p_info;

    if ((p_info.get(0) = function_points.find(a)) != end()) {
        auto aux = {p_info.get(0), p_info.get(0)};
        ln_info.get(0) = (p_info.get(0) != begin() ? --aux.first() : end());
        rn_info.get(0) = ++aux.second();
    } else {
        auto aux = function_points.upper_bound(create_point(a, v));
        rn_info.get(0) = aux;
        ln_info.get(0) = ((aux != end() && aux != begin()) ? --aux : end());
    }

    p_info.get(1) = (p_info.get(0) != end() && local_maxima.find(*p_info.get(0)) != mx_end());
    ln_info.get(1) = (ln_info.get(0) != end() &&
                      local_maxima.find(*ln_info.get(0)) != mx_end());
    rn_info.get(1) = (rn_info.get(0) != end() &&
                      local_maxima.find(*rn_info.get(0)) != mx_end());
    p_info.get(2) = (ln_info.get(0) == end() || !((*ln_info.get(0)).value() > v)) &&
                    (rn_info.get(0) == end() || !((*rn_info.get(0)).value() > v));

    auto aux = {ln_info.get(0), rn_info.get(0)};
    ln_info.get(2) = ln_info.get(0) != end() && (ln_info.get(0) == begin()
                                                 || !((*ln_info.get(0)).value() <
                                                      (*(--aux.first())).value()))
                     && !((*ln_info.get(0)).value() < v);

    rn_info.get(2) = rn_info.get(0) != end() && !((*rn_info.get(0)).value() < v) &&
                     (++aux.second() == end() ||
                      !((*rn_info.get(0)).value() < (*aux.second()).value()));

    p_info.get(3) = (p_info.get(0) != end() ? local_maxima.find(*p_info.get(0)) : mx_end());
    ln_info.get(3) = (ln_info.get(0) != end() ? local_maxima.find(*ln_info.get(0)) : mx_end());
    rn_info.get(3) = (rn_info.get(0) != end() ? local_maxima.find(*rn_info.get(0)) : mx_end());
}

template<typename A, typename V>
bool FunctionMaxima<A, V>::check_whether_the_same(const A &a, const V &v) const {
    auto aux = function_points.find(a);

    if (aux != end() && !(((*aux).value() > v) || ((*aux).value() < v))) {
        return true;
    }

    return false;
}

#endif // FUNCTION_MAXIMA_H
