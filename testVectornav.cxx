/**
 * This program sends VECTORNAV_INFO message
 * at a specified rate.
 *
 *   <message name="VECTORNAV_INFO" id="23">
 *     <field name="timestamp"     type="float"    unit="s"/>
 *     <field name="chksm_error"   type="uint32"/>
 *     <field name="hdr_error"     type="uint32"/>
 *     <field name="rate" type="uint16" unit="packets/s"/>
 *     <field name="ins_status" type="uint8" values="NoTracking|OutOfSpecs|OK"/>
 *     <field name="ins_err" type="uint8"/>
 *     <field name="YprU1"     type="float"    unit="deg"/>
 *     <field name="YprU2"     type="float"    unit="deg"/>
 *     <field name="YprU3"     type="float"    unit="deg"/>
 *   </message>
 */

#include <iostream> // for io
#include <thread> // for threads
#include <chrono> // for thread sleep and time count
#include <string> // for string handling
#include <unistd.h> // for getopt
#include <time.h>   // time_t, struct tm, time, localtime
#include <fstream> // Stream class to both read and write from/to files.
#include <sstream> // string operations
#include <string.h> // strings
#include <vector> // for vectors


#include "Ivycpp.h"
#include "IvyApplication.h"

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds milliseconds;

using namespace std;

class VectornavTest : public IvyApplicationCallback, public IvyMessageCallback {
public:
  Ivy *bus;

  void Start();
  void OnApplicationConnected(IvyApplication *app);
  void OnApplicationDisconnected(IvyApplication *app);
  void OnApplicationCongestion(IvyApplication *app);
  void OnApplicationDecongestion(IvyApplication *app);
  void OnApplicationFifoFull(IvyApplication *app);
  void OnMessage(IvyApplication *app, int argc, const char **argv){};

  static void periodic_send_status(VectornavTest *test);
  static void ivy_thread(VectornavTest *test);

  VectornavTest(char *domain, int rate);
  VectornavTest(char *domain);
  VectornavTest();


private:
  static void ivyAppConnCb( IvyApplication *app ) {};
  static void ivyAppDiscConnCb( IvyApplication *app ) {};

  const char *bus_domain_;
  int rate_;
  float timestamp_;

  Clock::time_point t0;

  const char* name_ = "vectornav-sim"; // ivy node name
  const char* msg_name_ = "VECTORNAV_INFO"; // message name
  const int default_rate_ = 100; // 100Hz by default
};

VectornavTest::VectornavTest(char *domain, int rate)
  {
  rate_ = rate;
  timestamp_ = 0.0;
  bus_domain_= domain;
  bus = new Ivy( "VectornavTest", "VectornavTest READY",
      BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}

VectornavTest::VectornavTest(char *domain)
  {
  rate_ = default_rate_;
  timestamp_ = 0.0;
  bus_domain_= domain;
  bus = new Ivy( "VectornavTest", "VectornavTest READY",
      BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}


VectornavTest::VectornavTest()
  {
  rate_ = default_rate_;
  timestamp_ = 0.0;
  bus_domain_= NULL;
  bus = new Ivy( "VectornavTest", "VectornavTest READY",
      BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}

void VectornavTest::Start()
{
  // start bus
  bus->start(bus_domain_);

  // enter main loop
  bus->ivyMainLoop();
}




void VectornavTest :: OnApplicationConnected (IvyApplication *app)
{
  const char *appname;
  const char *host;
  appname = app->GetName();
  host = app->GetHost();
  printf("%s connected from %s\n", appname,  host);
}

void VectornavTest :: OnApplicationDisconnected (IvyApplication *app)
{
  const char *appname;
  const char *host;
  appname = app->GetName ();
  host = app->GetHost();
  printf("%s disconnected from %s\n", appname,  host);
}

void VectornavTest::OnApplicationCongestion(IvyApplication *app)
{
  std::cerr << "Ivy Congestion notififation\n";
}
void VectornavTest::OnApplicationDecongestion(IvyApplication *app)
{
  std::cerr << "Ivy Decongestion notififation\n";
}
void VectornavTest::OnApplicationFifoFull(IvyApplication *app)
{
  std::cerr << "Ivy FIFO Full  notififation : MESSAGE WILL BE LOST\n";
}


/**
 * This thread starts the Ivy Bus and the enters the IvyMainLoop
 */
void VectornavTest::ivy_thread(VectornavTest *test)
{
  test->Start();
}

/**
 *
 *   <message name="VECTORNAV_INFO" id="23">
 *     <field name="timestamp"     type="float"    unit="s"/>
 *     <field name="chksm_error"   type="uint32"/>
 *     <field name="hdr_error"     type="uint32"/>
 *     <field name="rate" type="uint16" unit="packets/s"/>
 *     <field name="ins_status" type="uint8" values="NoTracking|OutOfSpecs|OK"/>
 *     <field name="ins_err" type="uint8"/>
 *     <field name="YprU1"     type="float"    unit="deg"/>
 *     <field name="YprU2"     type="float"    unit="deg"/>
 *     <field name="YprU3"     type="float"    unit="deg"/>
 *   </message>
 */
void VectornavTest::periodic_send_status(VectornavTest *test)
{
  static uint status = 2; // status OK
  static uint rate = 100; // fake VN rate
  static uint err = 0; // dummy for errors
  static float unc = 1.0; // dummy for uncertainty

  test->t0 = Clock::now();

  while(true){
    // get current time stamp
    Clock::time_point t1 = Clock::now();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(t1 - test->t0);
    float dt = (ms.count())/1000.0;

    test->timestamp_ = dt;

    test->bus->SendMsg("%s %s %f %u %u %u %u %u %f %f %f",
            test->name_,
            test->msg_name_,
            test->timestamp_,
            err, err, rate, status, err, unc, unc, unc);
    std::this_thread::sleep_for(std::chrono::milliseconds(test->rate_));
  }
}




void showhelpinfo(char *s) {
  cout<<"Usage:   "<<s<<" [-option] [argument]"<<endl;
  cout<<"option:  "<<"-h  show help information"<<endl;
  cout<<"         "<<"-b ivy bus (default is 127.255.255.255:2010)"<<endl;
  cout<<"         "<<"-r message rate in milliseconds"<<endl;
  cout<<"example: "<<s<<" -b 10.0.0.255:2010"<<endl;
}

int main(int argc, char** argv) {
  char tmp;
  char* ivy_bus = NULL;
  int rate = 1000; // 1s rate is default

  while((tmp=getopt(argc,argv,"hb:r:"))!=-1)
  {
    switch(tmp)
    {
      /*option h show the help information*/
      case 'h':
        showhelpinfo(argv[0]);
        exit(1);
        break;
      case 'b':
        ivy_bus = optarg;
        cout << "-b " << ivy_bus << endl;
        break;
      case 'r':
        rate = atoi(optarg);
        if (rate == 0) {
          cout << "Incorrect argument. Only integers are allowed." << endl;
          return -1;
        }
        break;
      /*do nothing on default*/
      default:
        break;
    }
  }

  VectornavTest test(ivy_bus, rate);


  //Launch a thread
  std::thread t1(VectornavTest::ivy_thread, &test);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::thread t2(VectornavTest::periodic_send_status, &test);


  //Wait for the ivy_thread to end
  t1.join();

  return 0;
}
