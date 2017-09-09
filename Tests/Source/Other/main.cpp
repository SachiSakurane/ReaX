#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "JuceHeader.h"

class TestRunnerApplication : public JUCEApplication
{
public:
    void initialise (const String& commandLine) override
    {
        Catch::ConfigData config;
//		config.shouldDebugBreak = true;
        
        Catch::Session session;
        session.useConfigData(config);
        session.run();
        
        quit();
    }
    
    void shutdown() override
    {}
    
    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return false; }
};

START_JUCE_APPLICATION(TestRunnerApplication)
