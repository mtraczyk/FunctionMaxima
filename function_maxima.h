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
    class FunctionPointsComparator; // Comparator used for storing function points inside a set.

    class LocalMaximaComparator; // Comparator used for storing local maximas inside a set.

    class PointInsertionGuard; // Guard used when inserting a point.

    class LocalMaximaUpdateGuard; // Guard used when updating set with local maximas.

public:
    class PointType;

    using point_type = PointType;

    FunctionMaxima<A, V>() = default;

    FunctionMaxima<A, V>(const FunctionMaxima<A, V> &other) = default;

    FunctionMaxima<A, V> &operator=(FunctionMaxima<A, V> other) noexcept;

    V const &value_at(A const &a) const;

    // Strong exception guarantee.
    void set_value(A const &a, V const &v);

    // Strong exception guarantee.
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

    ~FunctionMaxima<A, V>();

private:
    // Thanks to this function we can create new points even though PointType
    // constructor is private.
    PointType create_point(const A &arg, const V &val) const;

    /* It is being used in a following way:
     * - iterator stores the iterator to a point,
     * - first bool is true only if before updates the point was a local maximum,
     * - second bool is true only if after updates it is gonna be a local maximum,
     * - mx_iterator stores the (before updates) mx_iterator
     * to a point (mx_end() if it is not a local maximum before updates)
     */
    using tpl = typename std::tuple<iterator, bool, bool, mx_iterator>;

    // Used for obtaining data about points that might change during setting values.
    void get_info_for_set_value(tpl &p_info,
                                tpl &ln_info, tpl &rn_info, const A &a, const V &v) const;

    // Used for obtaining data about points that might change during erasing values.
    void get_info_for_erase(tpl &p_info,
                            tpl &ln_info, tpl &rn_info);

    // Returns true when there is already a point (a, v) inside function_points, false otherwise.
    bool check_whether_the_same(const A &a, const V &v) const;

    // Auxiliary function for set_value. Uses information gathered in get_info_for_set_value.
    void set_value_aux(const tpl &p_info, const tpl &ln_info, const tpl &rn_info,
                       const PointType &new_point);

    // Auxiliary function for erase. Uses information gathered in get_info_for_erase.
    void erase_aux(const tpl &p_info, const tpl &ln_info, const tpl &rn_info);

    // Used for storing all the points.
    std::set<point_type, FunctionPointsComparator> function_points;

    // Used for storing local maximas.
    std::set<point_type, LocalMaximaComparator> local_maxima;
};

template<typename A, typename V>
class FunctionMaxima<A, V>::PointType {
public:
    // Copying enabled.
    PointType(const PointType &other) noexcept = default;

    // Assigning enabled.
    PointType &operator=(PointType other) noexcept;

    A const &arg() const noexcept;

    V const &value() const noexcept;

    ~PointType();

private:
    // Creating new points is disabled for interface users.
    PointType(const A &arg, const V &val);

    // Declared as a friend, because we want to have a possibility
    // of creating new points inside implementation.
    friend PointType FunctionMaxima<A, V>::create_point(const A &arg, const V &val) const;

    std::shared_ptr<A> point_argument; // Copying objects of A might be expensive, therefore usage of shared_ptr.
    std::shared_ptr<V> point_value; // Copying objects of V might be expensive, therefore usage of shared_ptr.
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
        if (lk.arg() < fk.arg() || fk.arg() < lk.arg()) {
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
        if (lk.value() < fk.value() || fk.value() < lk.value()) {
            return lk.value() < fk.value();
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
        bool reverse;
        std::shared_ptr<A> *m_data; // It should be a pointer, because we don`t want to increase counter for sharing A.
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
    public:
        // Not declared as virtual on purpose.
        const char *what() const throw() override {
            return "Invalid argument!";
        }
    };
}

template<typename A, typename V>
class FunctionMaxima<A, V>::PointInsertionGuard {
public:
    PointInsertionGuard(const iterator &it,
                        std::set<point_type, FunctionPointsComparator> *fun_points)
            : m_it(it), reverse(true), m_function_points(fun_points) {}

