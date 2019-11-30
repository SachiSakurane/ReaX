#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcomma"
#if __has_include(<rxcpp/rx.hpp>)
    #include <rxcpp/rx.hpp>
#else
    #include "RxCpp/Rx/v2/src/rxcpp/rx.hpp"
#endif
#pragma clang diagnostic pop

// Enable stricter warnings
#include "util/internal/reax_ExtraWarnings.h"
#pragma clang diagnostic push
REAX_ENABLE_EXTRA_WARNINGS

namespace reax {
using namespace juce;

#include "util/internal/reax_any.h"
    
#include "rx/reax_Subscription.h"
#include "rx/internal/reax_Observable_Impl.h"
#include "rx/reax_Scheduler.h"
#include "rx/internal/reax_Observer_Impl.h"
#include "rx/internal/reax_Scheduler_Impl.h"
#include "rx/internal/reax_Subjects_Impl.h"
#include "rx/reax_DisposeBag.h"
#include "rx/reax_Subscription.cpp"
#include "rx/reax_DisposeBag.cpp"
#include "rx/reax_Scheduler.cpp"
#include "rx/internal/reax_Scheduler_Impl.cpp"
#include "rx/internal/reax_Observable_Impl.cpp"
#include "rx/internal/reax_Observer_Impl.cpp"
#include "rx/internal/reax_Subjects_Impl.cpp"
}

#pragma clang diagnostic pop
