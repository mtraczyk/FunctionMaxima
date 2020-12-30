#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

namespace {
    class GuardPointTypeConstruction {
    public:
        GuardPointTypeConstruction();

        ~GuardPointTypeConstruction() noexcept;

        void done() noexcept;

    private:
        bool reverse;
    };
}

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
    PointType(const PointType &other) noexcept;

    PointType &operator=(const PointType &other);

private:
    PointType(const A &arg, const V &val);

    friend PointType FunctionMaxima::create_point(const A &arg, const V &val);

    A const &arg() const;

    V const &val() const;

    std::shared_ptr<A> argument;
    std::shared_ptr<V> value;
};

GuardPointTypeConstruction::GuardPointTypeConstruction() {

}

GuardPointTypeConstruction::~GuardPointTypeConstruction() noexcept {

}

void GuardPointTypeConstruction::done() noexcept {

}

template<typename A, typename V>
FunctionMaxima<A, V>::PointType::PointType(const FunctionMaxima<A, V>::PointType &other) noexcept {

}

template<typename A, typename V>
typename FunctionMaxima<A, V>::PointType &
FunctionMaxima<A, V>::PointType::operator=(const FunctionMaxima<A, V>::PointType &other) {

}

template<typename A, typename V>
FunctionMaxima<A, V>::PointType::PointType(const A &arg, const V &val) {

}

template<typename A, typename V>
A const &FunctionMaxima<A, V>::PointType::arg() const {

}

template<typename A, typename V>
V const &FunctionMaxima<A, V>::PointType::val() const {

}


#endif // FUNCTION_MAXIMA_H
