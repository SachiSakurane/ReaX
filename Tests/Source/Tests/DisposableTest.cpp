/*
  ==============================================================================

    DisposableTest.cpp
    Created: 28 Apr 2017 8:12:29pm
    Author:  Martin Finke

  ==============================================================================
*/

#include "../Other/TestPrefix.h"


TEST_CASE("Disposable",
          "[Disposable]")
{
    // Create Observable which emits a single item asynchronously
    auto observable = std::make_shared<Observable>(Observable::create([](Observer observer) {
        MessageManager::getInstance()->callAsync([observer]() mutable {
            observer.onNext("Item");
        });
    }));

    // Subscribe to it
    Array<var> items;
    auto disposable = std::make_shared<Disposable>(observable->subscribe([&](String item) {
        items.add(item);
    }));

    IT("received items while being subscribed")
    {
        varxRunDispatchLoop();

        varxRequireItems(items, "Item");
    }

    IT("does not receive items after disposing")
    {
        disposable->dispose();
        varxRunDispatchLoop();

        REQUIRE(items.isEmpty());
    }

    IT("takes ownership when move constructing")
    {
        Disposable other = std::move(*disposable);
        other.dispose();
        varxRunDispatchLoop();

        REQUIRE(items.isEmpty());
    }

    IT("does not dispose when being destroyed")
    {
        disposable.reset();
        varxRunDispatchLoop();

        varxRequireItems(items, "Item");
    }

    IT("continues to receive items after the Observable is gone")
    {
        observable.reset();
        varxRunDispatchLoop();

        varxRequireItems(items, "Item");
    }

    // Unsubscribe after each IT(), to prevent old disposables from filling the items array
    if (disposable)
        disposable->dispose();
}


TEST_CASE("DisposeBag",
          "[DisposeBag]")
{
    auto disposeBag = std::make_shared<DisposeBag>();

    // Create Observable which emits a single item asynchronously
    auto observable = Observable::create([](Observer observer) {
        MessageManager::getInstance()->callAsync([observer]() mutable {
            observer.onNext("Item");
        });
    });

    // Subscribe to it
    Array<var> items;
    observable.subscribe([&](String item) {
                  items.add(item);
              })
        .disposedBy(*disposeBag);

    IT("received items while not destroyed")
    {
        varxRunDispatchLoop();

        varxRequireItems(items, "Item");
    }

    IT("does not receive items after being destroyed")
    {
        disposeBag.reset();
        varxRunDispatchLoop();

        REQUIRE(items.isEmpty());
    }

    IT("can dispose multiple Disposables")
    {
        for (int i = 0; i < 5; ++i) {
            observable.subscribe([&](String item) {
                          items.add(item);
                      })
                .disposedBy(*disposeBag);
        }

        disposeBag.reset();
        varxRunDispatchLoop();

        REQUIRE(items.isEmpty());
    }
}
