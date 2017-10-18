#include "varx_Subjects_Impl.h"

var SubjectBase::Impl::getLatestItem() const
{
    jassertfalse;
    return var::undefined();
}

BehaviorSubjectImpl::BehaviorSubjectImpl(juce::var&& initial)
: wrapped(initial)
{}

rxcpp::subscriber<var> BehaviorSubjectImpl::getSubscriber() const
{
    return wrapped.get_subscriber();
}

rxcpp::observable<var> BehaviorSubjectImpl::asObservable() const
{
    return wrapped.get_observable();
}

var BehaviorSubjectImpl::getLatestItem() const
{
    return wrapped.get_value();
}

PublishSubjectImpl::PublishSubjectImpl()
{}

rxcpp::subscriber<var> PublishSubjectImpl::getSubscriber() const
{
    return wrapped.get_subscriber();
}

rxcpp::observable<var> PublishSubjectImpl::asObservable() const
{
    return wrapped.get_observable();
}

ReplaySubjectImpl::ReplaySubjectImpl(size_t bufferSize)
: wrapped(bufferSize, rxcpp::identity_immediate())
{}

rxcpp::subscriber<var> ReplaySubjectImpl::getSubscriber() const
{
    return wrapped.get_subscriber();
}

rxcpp::observable<var> ReplaySubjectImpl::asObservable() const
{
    return wrapped.get_observable();
}
