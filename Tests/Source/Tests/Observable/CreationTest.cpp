#include "../../Other/TestPrefix.h"

using Catch::Contains;


TEST_CASE("Observable::create",
          "[Observable][Observable::create]")
{
    Array<var> items;

    IT("emits items when pushing items synchronously")
    {
        auto observable = Observable::create([](Observer observer) {
            observer.onNext("First");
            observer.onNext("Second");
        });
        varxCollectItems(observable, items);

        varxRequireItems(items, "First", "Second");
    }

    IT("emits items when pushing items asynchronously")
    {
        auto observable = Observable::create([](Observer observer) {
            MessageManager::getInstance()->callAsync([observer]() {
                observer.onNext("First");
                observer.onNext("Second");
            });
        });
        varxCollectItems(observable, items);

        // There shouldn't be any items until the async callback is executed
        CHECK(items.isEmpty());

        // The items should be there after running the dispatch loop
        varxRunDispatchLoop();
        varxRequireItems(items, "First", "Second");
    }

    IT("emits can emit items asynchronously after being destroyed")
    {
        auto observable = std::make_shared<Observable>(Observable::create([](Observer observer) {
            MessageManager::getInstance()->callAsync([observer]() {
                observer.onNext("First");
                observer.onNext("Second");
            });
        }));

        IT("emits when there's still a disposable")
        {
            auto disposable = observable->subscribe([&](var next) { items.add(next); });
            observable.reset();
            varxRunDispatchLoop();

            varxRequireItems(items, "First", "Second");
        }

        IT("doesn't emit when the disposable has unsubscribed")
        {
            auto disposable = observable->subscribe([&](var next) { items.add(next); });
            observable.reset();
            disposable.dispose();
            varxRunDispatchLoop();

            REQUIRE(items.isEmpty());
        }
    }

    IT("calls onSubscribe again for each new disposable")
    {
        auto observable = Observable::create([](Observer observer) {
            observer.onNext("onSubscribe called");
        });
        varxCollectItems(observable, items);
        varxCollectItems(observable, items);
        varxCollectItems(observable, items);

        varxRequireItems(items, "onSubscribe called", "onSubscribe called", "onSubscribe called");
    }

    IT("captures an object until the Observable is destroyed")
    {
        // Create a ref counted object
        class Dummy : public ReferenceCountedObject
        {
        public:
            Dummy()
            : ReferenceCountedObject() {}
        };
        ReferenceCountedObjectPtr<ReferenceCountedObject> pointer(new Dummy());

        // Capture it in the Observable
        auto observable = std::make_shared<Observable>(Observable::create([pointer](Observer observer) {}));

        // There should be 2 references: From pointer and from the Observable
        CHECK(pointer->getReferenceCount() == 2);

        // If a copy of the Observable is made, it should still be 2
        auto copy = std::make_shared<Observable>(*observable);
        CHECK(pointer->getReferenceCount() == 2);

        // After the first Observable is destroyed, there should still be 2
        observable.reset();
        CHECK(pointer->getReferenceCount() == 2);

        // Creating a copy should not increase the ref count
        DisposeBag disposeBag;
        copy->subscribe([](var) {}).disposedBy(disposeBag);
        CHECK(pointer->getReferenceCount() == 2);

        // After the copy is destroyed, there should be just 1 (from the pointer)
        copy.reset();
        REQUIRE(pointer->getReferenceCount() == 1);
    }
}


TEST_CASE("Observable::defer",
          "[Observable][Observable::defer]")
{
    Array<var> items;

    IT("calls the factory function on every new subscription")
    {
        int numCalls = 0;
        auto observable = Observable::defer([&]() {
            numCalls++;
            return Observable::from({ 3, 4 });
        });

        varxCollectItems(observable, items);
        varxCollectItems(observable, items);
        varxCollectItems(observable, items);

        varxRequireItems(items, 3, 4, 3, 4, 3, 4);
        REQUIRE(numCalls == 3);
    }
}


TEST_CASE("Observable::empty",
          "[Observable][Observable::empty]")
{
    Array<var> items;
    auto o = Observable::empty();

    IT("doesn't emit any items")
    {
        varxCollectItems(o, items);
        varxRunDispatchLoop();

        REQUIRE(items.isEmpty());
    }

    IT("notifies onCompleted immediately")
    {
        DisposeBag disposeBag;
        bool completed = false;
        o.subscribe([](var) {}, [](Error) {}, [&]() { completed = true; }).disposedBy(disposeBag);

        REQUIRE(completed);
    }
}