    ~PointInsertionGuard() noexcept {
        if (reverse) {
            (*m_function_points).erase(m_it);
        }
    }

    void done() noexcept {
        reverse = false;
    }

private:
    const iterator m_it;
    bool reverse;
    std::set<point_type, FunctionPointsComparator> *m_function_points; // It should be a pointer.
};

template<typename A, typename V>
class FunctionMaxima<A, V>::LocalMaximaUpdateGuard {
public:
    LocalMaximaUpdateGuard(std::set<point_type, LocalMaximaComparator> *loc_maxima)
            : m_local_maxima(loc_maxima), reverse(true),
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
    std::set<point_type, LocalMaximaComparator> *m_local_maxima; // It should be a pointer.
    bool reverse, is_point_it_not_null, is_ln_it_not_null, is_rn_it_not_null;
};

// Noexcept alignment operator for PointType.
template<typename A, typename V>
typename FunctionMaxima<A, V>::PointType &
FunctionMaxima<A, V>::PointType::operator=(FunctionMaxima<A, V>::PointType other) noexcept {
    point_argument.swap(other.arg()); // Swap is noexcept!!
    point_value.swap(other.value()); // Swap is noexcept!!

    return *this;
}

template<typename A, typename V>
FunctionMaxima<A, V>::PointType::PointType(const A &arg, const V &val) {
    point_argument = std::make_shared<A>(A(arg)); // Copy constructor of A might throw an exception.
    PointConstructionGuard<A> point_construction_guard = PointConstructionGuard<A>(&point_argument);
    point_value = std::make_shared<A>(V(val)); // Copy constructor of V might throw an exception.
    point_construction_guard.done(); // No exception thrown, successful creation of a new point.
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
FunctionMaxima<A, V>::create_point(const A &arg, const V &val) const {
    // This way constructing a new point is possible inside implementation.
    return typename FunctionMaxima<A, V>::PointType::PointType(arg, val);
}

template<typename A, typename V>
FunctionMaxima<A, V>::PointType::~PointType() {
    // Counters decreased.
    point_argument.reset();
    point_value.reset();
}

template<typename A, typename V>
FunctionMaxima<A, V>::~FunctionMaxima() {
    // Containers cleared.
    function_points.clear();
    local_maxima.clear();
}

template<typename A, typename V>
FunctionMaxima<A, V> &FunctionMaxima<A, V>::operator=(FunctionMaxima<A, V> other) noexcept {
    function_points.swap(other.function_points); // Swapping sets is noexcept.
    local_maxima.swap(other.local_maxima); // Swapping sets is noexcept.

    return *this;
}

template<typename A, typename V>
V const &FunctionMaxima<A, V>::value_at(const A &a) const {
    // If a does not belong to the domain - InvalidArg is thrown.
    if(function_points.find(a) != function_points.end())
        return (*function_points.find(a)).value();
    throw InvalidArg();
}

template<typename A, typename V>
void FunctionMaxima<A, V>::set_value(const A &a, const V &v) {
    if (check_whether_the_same(a, v))
        return; // Nothing changes if we set the same value for a.

    // Storing info for the points that might change during updates.
    tpl point_info, left_neighbour_info, right_neighbour_info;
    get_info_for_set_value(point_info, left_neighbour_info, right_neighbour_info, a, v);

    auto new_point = create_point(a, v);
    set_value_aux(point_info, left_neighbour_info, right_neighbour_info, new_point);
}

template<typename A, typename V>
void FunctionMaxima<A, V>::erase(const A &a) {
    using std::get;
    tpl point_info, left_neighbour_info, right_neighbour_info;

    if ((get<0>(point_info) = function_points.find(a)) != end()) {
        get_info_for_erase(point_info, left_neighbour_info, right_neighbour_info);
    } else {
        return;
    }

    erase_aux(point_info, left_neighbour_info, right_neighbour_info);
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
void FunctionMaxima<A, V>::get_info_for_set_value(tpl &p_info, tpl &ln_info,
                                                  tpl &rn_info, const A &a, const V &v) const {
    using std::get;
    p_info = std::make_tuple(end(), false, false, mx_end());
    ln_info = p_info, rn_info = p_info;

    if ((get<0>(p_info) = function_points.find(a)) != end()) {
        std::tuple<iterator, iterator> aux = std::make_tuple(get<0>(p_info), get<0>(p_info));
        if (size() != 0) {
            get<0>(ln_info) = (get<0>(p_info) != begin() ? --get<0>(aux) : end());
            get<0>(rn_info) = ++get<1>(aux);
        }
    } else {
        if (size() != 0) {
            auto new_point = create_point(a, v);
            auto aux = function_points.lower_bound(new_point);
            get<0>(rn_info) = aux;
            get<0>(ln_info) = (aux != begin() ? --aux : end());
        }
    }

    get<1>(p_info) = (get<0>(p_info) != end() && local_maxima.find(*get<0>(p_info)) != mx_end());
    get<1>(ln_info) = (get<0>(ln_info) != end() && local_maxima.find(*get<0>(ln_info)) != mx_end());
    get<1>(rn_info) = (get<0>(rn_info) != end() && local_maxima.find(*get<0>(rn_info)) != mx_end());
    get<2>(p_info) = (get<0>(ln_info) == end() || !((v < (*get<0>(ln_info)).value())))
                     && (get<0>(rn_info) == end() || !(v < (*get<0>(rn_info)).value()));

    std::tuple<iterator, iterator> aux = std::make_tuple(get<0>(ln_info), get<0>(rn_info));
    get<2>(ln_info) = get<0>(ln_info) != end() && (get<0>(ln_info) == begin() ||
                                                   !((*get<0>(ln_info)).value() <
                                                     (*(--get<0>(aux))).value()))
                      && !((*get<0>(ln_info)).value() < v);

    get<2>(rn_info) = get<0>(rn_info) != end() && !((*get<0>(rn_info)).value() < v) &&
                      (++get<1>(aux) == end() ||
                       !((*get<0>(rn_info)).value() < (*get<1>(aux)).value()));

    get<3>(p_info) = (get<0>(p_info) != end() ? local_maxima.find(*get<0>(p_info)) : mx_end());
    get<3>(ln_info) = (get<0>(ln_info) != end() ? local_maxima.find(*get<0>(ln_info)) : mx_end());
    get<3>(rn_info) = (get<0>(rn_info) != end() ? local_maxima.find(*get<0>(rn_info)) : mx_end());
}

template<typename A, typename V>
bool FunctionMaxima<A, V>::check_whether_the_same(const A &a, const V &v) const {
    auto aux = function_points.find(a);

    if (aux != end() && !((v < (*aux).value()) || ((*aux).value() < v))) {
        return true;
    }

    return false;
}

template<typename A, typename V>
void
FunctionMaxima<A, V>::get_info_for_erase(FunctionMaxima<A, V>::tpl &p_info,
                                         FunctionMaxima<A, V>::tpl &ln_info,
                                         FunctionMaxima<A, V>::tpl &rn_info) {
    using std::get;
    get<3>(p_info) = local_maxima.find(*get<0>(p_info));
    get<1>(p_info) = get<3>(p_info) != mx_end();

    if (size() != 0) {
        std::tuple<iterator, iterator> aux = std::make_tuple(get<0>(p_info), get<0>(p_info));
        get<0>(ln_info) = (get<0>(p_info) != begin() ? --get<0>(aux) : end());
        get<0>(rn_info) = ++get<1>(aux);
    }

    get<3>(ln_info) = (get<0>(ln_info) != end() ? local_maxima.find(*get<0>(ln_info)) : mx_end());
    get<3>(rn_info) = (get<0>(rn_info) != end() ? local_maxima.find(*get<0>(rn_info)) : mx_end());
    get<1>(ln_info) = (get<0>(ln_info) != end() && local_maxima.find(*get<0>(ln_info)) != mx_end());
    get<1>(rn_info) = (get<0>(rn_info) != end() && local_maxima.find(*get<0>(rn_info)) != mx_end());

    std::tuple<iterator, iterator> aux = std::make_tuple(get<0>(ln_info), get<0>(rn_info));
    get<2>(ln_info) = get<0>(ln_info) != end() && (get<0>(ln_info) == begin() ||
                                                   !((*get<0>(ln_info)).value() <
                                                     (*(--get<0>(aux))).value()))
                      && ((get<0>(rn_info) == end()) ||
                          !((*get<0>(ln_info)).value() < (*get<0>(rn_info)).value()));

    get<2>(rn_info) = get<0>(rn_info) != end() && (get<0>(ln_info) == end() ||
                                                   !((*get<0>(rn_info)).value() <
                                                     (*get<0>(ln_info)).value())) &&
                      (++get<1>(aux) == end() ||
                       !((*get<0>(rn_info)).value() < (*get<1>(aux)).value()));
}

template<typename A, typename V>
void FunctionMaxima<A, V>::erase_aux(const FunctionMaxima::tpl &p_info,
                                     const FunctionMaxima::tpl &ln_info,
                                     const FunctionMaxima::tpl &rn_info) {
    using std::get;
    auto local_maxima_g = LocalMaximaUpdateGuard(&local_maxima);

    if (!get<1>(ln_info) && get<2>(ln_info))
        local_maxima_g.set_point_it(get<0>(local_maxima.insert(*get<0>(ln_info))));

    if (!get<1>(rn_info) && get<2>(rn_info))
        local_maxima_g.set_point_it(get<0>(local_maxima.insert(*get<0>(rn_info))));

    function_points.erase(get<0>(p_info));

    if (get<1>(p_info))
        local_maxima.erase(get<3>(p_info));

    if (get<1>(ln_info) && !get<2>(ln_info))
        local_maxima.erase(get<3>(ln_info));

    if (get<1>(rn_info) && !get<2>(rn_info))
        local_maxima.erase(get<3>(rn_info));

    local_maxima_g.done();
}

template<typename A, typename V>
void FunctionMaxima<A, V>::set_value_aux(const FunctionMaxima<A, V>::tpl &p_info,
                                         const FunctionMaxima<A, V>::tpl &ln_info,
                                         const FunctionMaxima<A, V>::tpl &rn_info,
                                         const FunctionMaxima<A, V>::PointType &new_point) {
    using std::get;
    auto point_insertion_g = PointInsertionGuard(get<0>(function_points.insert(new_point)),
                                                 &function_points);
    auto local_maxima_g = LocalMaximaUpdateGuard(&local_maxima);

    if (get<2>(p_info))
        local_maxima_g.set_point_it(get<0>(local_maxima.insert(new_point)));

    if (!get<1>(ln_info) && get<2>(ln_info))
        local_maxima_g.set_ln_it(get<0>(local_maxima.insert(*get<0>(ln_info))));

    if (!get<1>(rn_info) && get<2>(rn_info))
        local_maxima_g.set_rn_it(get<0>(local_maxima.insert(*get<0>(rn_info))));

    if (get<0>(p_info) != end())
        function_points.erase(get<0>(p_info));

    if (get<1>(p_info))
        local_maxima.erase(get<3>(p_info));

    if (get<1>(ln_info) && !get<2>(ln_info))
        local_maxima.erase(get<3>(ln_info));

    if (get<1>(rn_info) && !get<2>(rn_info))
        local_maxima.erase(get<3>(rn_info));

    point_insertion_g.done();
    local_maxima_g.done();
}

#endif // FUNCTION_MAXIMA_H
