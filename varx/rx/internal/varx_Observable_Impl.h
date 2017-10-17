#pragma once

struct Observable::Impl
{
public:
    Impl(const rxcpp::observable<var>& wrapped = rxcpp::observable<>::never<var>());

    static std::shared_ptr<Impl> fromRxCpp(const rxcpp::observable<var>& wrapped);

    static std::shared_ptr<Impl> fromValue(const Value& value);

    template<typename Transform, typename... Os>
    std::shared_ptr<Impl> combineLatest(Transform&& transform, Os&&... observables)
    {
        return fromRxCpp(wrapped.combine_latest(transform, observables.impl->wrapped...));
    }

    template<typename... Os>
    std::shared_ptr<Impl> concat(Os&&... observables)
    {
        return fromRxCpp(wrapped.concat(observables.impl->wrapped...));
    }

    template<typename... Os>
    std::shared_ptr<Impl> merge(Os&&... observables)
    {
        return fromRxCpp(wrapped.merge(observables.impl->wrapped...));
    }

    template<typename... Items>
    std::shared_ptr<Impl> startWith(Items&&... items)
    {
        return fromRxCpp(wrapped.start_with(items...));
    }

    template<typename Transform, typename... Os>
    std::shared_ptr<Impl> withLatestFrom(Transform&& transform, Os&&... observables)
    {
        return fromRxCpp(wrapped.with_latest_from(transform, observables.impl->wrapped...));
    }

    template<typename Transform, typename... Os>
    std::shared_ptr<Impl> zip(Transform&& transform, Os&&... observables)
    {
        return fromRxCpp(wrapped.zip(transform, observables.impl->wrapped...));
    }

    rxcpp::observable<var> wrapped;

private:
    friend class PublishSubject;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Impl)
};






struct ObservableBase::Impl
{
public:
    Impl(const rxcpp::observable<var>& wrapped = rxcpp::observable<>::never<var>());
    
    static std::shared_ptr<Impl> fromRxCpp(const rxcpp::observable<var>& wrapped);
    
    static std::shared_ptr<Impl> fromValue(const Value& value);
    
    template<typename T>
    static std::shared_ptr<Impl> range(const T& first, const T& last, unsigned int step)
    {
        if (first > last)
            throw InvalidRangeError;
        
        auto o = rxcpp::observable<>::range<T>(first, last, step, rxcpp::identity_immediate());
        
        return Impl::fromRxCpp(o.map([](const T& item) { return toVar(item); }));
    }
    
    template<typename Transform, typename... Os>
    std::shared_ptr<Impl> combineLatest(Transform&& transform, Os&&... observables)
    {
        return fromRxCpp(wrapped.combine_latest(transform, observables.impl->wrapped...));
    }
    
    template<typename... Os>
    std::shared_ptr<Impl> concat(Os&&... observables)
    {
        return fromRxCpp(wrapped.concat(observables.impl->wrapped...));
    }
    
    template<typename... Os>
    std::shared_ptr<Impl> merge(Os&&... observables)
    {
        return fromRxCpp(wrapped.merge(observables.impl->wrapped...));
    }
    
    template<typename... Items>
    std::shared_ptr<Impl> startWith(Items&&... items)
    {
        return fromRxCpp(wrapped.start_with(items...));
    }
    
    template<typename Transform, typename... Os>
    std::shared_ptr<Impl> withLatestFrom(Transform&& transform, Os&&... observables)
    {
        return fromRxCpp(wrapped.with_latest_from(transform, observables.impl->wrapped...));
    }
    
    template<typename Transform, typename... Os>
    std::shared_ptr<Impl> zip(Transform&& transform, Os&&... observables)
    {
        return fromRxCpp(wrapped.zip(transform, observables.impl->wrapped...));
    }
    
    rxcpp::observable<var> wrapped;
    
private:
    static const std::runtime_error InvalidRangeError;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Impl)
};
