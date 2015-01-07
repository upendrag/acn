#include <iostream>
#include <string>
#include <list>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <netinet/in.h>

#include <vector>
#include <router.h>

using namespace CppUnit;
using namespace std;

//-----------------------------------------------------------------------------

class TestRouter : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestRouter);
    CPPUNIT_TEST(testParse);
    CPPUNIT_TEST(testbroadcastLSA);
    CPPUNIT_TEST_SUITE_END();

    public:
        void setUp(void);
        void tearDown(void);

    protected:
        void testParse(void);
        void testbroadcastLSA(void);
    
    private:
        Router *router;       
};

//-----------------------------------------------------------------------------

void TestRouter::testParse(void)
{    
    CPPUNIT_ASSERT("08" == router->getId());
    vector<IPAddress> ips = router->getIpAddresses();
    CPPUNIT_ASSERT("01" == ips.at(0).getNetNum());
    CPPUNIT_ASSERT("10" == ips.at(0).getHostNum());
    CPPUNIT_ASSERT("11" == ips.at(1).getNetNum());
    CPPUNIT_ASSERT("02" == ips.at(1).getHostNum());
}

void TestRouter::testbroadcastLSA(void)
{
    CPPUNIT_ASSERT(0 == router->getBroadcastId());
    router->broadcastLSA();
    CPPUNIT_ASSERT(1 == router->getBroadcastId());
}

void TestRouter::setUp(void)
{
    char *argv[] = { "irouter", "08", "01", "10", "11", "02" };
    router = new Router(6, argv);
}

void TestRouter::tearDown(void)
{
}

//-----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_REGISTRATION(TestRouter);

int main(int argc, char* argv[])
{
     // informs test-listener about testresults
     CPPUNIT_NS::TestResult testresult;

     // register listener for collecting the test-results
     CPPUNIT_NS::TestResultCollector collectedresults;
     testresult.addListener (&collectedresults);

     // register listener for per-test progress output
     CPPUNIT_NS::BriefTestProgressListener progress;
     testresult.addListener (&progress);

     // insert test-suite at test-runner by registry
     CPPUNIT_NS::TestRunner testrunner;
     testrunner.addTest (CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest ());
     testrunner.run(testresult);

     // output results in compiler-format
     CPPUNIT_NS::CompilerOutputter compileroutputter(&collectedresults, std::cerr);
     compileroutputter.write ();

     // Output XML for Jenkins CPPunit plugin
     //ofstream xmlFileOut("cppTestBasicMathResults.xml");
     //XmlOutputter xmlOut(&collectedresults, xmlFileOut);
     //xmlOut.write();

     // return 0 if tests were successful
     return collectedresults.wasSuccessful() ? 0 : 1;
}
          
