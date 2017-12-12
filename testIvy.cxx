#include <iostream> // for io
#include <thread> // for threads
#include <chrono> // for thread sleep
#include <string> // for string handling
#include <unistd.h> // for getopt

#include "Ivycpp.h"
#include "IvyApplication.h"

class IvyTest: public IvyApplicationCallback, public IvyMessageCallback
{
public:
  Ivy *bus;
  const char *bus_domain;
  int counter = 0;
  void Start();
  void OnApplicationConnected(IvyApplication *app);
  void OnApplicationDisconnected(IvyApplication *app);
  void OnApplicationCongestion(IvyApplication *app);
  void OnApplicationDecongestion(IvyApplication *app);
  void OnApplicationFifoFull(IvyApplication *app);
  void OnMessage(IvyApplication *app, int argc, const char **argv);

  IvyMessageCallbackFunction world_env_cb;

  static void OnWORLD_ENV(IvyApplication *app, void *user_data, int argc,
      const char **argv);

  static void ivy_thread(IvyTest *test);
  static void message_thread(IvyTest *test);

  IvyTest(char* ivy_bus);
private:
  static void ivyAppConnCb(IvyApplication *app)
  {
  }
  ;
  static void ivyAppDiscConnCb(IvyApplication *app)
  {
  }
  ;
};

bool evil;

IvyTest::IvyTest(char* ivy_bus) :
    world_env_cb(OnWORLD_ENV, this)
{
  bus_domain = ivy_bus;
  bus = new Ivy("TestIvy", "TestIvy READY",
      BUS_APPLICATION_CALLBACK(ivyAppConnCb, ivyAppDiscConnCb), false);
}

void IvyTest::Start()
{
  //bus->BindMsg( "(.*)", this );
  //bus->BindMsg("^(\\S*) WORLD_ENV (\\S*) (\\S*) (\\S*) (\\S*) (\\S*) (\\S*)", &world_env_cb);
  bus->start(bus_domain);
  bus->ivyMainLoop();
}

/**
 * this is a callback for received messages
 * right now it binds with all messages and displays them on terminal
 * (sort of like ivyprobe)
 */
void IvyTest::OnMessage(IvyApplication *app, int argc, const char **argv)
{
  int i;
  printf("%s sent ", app->GetName());
  for (i = 0; i < argc; i++)
    printf(" '%s'", argv[i]);
  printf("\n");
}

/**
 * This is an example of a callback for a particular (WORLD_ENV) message
 * Shows how to pass the object to a static message so we can operate with
 * the received data
 */
void IvyTest::OnWORLD_ENV(IvyApplication *app, void *user_data, int argc,
    const char **argv)
{
  static IvyTest ivy_test = (*(IvyTest *) user_data);
  ivy_test.counter++;
  printf("Got WORLD_ENV message!\n");
  printf("counter=%u\n", ivy_test.counter);
}

void IvyTest::OnApplicationConnected(IvyApplication *app)
{
  const char *appname;
  const char *host;
  appname = app->GetName();
  host = app->GetHost();

  printf("%s connected from %s\n", appname, host);

}

void IvyTest::OnApplicationDisconnected(IvyApplication *app)
{
  const char *appname;
  const char *host;
  appname = app->GetName();
  host = app->GetHost();

  printf("%s disconnected from %s\n", appname, host);
}

void IvyTest::OnApplicationCongestion(IvyApplication *app)
{
  std::cerr << "Ivy Congestion notififation\n";
}
void IvyTest::OnApplicationDecongestion(IvyApplication *app)
{
  std::cerr << "Ivy Decongestion notififation\n";
}
void IvyTest::OnApplicationFifoFull(IvyApplication *app)
{
  std::cerr << "Ivy FIFO Full  notififation : MESSAGE WILL BE LOST\n";
}

/**
 * This thread starts the Ivy Bus and the enters the IvyMainLoop
 */
void IvyTest::ivy_thread(IvyTest *test)
{
  test->Start();
}

/**
 * This thread sends a status message every second
 *
 <message name="MISSION_SURVEY" id="28" link="forwarded">
 <field name="ac_id" type="uint8"/>
 <field name="insert" type="uint8" values="APPEND|PREPEND|REPLACE_CURRENT|REPLACE_ALL"/>
 <field name="survey_east_1" type="float" unit="m"/>
 <field name="survey_north_1" type="float" unit="m"/>
 <field name="survey_east_2" type="float" unit="m"/>
 <field name="survey_north_2" type="float" unit="m"/>
 <field name="survey_alt" type="float" unit="m">altitude above geoid (MSL)</field>
 <field name="duration" type="float" unit="s"/>
 <field name="index" type="uint8"/>
 </message>
 *
 */
