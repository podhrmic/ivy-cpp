/**
 * This application simulates Copilot running on Gumstix Computer
 * It has the following functions:
 * - gets timestamp from TIME message prior logging
 * - logs all incoming IVY messages on disk
 * - sends periodically COPILOT_INFO message
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

enum CopilotStatus {
  UNKNOWN,
  INIT,
  LOGGING,
  FAULT
};

using namespace std;

class CopilotTest : public IvyApplicationCallback, public IvyMessageCallback {
public:
  Ivy *bus;

  void Start();
  void OnApplicationConnected(IvyApplication *app);
  void OnApplicationDisconnected(IvyApplication *app);
  void OnApplicationCongestion(IvyApplication *app);
  void OnApplicationDecongestion(IvyApplication *app);
  void OnApplicationFifoFull(IvyApplication *app);
  void OnMessage(IvyApplication *app, int argc, const char **argv);

  IvyMessageCallbackFunction cb_time;

  static void OnTIME(IvyApplication *app, void *user_data, int argc, const char **argv);

  static std::vector<std::string> parse_msg(const char **argv);

  static void periodic_send_status(CopilotTest *test);
  static void ivy_thread(CopilotTest *test);

  CopilotTest(char *domain, bool debug);
  CopilotTest(char *domain);
  CopilotTest();


private:
  static void ivyAppConnCb( IvyApplication *app ) {};
  static void ivyAppDiscConnCb( IvyApplication *app ) {};

  const char *bus_domain_;
  const char* TIME = "^(\\S*) TIME (\\S*)";

  time_t rawtime_;
  struct tm * timeinfo_;
  float sec_since_startup_;

  enum CopilotStatus status_;

  Clock::time_point t0;

  const char* name_ = "copilot"; // ivy node name
  bool debug_; // are we in debug mode?

  // sender message names
  std::vector<std::string> copilot_status_ = {"COPILOT_STATUS_DL","COPILOT_STATUS"};
};

CopilotTest::CopilotTest(char *domain, bool debug)
: cb_time(OnTIME,this)
  {
  debug_ = debug;
  sec_since_startup_ = 0.0;
  status_ = INIT;
  rawtime_ = 0;
  timeinfo_ = NULL;
  bus_domain_= domain;
  bus = new Ivy( "CopilotTest", "CopilotTest READY",
      BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}

CopilotTest::CopilotTest(char *domain)
: cb_time(OnTIME,this)
  {
  debug_ = false;
  sec_since_startup_ = 0.0;
  status_ = INIT;
  rawtime_ = 0;
  timeinfo_ = NULL;
  bus_domain_= domain;
  bus = new Ivy( "CopilotTest", "CopilotTest READY",
      BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}


CopilotTest::CopilotTest()
: cb_time(OnTIME,this)
  {
  debug_ = false;
  sec_since_startup_ = 0.0;
  status_ = INIT;
  rawtime_ = 0;
  timeinfo_ = NULL;
  bus_domain_= NULL;
  bus = new Ivy( "CopilotTest", "CopilotTest READY",
      BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}

void CopilotTest::Start()
{
  // bind messages
  bus->BindMsg( "(.*)", this );
  bus->BindMsg(TIME, &cb_time);

  // start bus
  bus->start(bus_domain_);

  // enter main loop
  bus->ivyMainLoop();
}


void CopilotTest::OnTIME ( IvyApplication *app, void *user_data, int argc, const char **argv )
{
  CopilotTest* copilot = static_cast<CopilotTest*>(user_data);

  if (copilot->status_ == INIT) {
    // parse time val
    int val = atoi(argv[1]);
    copilot->rawtime_ = val;

    // update local time
    copilot->timeinfo_ = localtime(&copilot->rawtime_);
    cout << "New Current local time and date: " <<  asctime(copilot->timeinfo_) << endl;

    // set status
    copilot->status_ = LOGGING;

    // update clock
    copilot->t0 = Clock::now();
  }


}

/**
 * Parse the incoming messages
 */
std::vector<std::string> CopilotTest::parse_msg(const char **argv)
{
  std::string s(argv[0]);
  std::string delimiter = " ";
  std::vector<std::string> argList;

  size_t pos = 0;
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos) {
      token = s.substr(0, pos);
      std::string tmp = token;
      argList.push_back(tmp);
      s.erase(0, pos + delimiter.length());
  }
  argList.push_back(s);


#if DEBUG
  cout << "Original message: " << endl;
  cout << argv[0] << endl;

  for(std::vector<string>::iterator it = argList.begin(); it != argList.end(); ++it) {
      cout << *it << " ";
  }
  cout << endl;
#endif

  return argList;
}

/**
 * Logging all incoming messages
 */
