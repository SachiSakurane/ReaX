/*
  ==============================================================================

    SubjectsTest.cpp
    Created: 30 Apr 2017 11:25:48am
    Author:  Martin Finke

  ==============================================================================
*/

#include "rxjuce_TestPrefix.h"

#include "rxjuce_Observer.h"
#include "rxjuce_Subjects.h"


TEST_CASE("BehaviorSubject",
		  "[BehaviorSubject]")
{
	BehaviorSubject subject("Initial Item");
	
	// Subscribe to the subject's Observable
	Array<var> items;
	RxJUCECollectItems(subject, items);
	
	IT("changes value when changing the Observer") {
		CHECK(subject.getLatestItem() == "Initial Item");
		subject.onNext(32.55);
		
		REQUIRE(subject.getLatestItem() == var(32.55));
	}
	
	IT("has the initial item after being created") {
		CHECK(subject.getLatestItem() == "Initial Item");
		RxJUCERequireItems(items, "Initial Item");
	}
	
	IT("emits when pushing a new item") {
		RxJUCECheckItems(items, "Initial Item");
		
		subject.onNext("New Item");
		
		CHECK(subject.getLatestItem() == "New Item");
		RxJUCERequireItems(items, "Initial Item", "New Item");
	}
	
	IT("emits an error when calling onError") {
		BehaviorSubject subject(17);
		bool onErrorCalled = false;
		subject.onError(Error());
		subject.subscribe([](var){}, [&](Error) { onErrorCalled = true; });
		REQUIRE(onErrorCalled);
	}
	
	IT("notifies onCompleted when calling onCompleted") {
		bool completed = false;
		subject.subscribe([](var){}, [&](){ completed = true; });
		subject.onCompleted();
		
		REQUIRE(completed);
	}
	
	IT("does not call onCompleted when destroying the subject") {
		auto subject = std::make_shared<BehaviorSubject>(3);
		bool completed = false;
		subject->subscribe([](var){}, [&](){ completed = true; });
		subject.reset();
		
		REQUIRE(!completed);
	}
	
	IT("can call onCompleted multiple times") {
		subject.onCompleted();
		subject.onCompleted();
		subject.onCompleted();
	}
}


TEST_CASE("PublishSubject",
		  "[PublishSubject]")
{
	PublishSubject subject;
	DisposeBag disposeBag;
	
	// Subscribe to the subject's Observable
	Array<var> items;
	RxJUCECollectItems(subject, items);
	
	IT("does not emit an item if nothing has been pushed") {
		REQUIRE(items.isEmpty());
	}
	
	IT("emits when pushing a new item") {
		CHECK(items.isEmpty());
		
		subject.onNext("First Item");
	
		RxJUCERequireItems(items, "First Item");
	}
	
	IT("does not emit previous item(s) when subscribing") {
		subject.onNext(1);
		subject.onNext(2);
		RxJUCECheckItems(items, 1, 2);
		
		Array<var> laterItems;
		RxJUCECollectItems(subject, laterItems);
		
		REQUIRE(laterItems.isEmpty());
	}
	
	IT("changes value when changing the Observer") {
		subject.onNext(32.51);
		subject.onNext(3.0);
		
		RxJUCERequireItems(items, 32.51, 3.0);
	}
	
	IT("emits after destruction, if there's still an Observer pushing items") {
		auto subject = std::make_shared<PublishSubject>();
		auto observer = subject->asObserver();
		
		RxJUCECollectItems(subject->asObservable(), items);
		subject.reset();
		observer.onNext(12345);
		
		RxJUCERequireItems(items, 12345);
	}
	
	IT("emits an error when calling onError") {
		PublishSubject subject;
		bool onErrorCalled = false;
		subject.onError(Error());
		subject.subscribe([](var){}, [&](Error) { onErrorCalled = true; }).disposedBy(disposeBag);
		REQUIRE(onErrorCalled);
	}
	
	CONTEXT("onCompleted") {
		auto subject = std::make_shared<PublishSubject>();
		bool completed = false;
		
		IT("notifies onCompleted when calling onCompleted") {
			subject->onCompleted();
			subject->subscribe([](var){}, [&](){ completed = true; }).disposedBy(disposeBag);
			
			REQUIRE(completed);
		}
		
		IT("does not call onCompleted when destroying the subject") {
			subject->subscribe([](var){}, [&](){ completed = true; }).disposedBy(disposeBag);
			CHECK(!completed);
			subject.reset();
			
			REQUIRE(!completed);
		}
		
		IT("can call onCompleted multiple times") {
			subject->onCompleted();
			subject->onCompleted();
			subject->onCompleted();
		}
	}
}


TEST_CASE("ReplaySubject",
		  "[ReplaySubject]")
{
#warning TODO
}
