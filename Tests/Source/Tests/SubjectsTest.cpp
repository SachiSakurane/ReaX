#include "../Other/TestPrefix.h"


TEST_CASE("BehaviorSubject",
          "[Subject][BehaviorSubject]")
{
    BehaviorSubject subject("Initial Item");

    // Subscribe to the subject's Observable
    Array<var> items;
    varxCollectItems(subject.asObservable(), items);

    IT("changes value when changing the Observer")
    {
        CHECK(subject.getLatestItem() == "Initial Item");
        subject.asObserver().onNext(32.55);

        REQUIRE(subject.getLatestItem() == var(32.55));
    }

    IT("has the initial item after being created")
    {
        CHECK(subject.getLatestItem() == "Initial Item");
        varxRequireItems(items, "Initial Item");
    }

    IT("emits when pushing a new item")
    {
        varxCheckItems(items, "Initial Item");

        subject.onNext("New Item");

        CHECK(subject.getLatestItem() == "New Item");
        varxRequireItems(items, "Initial Item", "New Item");
    }

    IT("emits an error when calling onError")
    {
        BehaviorSubject subject(17);
        bool onErrorCalled = false;
        subject.onError(Error());
        subject.asObservable().subscribe([](var) {}, [&](Error) { onErrorCalled = true; });
        REQUIRE(onErrorCalled);
    }

    IT("notifies onCompleted when calling onCompleted")
    {
        bool completed = false;
        subject.asObservable().subscribe([](var) {}, [](Error) {}, [&]() { completed = true; });
        subject.onCompleted();

        REQUIRE(completed);
    }

    IT("does not call onCompleted when destroying the subject")
    {
        auto subject = std::make_shared<BehaviorSubject>(3);
        bool completed = false;
        subject->asObservable().subscribe([](var) {}, [](Error) {}, [&]() { completed = true; });
        subject.reset();

        REQUIRE(!completed);
    }

    IT("can call onCompleted multiple times")
    {
        subject.onCompleted();
        subject.onCompleted();
        subject.onCompleted();
    }
    
    IT("can receive an initial value of a custom type, without wrapping with toVar()")
    {
        BehaviorSubject subject(Point<int>(13, 556));
        REQUIRE(fromVar<Point<int>>(subject.getLatestItem()) == Point<int>(13, 556));
    }
}


TEST_CASE("PublishSubject",
          "[Subject][PublishSubject]")
{
    PublishSubject subject;
    DisposeBag disposeBag;

    // Subscribe to the subject's Observable
    Array<var> items;
    varxCollectItems(subject.asObservable(), items);

    IT("does not emit an item if nothing has been pushed")
    {
        REQUIRE(items.isEmpty());
    }

    IT("emits when pushing a new item")
    {
        CHECK(items.isEmpty());

        subject.onNext("First Item");

        varxRequireItems(items, "First Item");
    }

    IT("does not emit previous item(s) when subscribing")
    {
        subject.onNext(1);
        subject.onNext(2);
        varxCheckItems(items, 1, 2);

        Array<var> laterItems;
        varxCollectItems(subject.asObservable(), laterItems);

        REQUIRE(laterItems.isEmpty());
    }

    IT("changes value when changing the Observer")
    {
        subject.asObserver().onNext(32.51);
        subject.asObserver().onNext(3.0);

        varxRequireItems(items, 32.51, 3.0);
    }

    IT("emits after destruction, if there's still an Observer pushing items")
    {
        auto subject = std::make_shared<PublishSubject>();
        auto observer = subject->asObserver();

        varxCollectItems(subject->asObservable(), items);
        subject.reset();
        observer.onNext(12345);

        varxRequireItems(items, 12345);
    }

    IT("emits an error when calling onError")
    {
        PublishSubject subject;
        bool onErrorCalled = false;
        subject.onError(Error());
        subject.asObservable().subscribe([](var) {}, [&](Error) { onErrorCalled = true; }).disposedBy(disposeBag);
        REQUIRE(onErrorCalled);
    }

    CONTEXT("onCompleted")
    {
        auto subject = std::make_shared<PublishSubject>();
        bool completed = false;

        IT("notifies onCompleted when calling onCompleted")
        {
            subject->onCompleted();
            subject->asObservable().subscribe([](var) {}, [](Error) {}, [&]() { completed = true; }).disposedBy(disposeBag);

            REQUIRE(completed);
        }

        IT("does not call onCompleted when destroying the subject")
        {
            subject->asObservable().subscribe([](var) {}, [](Error) {}, [&]() { completed = true; }).disposedBy(disposeBag);
            CHECK(!completed);
            subject.reset();

            REQUIRE(!completed);
        }

        IT("can call onCompleted multiple times")
        {
            subject->onCompleted();
            subject->onCompleted();
            subject->onCompleted();
        }
    }
}


