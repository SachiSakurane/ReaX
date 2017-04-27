/*
  ==============================================================================

    rxjuce_LifetimeWatcherPool.h
    Created: 27 Apr 2017 7:14:37am
    Author:  Martin Finke

  ==============================================================================
*/

#pragma once

#include "rxjuce_Prefix.h"

RXJUCE_NAMESPACE_BEGIN

class LifetimeWatcher;

/**
	Keeps a list of LifetimeWatcher instances and periodically checks whether they have expired.
 
	An expired watcher is removed from the pool and deleted. If you want to react when this happens, please implement the destructor in your LifetimeWatcher subclass.
 
	@see LifetimeWatcher, WeakReferenceLifetimeWatcher
 */
class LifetimeWatcherPool : private juce::Timer, private juce::DeletedAtShutdown
{
public:
	/**
		Returns the global pool.
	 */
	static LifetimeWatcherPool& getInstance();

	/** Adds a new watcher to the pool. The pool takes ownership of the watcher. */
	void add(std::unique_ptr<const LifetimeWatcher>&& watcher);

private:
	LifetimeWatcherPool();
	
	void timerCallback() override;
	
	std::multimap<const void*, std::unique_ptr<const LifetimeWatcher>> watchers;
	
	static LifetimeWatcherPool* instance;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LifetimeWatcherPool)
};

RXJUCE_NAMESPACE_END
