/*
  ==============================================================================

    rxjuce_Subscription.h
    Created: 27 Apr 2017 7:09:19am
    Author:  Martin Finke

  ==============================================================================
*/

#pragma once

#include "rxjuce_Prefix.h"

RXJUCE_NAMESPACE_BEGIN

/**
	Manages the lifetime of a subscription to an Observable.
 
	@see Observable::subscribe
 */
class Subscription
{
public:
	
	Subscription(Subscription&&) = default;
	Subscription& operator=(Subscription&&) = default;
	
	/**
		Unsubuscribes from the Observable.
	 */
	void unsubscribe() const;
	
	/**
		Returns true iff the Subscription is still subscribed to the source Observable.
	 */
	bool isSubscribed() const;
	
private:
	friend class Observable;
	explicit Subscription(const std::function<bool()>&, const std::function<void()>&);
	const std::function<bool()> _isSubscribed;
	const std::function<void()> _unsubscribe;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Subscription)
};

/**
	A Subscription that automatically unsubscribes when it is destroyed.
 
	You can use this to prevent onNext from getting called after one of your objects is destroyed.
 
	For example:
		
		class MyClass
		{
		public:
			MyClass(Observable input)
			: subscription(input.subscribe([](var newValue) { ... })) {}
 
		private:
			const RAIISubscription subscription;
		};
 */
class RAIISubscription : public Subscription
{
public:
	RAIISubscription(Subscription&& subscription);
	RAIISubscription(RAIISubscription&&) = default;
	~RAIISubscription();
	
	RAIISubscription& operator=(RAIISubscription&&) = default;
	
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RAIISubscription)
};

RXJUCE_NAMESPACE_END