TEST_CASE("ReplaySubject",
          "[Subject][ReplaySubject]")
{
    ReplaySubject subject;
    DisposeBag disposeBag;

    // Subscribe to the subject's Observable
    Array<var> items;
    varxCollectItems(subject.asObservable(), items);

    IT("does not emit an item if nothing has been pushed")
    {
        REQUIRE(items.isEmpty());
    }

    IT("emits when pushing a new item")
    {
        CHECK(items.isEmpty());

        subject.onNext("First Item");

        varxRequireItems(items, "First Item");
    }

    IT("emits previous items when subscribing")
    {
        subject.onNext(1);
        subject.onNext(2);
        varxCheckItems(items, 1, 2);

        Array<var> laterItems;
        varxCollectItems(subject.asObservable(), laterItems);

        varxRequireItems(laterItems, 1, 2);
    }

    IT("emits previous items limited by the max. buffer size")
    {
        auto subject = std::make_shared<ReplaySubject>(4);

        // These should be forgotten:
        subject->onNext(17.5);
        subject->onNext("Hello!");

        // These should be remembered:
        subject->onNext(7);
        subject->onNext(28);
        subject->onNext(3);
        subject->onNext(6);

        Array<var> items;
        varxCollectItems(*subject, items);

        varxRequireItems(items, 7, 28, 3, 6);
    }

    IT("changes value when changing the Observer")
    {
        subject.asObserver().onNext(32.51);
        subject.asObserver().onNext(3.0);

        varxRequireItems(items, 32.51, 3.0);
    }

    IT("emits after destruction, if there's still an Observer pushing items")
    {
        auto subject = std::make_shared<ReplaySubject>();
        auto observer = subject->asObserver();

        varxCollectItems(subject->asObservable(), items);
        subject.reset();
        observer.onNext(12345);

        varxRequireItems(items, 12345);
    }

    IT("emits an error when calling onError")
    {
        ReplaySubject subject;
        bool onErrorCalled = false;
        subject.onError(Error());
        subject.asObservable().subscribe([](var) {}, [&](Error) { onErrorCalled = true; }).disposedBy(disposeBag);
        REQUIRE(onErrorCalled);
    }

    CONTEXT("onCompleted")
    {
        auto subject = std::make_shared<PublishSubject>();
        bool completed = false;

        IT("notifies onCompleted when calling onCompleted")
        {
            subject->onCompleted();
            subject->asObservable().subscribe([](var) {}, [](Error) {}, [&]() { completed = true; }).disposedBy(disposeBag);

            REQUIRE(completed);
        }

        IT("does not call onCompleted when destroying the subject")
        {
            subject->asObservable().subscribe([](var) {}, [](Error) {}, [&]() { completed = true; }).disposedBy(disposeBag);
            CHECK(!completed);
            subject.reset();

            REQUIRE(!completed);
        }

        IT("can call onCompleted multiple times")
        {
            subject->onCompleted();
            subject->onCompleted();
            subject->onCompleted();
        }
    }
}
