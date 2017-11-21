/**
 * Ping Pong test - measures time difference between PING and PONG messages
 */


#include <iostream> // for io
#include <thread> // for threads
#include <chrono> // for thread sleep
#include <string> // for string handling
#include <unistd.h> // for getopt
#include <sstream> // string operations
#include <string.h> // strings
#include <vector> // for vectors
#include <time.h>

#include <sys/time.h>

#include "Ivycpp.h"
#include "IvyApplication.h"

using namespace std;

class PingPongTest : public IvyApplicationCallback, public IvyMessageCallback {
public:
  Ivy *bus;

  void Start();
  void OnApplicationConnected(IvyApplication *app);
  void OnApplicationDisconnected(IvyApplication *app);
  void OnApplicationCongestion(IvyApplication *app);
  void OnApplicationDecongestion(IvyApplication *app);
  void OnApplicationFifoFull(IvyApplication *app);
  void OnMessage(IvyApplication *app, int argc, const char **argv){};

  IvyMessageCallbackFunction cb_ping;
  IvyMessageCallbackFunction cb_pong;

  static void OnPING(IvyApplication *app, void *user_data, int argc, const char **argv);
  static void OnPONG(IvyApplication *app, void *user_data, int argc, const char **argv);

  static void ivy_thread(PingPongTest *test);

  PingPongTest(char *domain, bool debug, char* name);
  PingPongTest(char *domain, bool debug);
  PingPongTest(char *domain);
  PingPongTest();


private:
  static void ivyAppConnCb( IvyApplication *app ) {};
  static void ivyAppDiscConnCb( IvyApplication *app ) {};

  const char *bus_domain_;
  const char* PING = "^(\\S*) PING";
  const char* PONG = "^(\\S*) PONG";

  struct timeval  ping_time, pong_time;
  double avg_dt;
  static constexpr double alpha = 0.9;

  string name_ = "ping_pong"; // ivy node name
};

PingPongTest::PingPongTest(char *domain, bool debug, char* name)
: cb_ping(OnPING,this),
  cb_pong(OnPONG,this)
  {
  avg_dt = 0;
  bus_domain_= domain;
  if(name!=NULL){
    this->name_ = string(name);
  }
  bus = new Ivy(this->name_.c_str(), "PingPong READY",
      BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}

PingPongTest::PingPongTest(char *domain, bool debug)
: cb_ping(OnPING,this),
  cb_pong(OnPONG,this)
  {
  avg_dt = 0;
  bus_domain_= domain;
  bus = new Ivy(this->name_.c_str(), "PingPong READY",
      BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}

PingPongTest::PingPongTest(char *domain)
: cb_ping(OnPING,this),
  cb_pong(OnPONG,this)
  {
  avg_dt = 0;
  bus_domain_= domain;
  bus = new Ivy(this->name_.c_str(), "PingPong READY",
      BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}


PingPongTest::PingPongTest()
: cb_ping(OnPING,this),
  cb_pong(OnPONG,this)
  {
  avg_dt = 0;
  bus_domain_= NULL;
  bus = new Ivy(this->name_.c_str(), "PingPong READY",
      BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}


void PingPongTest::Start()
{
  // bind messages
  bus->BindMsg(PING, &cb_ping);
  bus->BindMsg(PONG, &cb_pong);

  // start bus
  bus->start(bus_domain_);

  // enter main loop
  bus->ivyMainLoop();
}


void PingPongTest::OnPING ( IvyApplication *app, void *user_data, int argc, const char **argv )
{
  PingPongTest* copilot = static_cast<PingPongTest*>(user_data);
  printf ("Got PING message.\n");
  gettimeofday(&copilot->ping_time, NULL);
}

void PingPongTest::OnPONG( IvyApplication *app, void *user_data, int argc, const char **argv )
{
  PingPongTest* copilot = static_cast<PingPongTest*>(user_data);
  printf ("Got PONG message.\n");
  gettimeofday(&copilot->pong_time, NULL);

  double dt = (double) (copilot->pong_time.tv_usec - copilot->ping_time.tv_usec) / 1000000 +
      (double) (copilot->pong_time.tv_sec - copilot->ping_time.tv_sec);
  copilot->avg_dt = PingPongTest::alpha * copilot->avg_dt + (1.0 - PingPongTest::alpha) * dt;
  cout << "PingPongTest::alpha * copilot->avg_dt =" << PingPongTest::alpha * copilot->avg_dt <<
      ", (1.0 - PingPongTest::alpha) * dt = " << (1.0 - PingPongTest::alpha) * dt << endl;
  cout << "Ping times: current: " << dt << "[s], average: " << copilot->avg_dt << "[s]" << endl;
}


void PingPongTest :: OnApplicationConnected (IvyApplication *app)
{
  const char *appname;
  const char *host;
  appname = app->GetName();
  host = app->GetHost();
  printf("%s connected from %s\n", appname,  host);
}

void PingPongTest :: OnApplicationDisconnected (IvyApplication *app)
{
  const char *appname;
  const char *host;
  appname = app->GetName ();
  host = app->GetHost();
  printf("%s disconnected from %s\n", appname,  host);
}

void PingPongTest::OnApplicationCongestion(IvyApplication *app)
{
  std::cerr << "Ivy Congestion notififation\n";
}
void PingPongTest::OnApplicationDecongestion(IvyApplication *app)
{
  std::cerr << "Ivy Decongestion notififation\n";
}
void PingPongTest::OnApplicationFifoFull(IvyApplication *app)
{
  std::cerr << "Ivy FIFO Full  notififation : MESSAGE WILL BE LOST\n";
}


/**
 * This thread starts the Ivy Bus and the enters the IvyMainLoop
 */
void PingPongTest::ivy_thread(PingPongTest *test)
{
  test->Start();
}



void showhelpinfo(char *s) {
  cout<<"Usage:   "<<s<<" [-option] [argument]"<<endl;
  cout<<"option:  "<<"-h  show help information"<<endl;
  cout<<"         "<<"-b ivy bus (default is 127.255.255.255:2010)"<<endl;
  cout<<"         "<<"-d simulation mode on/off (default is false, use 'true' or '1')"<<endl;
  cout<<"         "<<"-n name (default is \"PingPongTest\")"<<endl;
  cout<<"example: "<<s<<" -b 10.0.0.255:2010"<<endl;
}

int main(int argc, char** argv) {
  char tmp;
  char* ivy_bus = NULL;
  bool debug = false;
  char* name = NULL;

  while((tmp=getopt(argc,argv,"hb:d:n:"))!=-1)
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
      case 'd':
        // check values of debug argument
        if (!strcmp("true",optarg)) {
          // equals true
          debug = true;
        }
        if (!strcmp("1",optarg)) {
          debug = true;
        }
        break;
      case 'n':
        name = optarg;
        break;
      /*do nothing on default*/
      default:
        break;
    }
  }

  PingPongTest test(ivy_bus, debug, name);

  //Launch a thread
  std::thread t1(PingPongTest::ivy_thread, &test);

  //Wait for the ivy_thread to end
  t1.join();

  return 0;
}