TEST_CASE("Observable::error",
          "[Observable][Observable::error]")
{
    Array<var> items;
    auto o = Observable::error(std::runtime_error("Error!!111!"));
    DisposeBag disposeBag;

    IT("doesn't emit any items")
    {
        o.subscribe([&](var item) { items.add(item); }, [](Error e) {}).disposedBy(disposeBag);
        varxRunDispatchLoop();

        REQUIRE(items.isEmpty());
    }

    IT("notifies onCompleted immediately")
    {
        bool onErrorCalled = false;
        o.subscribe([](var) {}, [&](Error e) { onErrorCalled = true; }).disposedBy(disposeBag);

        REQUIRE(onErrorCalled);
    }
}


TEST_CASE("Observable::from",
          "[Observable][Observable::from]")
{
    Array<var> items;

    IT("can be created from an Array<var>")
    {
        varxCollectItems(Observable::from(Array<var>({ 3, 6, 8 })), items);

        varxRequireItems(items, 3, 6, 8);
    }

    IT("can be created from a std::initializer_list<var>")
    {
        varxCollectItems(Observable::from({ var("Hello"), var(15.5) }), items);

        varxRequireItems(items, var("Hello"), var(15.5));
    }

    IT("can be created from a std::initializer_list<int>")
    {
        varxCollectItems(Observable::from({ 1, 4 }), items);

        varxRequireItems(items, 1, 4);
    }

    IT("can be created from a std::initializer_list<String>")
    {
        varxCollectItems(Observable::from({ "Hello", "Test" }), items);

        varxRequireItems(items, "Hello", "Test");
    }
}


TEST_CASE("Observable::fromValue",
          "[Observable][Observable::fromValue]")
{
    Value value(String("Initial Item"));
    const auto observable = Observable::fromValue(value);
    Array<var> items;
    varxCollectItems(observable, items);

    varxCheckItems(items, "Initial Item");

    IT("emits if a copy of the Value sets a new value")
    {
        Value copy(value);
        copy.setValue("Set by copy");
        varxRunDispatchLoop();

        varxRequireItems(items, "Initial Item", "Set by copy");
    }

    IT("emites only one item if the Value is set multiple times synchronously")
    {
        value = "2";
        value = "3";
        value = "4";
        varxRunDispatchLoop();

        varxRequireItems(items, "Initial Item", "4");
    }

    IT("notifies multiple Disposables on subscribe")
    {
        Observable another = Observable::fromValue(value);
        varxCollectItems(another, items);

        varxRequireItems(items, "Initial Item", "Initial Item");
    }

    IT("notifies multiple Values referring to the same ValueSource")
    {
        Value anotherValue(value);
        Observable anotherObservable = Observable::fromValue(anotherValue);
        varxCollectItems(anotherObservable, items);

        varxRequireItems(items, "Initial Item", "Initial Item");
    }

    IT("notifies multiple Disposables if a Value is set multiple times")
    {
        DisposeBag disposeBag;
        observable.subscribe([&](String newValue) {
                      items.add(newValue.toUpperCase());
                  })
            .disposedBy(disposeBag);

        value = "Bar";
        varxRunDispatchLoop();

        value = "Baz";
        varxRunDispatchLoop();

        CHECK(items.size() == 6);

        // Subscribers are notified in no particular order
        for (auto s : { "Initial Item", "INITIAL ITEM", "BAR", "Bar", "BAZ", "Baz" })
            REQUIRE(items.contains(s));
    }
}