void CopilotTest :: OnMessage(IvyApplication *app, int argc, const char **argv)
{

  std::vector<std::string> argList = parse_msg(argv);

  if (argList.size() > 1) {
    // log data
    if (this->status_ == LOGGING)
    {

      static ofstream logfile;

      int year = this->timeinfo_->tm_year - 100;
      string y = to_string(year);

      int mon = this->timeinfo_->tm_mon;
      string mo;
      if (mon < 10) {
        mo = "0" + to_string(mon);
      }
      else {
        mo = to_string(mon);
      }

      int day = this->timeinfo_->tm_mday;
      string d;
      if (day < 10) {
        d = "0" + to_string(day);
      }
      else {
        d = to_string(day);
      }

      int hour = this->timeinfo_->tm_hour;
      string h;
      if (hour<10){
        h = "0"+to_string(hour);
      }
      else {
        h=to_string(hour);
      }

      int min = this->timeinfo_->tm_min;
      string m;
      if (min<10){
        m = "0" + to_string(min);
      }
      else {
        m = to_string(min);
      }

      int sec = this->timeinfo_->tm_sec;
      string s;
      if (sec<10){
        s = "0" + to_string(sec);
      }
      else {
        s = to_string(sec);
      }

      // YY_MM_DD__HH_MM_SS.data
      static std::string fname =  y + "_" + mo + "_" +
          d + "__" + h + "_" + m + "_" + s +
          ".data";

      static bool show = true;

      if (show) {
        cout << "Logging to: " << fname << endl;
        show=false;
      }

      logfile.open(fname.c_str(), std::ios_base::app);

      // get current time stamp
      Clock::time_point t1 = Clock::now();
      milliseconds ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
      float dt = (ms.count())/1000.0;

      // write message
      logfile << dt << " ";
      logfile << argList[0] << " ";
      for  (int i = 1; i < argList.size()-1; i++)
      {
        logfile << argList[i] << " ";
      }
      logfile << argList[argList.size()-1] << endl;

      logfile.close();
    }
  }
}


void CopilotTest :: OnApplicationConnected (IvyApplication *app)
{
  const char *appname;
  const char *host;
  appname = app->GetName();
  host = app->GetHost();
  printf("%s connected from %s\n", appname,  host);
}

void CopilotTest :: OnApplicationDisconnected (IvyApplication *app)
{
  const char *appname;
  const char *host;
  appname = app->GetName ();
  host = app->GetHost();
  printf("%s disconnected from %s\n", appname,  host);
}

void CopilotTest::OnApplicationCongestion(IvyApplication *app)
{
  std::cerr << "Ivy Congestion notififation\n";
}
void CopilotTest::OnApplicationDecongestion(IvyApplication *app)
{
  std::cerr << "Ivy Decongestion notififation\n";
}
void CopilotTest::OnApplicationFifoFull(IvyApplication *app)
{
  std::cerr << "Ivy FIFO Full  notififation : MESSAGE WILL BE LOST\n";
}


/**
 * This thread starts the Ivy Bus and the enters the IvyMainLoop
 */
void CopilotTest::ivy_thread(CopilotTest *test)
{
  test->Start();
}

/**
 *
    <message name="COPILOT_STATUS_DL" id="33" link="forwarded">
      <field name="ac_id" type="uint8"/>
      <field name="timestamp" type="float" unit="s">Mission computer timestamp</field>
      <field name="used_memory" type="uint8" unit="%">Percentage of used memory (RAM) of the mission computer rounded up to whole percent</field>
      <field name="used_disk" type="uint8" unit="%">Percentage of used disk of the mission computer rounded up to whole percent</field>
      <field name="status" type="uint8" values="UNKNOWN|RUNNING|FAULT">Mission computer status</field>
      <field name="error_code" type="uint8" values="NONE|IO_ERROR">Error codes of the mission computer</field>
    </message>
 */
void CopilotTest::periodic_send_status(CopilotTest *test)
{
  static uint ac_id = 1; // use AC_ID and include airframe.h
  static uint mem = 30;
  static uint disk = 60;
  static uint err = 0;

  while(true){
    test->bus->SendMsg("%s %s %u %f %u %u %u %u",
            test->name_,
            test->copilot_status_.at(test->debug_).c_str(),
            ac_id, test->sec_since_startup_, mem, disk, test->status_, err);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // increment variables
    test->sec_since_startup_ += 2.0;
  }
}




void showhelpinfo(char *s) {
  cout<<"Usage:   "<<s<<" [-option] [argument]"<<endl;
  cout<<"option:  "<<"-h  show help information"<<endl;
  cout<<"         "<<"-b ivy bus (default is 127.255.255.255:2010)"<<endl;
  cout<<"         "<<"-d simulation mode (default is false, use 'true' or '1')"<<endl;
  cout<<"example: "<<s<<" -b 10.0.0.255:2010"<<endl;
}

int main(int argc, char** argv) {
  char tmp;
  char* ivy_bus = NULL;
  bool debug = false;

  while((tmp=getopt(argc,argv,"hb:d:"))!=-1)
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
      /*do nothing on default*/
      default:
        break;
    }
  }

  CopilotTest test(ivy_bus, debug);


  //Launch a thread
  std::thread t1(CopilotTest::ivy_thread, &test);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::thread t2(CopilotTest::periodic_send_status, &test);


  //Wait for the ivy_thread to end
  t1.join();

  return 0;
}