void IvyTest::message_thread(IvyTest *test)
{
  /*
   // not currently supported
   static uint8_t ac_id = 22;
   static uint8_t insert = 3; // append
   static float survey_east_1  = 100;
   static float survey_north_1 = 100;
   static float survey_east_2 = 200;
   static float survey_north_2 = 200;
   static float survey_alt = 100;
   static float duration = 10;
   static uint8_t index = 66;

   while(true) {
   test->bus->SendMsg("%mission_planner MISSION_SURVEY %u %u %f %f %f %f %f %f %u",
   ac_id, insert, survey_east_1, survey_north_1, survey_east_2, survey_north_2, survey_alt, duration, index);
   std::this_thread::sleep_for(std::chrono::seconds(1));
   }
   */

  /*
   // WORKS
   // time doesn't matter, the path has to be finished
   static uint8_t id = 22;
   static uint8_t insert = 3;

   static float east_1 = 100;
   static float north_1 = 200;

   static float east_2 = 120;
   static float north_2 = 220;

   static float east_3 = 140;
   static float north_3 = 240;

   static float east_4 = 160;
   static float north_4 = 260;

   static float east_5 = -10;
   static float north_5 = -70;

   static float wp_alt = 300;
   static float duration = 10;
   static uint8_t nb = 5;
   static uint8_t index = 22;

   while(true){
   test->bus->SendMsg("%mission_planner MISSION_PATH %u %u %f %f %f %f %f %f %f %f %f %f %f %f %u %u",
   id, insert, east_1, north_1, east_2, north_2, east_3, north_3, east_4, north_4, east_5, north_5, wp_alt, duration, nb, index);
   std::this_thread::sleep_for(std::chrono::seconds(1));
   index++;
   }
   */

  /*
   // WORKS
   // exits upon flying the segment, time doesn't matter
   static uint8_t id = 22;
   static uint8_t insert = 3;
   static float east_1 = 100;
   static float north_1= 120;
   static float east_2 = -150;
   static float north_2= -150;
   static float wp_alt = 300;
   static float duration = 10;
   static uint8_t index = 10;

   while(true){
   test->bus->SendMsg("%mission_planner MISSION_SEGMENT %u %u %f %f %f %f %f %f %u",
   id, insert, east_1, north_1, east_2, north_2, wp_alt, duration, index);
   std::this_thread::sleep_for(std::chrono::seconds(1));
   index++;
   }
   */

  /*
   // WORKS
   // time specifies max time spent flying a circle
   static uint8_t id = 22;
   static uint8_t insert = 3;
   static float wp_east = 100;
   static float wp_north= 130;
   static float wp_alt = 300;
   static float radius = 100;
   static float duration = 10;
   static uint8_t index = 10;

   while(true){
   test->bus->SendMsg("%mission_planner MISSION_CIRCLE %u %u %f %f %f %f %f %u",
   id, insert, wp_east, wp_north, wp_alt, radius, duration, index);
   std::this_thread::sleep_for(std::chrono::seconds(1));
   index++;
   }
   */

  // WORKS
  // time is irrelevant, exits upon reaching a waypoint
  static uint8_t id = 21;
  static uint8_t insert = 3;  // replace
  static float wp_east = 50;
  static float wp_north = 0;
  static float wp_alt = 5;
  static float duration = 60;
  static uint8_t index = 1;

  if (evil) {
    std::cout << "Running in evil mode" << std::endl;
    index  = 20;
    while (true) {
      wp_east = wp_east + 50;
      index++;
      std::cout << "Sending a new evil WP: wp_east=" << wp_east
          << "; wp_north = " << wp_north << std::endl;
      test->bus->SendMsg(
          "%mission_planner MISSION_GOTO_WP %u %u %f %f %f %f %u", id, insert,
          wp_east, wp_north, wp_alt, duration, index);
      std::this_thread::sleep_for(std::chrono::seconds(10));
    }
  } else {
    std::cout << "Running in standard mode" << std::endl;
    while (true) {
      wp_east = wp_east * -1;
      wp_north = wp_north * -1;
      index++;
      std::cout << "Sending a new WP: wp_east=" << wp_east << "; wp_north = "
          << wp_north << std::endl;
      test->bus->SendMsg(
          "%mission_planner MISSION_GOTO_WP %u %u %f %f %f %f %u", id, insert,
          wp_east, wp_north, wp_alt, duration, index);
      std::this_thread::sleep_for(std::chrono::seconds(10));
    }
  }

}

/*
 * Here starts the multithreaded example
 */

using namespace std;

void showhelpinfo(char *s)
{
  cout << "Usage:   " << s << " [-option] [argument]" << endl;
  cout << "option:  " << "-h  show help information" << endl;
  cout << "evil link" << "-e  evil mode" << endl;
  cout << "         " << "-b ivy bus (default is 127.255.255.255:2010)" << endl;
  cout << "example: " << s << " -b 10.0.0.255:2010" << endl;
}

int main(int argc, char** argv)
{
  char tmp;
  char* ivy_bus = NULL;
  evil = false;

  while ((tmp = getopt(argc, argv, "hb:e")) != -1) {
    switch (tmp) {
      /*option h show the help information*/
      case 'h':
        showhelpinfo(argv[0]);
        exit(1);
        break;
      case 'b':
        ivy_bus = optarg;
        cout << "-b " << ivy_bus << endl;
        break;
      case 'e':
        evil = true;
        break;
        /*do nothing on default*/
      default:
        break;
    }
  }

  IvyTest test(ivy_bus);

  //Launch a thread
  std::thread t1(IvyTest::ivy_thread, &test);
  std::thread t2(IvyTest::message_thread, &test);

  //Wait for the ivy_thread to end
  t1.join();

  return 0;
}