TEST_CASE("Observable::fromValue lifetime",
          "[Observable][Observable::fromValue]")
{
    // Create an Observable from a Value
    Value value(String("Initial"));
    auto source = std::make_shared<Observable>(Observable::fromValue(value));

    // Create another Observable from the source Observable
    auto mapped = source->map([](String s) { return s; });

    // Collect items from the mapped Observable
    Array<var> items;
    varxCollectItems(mapped, items);

    varxCheckItems(items, "Initial");

    IT("emits items when the source Observable is alive")
    {
        value.setValue("New Value");
        varxRunDispatchLoop();

        varxRequireItems(items, "Initial", "New Value");
    }

    IT("stops emitting items as soon as the source Observable is destroyed")
    {
        source.reset();
        value.setValue("Two");
        value.setValue("Three");
        varxRunDispatchLoop();

        varxRequireItems(items, "Initial");
    }

    IT("does not emit an item if the Observable is destroyed immediately after calling setValue")
    {
        value.setValue("New Value");
        source.reset();
        varxRunDispatchLoop();

        varxRequireItems(items, "Initial");
    }

    IT("continues to emit items if the source Observable is copied and then destroyed")
    {
        auto copy = std::make_shared<Observable>(*source);
        Array<var> copyItems;
        varxCollectItems(*copy, copyItems);

        varxCheckItems(copyItems, "Initial");

        source.reset();
        varxRunDispatchLoop();
        value.setValue("New");
        varxRunDispatchLoop();

        varxRequireItems(copyItems, "Initial", "New");
    }

    IT("notified onComplete when the Observable is destroyed")
    {
        bool completed = false;
        source->subscribe([](var) {}, [](Error) {}, [&]() { completed = true; });
        CHECK(!completed);

        source.reset();

        REQUIRE(completed);
    }
}


TEST_CASE("Observable::fromValue with a Slider",
          "[Observable][Observable::fromValue]")
{
    Slider slider;
    slider.setValue(7.6);
    Observable o = Observable::fromValue(slider.getValueObject());
    Array<var> items;
    varxCollectItems(o, items);
    varxCheckItems(items, 7.6);

    IT("emits once if the Slider is changed once")
    {
        slider.setValue(0.45);
        varxRunDispatchLoop();

        varxRequireItems(items, 7.6, 0.45);
    }

    IT("emits just once if the Slider value changes rapidly")
    {
        for (double value : { 3.41, 9.54, 4.67, 3.56 })
            slider.setValue(value);

        varxRunDispatchLoop();

        varxRequireItems(items, 7.6, 3.56);
    }
}


TEST_CASE("Observable::interval",
          "[Observable][Observable::interval]")
{
    IT("can create an interval below one second")
    {
        auto o = Observable::interval(RelativeTime::seconds(0.003)).take(3);
        auto lastTime = Time::getCurrentTime();
        Array<RelativeTime> intervals;
        Array<var> ints;
        o.subscribe([&](int i) {
            auto time = Time::getCurrentTime();
            intervals.add(time - lastTime);
            lastTime = time;
            ints.add(i);
        });

        CHECK(intervals.size() == 3);
        REQUIRE(intervals[0].inSeconds() == Approx(0).epsilon(0.01));
        REQUIRE(intervals[1].inSeconds() == Approx(0.003).epsilon(0.001));
        REQUIRE(intervals[2].inSeconds() == Approx(0.003).epsilon(0.001));

        varxRequireItems(ints, 1, 2, 3);
    }
}


TEST_CASE("Observable::just",
          "[Observable][Observable::just]")
{
    Array<var> items;

    IT("emits a single value on subscribe")
    {
        varxCollectItems(Observable::just(18.3), items);

        varxRequireItems(items, 18.3);
    }

    IT("notifies multiple disposables")
    {
        Observable o = Observable::just("Hello");
        varxCollectItems(o, items);
        varxCollectItems(o, items);

        varxRequireItems(items, "Hello", "Hello");
    }
}


TEST_CASE("Observable::never",
          "[Observable][Observable::never]")
{
    auto o = Observable::never();
    DisposeBag disposeBag;

    IT("doesn't terminate and doesn't emit")
    {
        bool onNextCalled = false;
        bool onErrorCalled = false;
        bool onCompletedCalled = false;
        o.subscribe([&](var) { onNextCalled = true; },
                    [&](Error) { onErrorCalled = true; },
                    [&]() { onCompletedCalled = true; })
            .disposedBy(disposeBag);

        varxRunDispatchLoop();

        REQUIRE(!onNextCalled);
        REQUIRE(!onErrorCalled);
        REQUIRE(!onCompletedCalled);
    }
}


TEST_CASE("Observable::range",
          "[Observable][Observable::range]")
{
    Array<var> items;

    IT("emits integer numbers with an integer range")
    {
        varxCollectItems(Observable::range(3, 7, 3), items);
        varxRequireItems(items, 3, 6, 7);
    }

    IT("emits double numbers with a double range")
    {
        varxCollectItems(Observable::range(17.5, 22.8, 2), items);
        varxRequireItems(items, 17.5, 19.5, 21.5, 22.8);
    }

    IT("emits just start if start == end")
    {
        varxCollectItems(Observable::range(10, 10), items);
        varxRequireItems(items, 10);
    }

    IT("throws if start > end")
    {
        REQUIRE_THROWS_WITH(Observable::range(10, 9), Contains("Invalid range"));
    }
}


TEST_CASE("Observable::repeat",
          "[Observable][Observable::repeat]")
{
    Array<var> items;

    IT("repeats an item indefinitely")
    {
        varxCollectItems(Observable::repeat(8).take(9), items);

        varxRequireItems(items, 8, 8, 8, 8, 8, 8, 8, 8, 8);
    }

    IT("repeats an items a limited number of times")
    {
        varxCollectItems(Observable::repeat("4", 7), items);

        varxRequireItems(items, "4", "4", "4", "4", "4", "4", "4");
    }
}

TEST_CASE("TypedObservable<T>")
{
    auto o1 = TypedObservable<Point<int>>::just(Point<int>(14, -2));
    auto o2 = TypedObservable<double>::from({3.14, 5.2, 110.3});
    auto o3 = TypedObservable<>::fromValue(Value());
    auto o4 = TypedObservable<>::interval(RelativeTime());
    auto o5 = TypedObservable<>::range(4, 18);
    auto o6 = TypedObservable<>::range(4.5345f, 15.13f);
    
    o1.subscribe([](const Point<int>& p) {});
    
//    TypedObserver<Point<String>> *observer;
//    o1.subscribe(*observer);
    
    auto combined = o1.combineLatest(o6, [](Point<int> p, double f) {
        return String(p.x) + ", " + String(p.y) + ", " + String(f);
    });
    
    combined.subscribe([](String s) {});
    
    auto higherOrder = o2.map([](float f) {
        return TypedObservable<>::range(0.0, f + 10.1);
    });
    
    auto firstOrder = higherOrder.switchOnNext();
    firstOrder.subscribe([](int i) {});
}


// Dummy struct that just counts copy and move constructions
struct CopyAndMoveConstructible
{
    CopyAndMoveConstructible(int& numCopyConstructorCalls, int& numMoveConstructorCalls)
    : numCopyConstructorCalls(numCopyConstructorCalls),
      numMoveConstructorCalls(numMoveConstructorCalls)
    {}

    // Copy Constructor
    CopyAndMoveConstructible(const CopyAndMoveConstructible& other)
    : numCopyConstructorCalls(other.numCopyConstructorCalls),
      numMoveConstructorCalls(other.numMoveConstructorCalls)
    {
        numCopyConstructorCalls++;
    }

    // Move constructor
    CopyAndMoveConstructible(CopyAndMoveConstructible&& other)
    : numCopyConstructorCalls(other.numCopyConstructorCalls),
      numMoveConstructorCalls(other.numMoveConstructorCalls)
    {
        numMoveConstructorCalls++;
    }

    int& numCopyConstructorCalls;
    int& numMoveConstructorCalls;
};

VARX_DEFINE_VARIANT_CONVERTER(CopyAndMoveConstructible)

TEST_CASE("VariantConverter<T>")
{
    IT("prefers the move constructor when wrapping in a juce::var")
    {
        // Counters for copy and move constructions
        int numCopyConstructorCalls = 0;
        int numMoveConstructorCalls = 0;
        
        // Create instance of custom type
        CopyAndMoveConstructible original(numCopyConstructorCalls, numMoveConstructorCalls);
        CHECK(numCopyConstructorCalls == 0);
        CHECK(numMoveConstructorCalls == 0);
        
        // Wrap custom type in a var using rvalue
        var v(toVar(std::move(original)));
        
        // Only the move constructor should have been called
        REQUIRE(numCopyConstructorCalls == 0);
        REQUIRE(numMoveConstructorCalls == 1);
        
        IT("uses the copy constructor when unwrapping")
        {
            auto unwrapped = fromVar<CopyAndMoveConstructible>(v);
            REQUIRE(numCopyConstructorCalls == 1);
            REQUIRE(numMoveConstructorCalls == 1);
        }
    }
}
