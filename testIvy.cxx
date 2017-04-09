#include <iostream>
#include <thread>
#include <chrono>

#include "Ivycpp.h"
#include "IvyApplication.h"

class IvyTest : public IvyApplicationCallback, public IvyMessageCallback {
public:
  Ivy *bus;
  int counter = 0;
  void Start();
  void OnApplicationConnected(IvyApplication *app);
  void OnApplicationDisconnected(IvyApplication *app);
  void OnApplicationCongestion(IvyApplication *app);
  void OnApplicationDecongestion(IvyApplication *app);
  void OnApplicationFifoFull(IvyApplication *app);
  void OnMessage(IvyApplication *app, int argc, const char **argv);

  IvyMessageCallbackFunction world_env_cb;

  static void OnWORLD_ENV ( IvyApplication *app, void *user_data, int argc, const char **argv);

  static void ivy_thread(IvyTest *test);
  static void message_thread(IvyTest *test);

  IvyTest();
private:
  static void ivyAppConnCb( IvyApplication *app ) {};
  static void ivyAppDiscConnCb( IvyApplication *app ) {};
};

IvyTest::IvyTest() : world_env_cb(OnWORLD_ENV,this) {
  bus = new Ivy( "TestIvy", "TestIvy READY",
      BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}

void IvyTest::Start()
{
  bus->BindMsg( "(.*)", this );
  bus->BindMsg("^(\\S*) WORLD_ENV (\\S*) (\\S*) (\\S*) (\\S*) (\\S*) (\\S*)", &world_env_cb);
  bus->start(NULL);
  bus->ivyMainLoop();
}

/**
 * this is a callback for received messages
 * right now it binds with all messages and displays them on terminal
 * (sort of like ivyprobe)
 */
void IvyTest :: OnMessage(IvyApplication *app, int argc, const char **argv)
{
  int i;
  printf ("%s sent ",app->GetName());
  for  (i = 0; i < argc; i++)
    printf(" '%s'",argv[i]);
  printf("\n");
}

/**
 * This is an example of a callback for a particular (WORLD_ENV) message
 * Shows how to pass the object to a static message so we can operate with
 * the received data
 */
void IvyTest::OnWORLD_ENV ( IvyApplication *app, void *user_data, int argc, const char **argv )
{
  static IvyTest ivy_test = (*(IvyTest *)user_data);
  ivy_test.counter++;
  printf ("Got WORLD_ENV message!\n");
  printf("counter=%u\n",ivy_test.counter);
}


void IvyTest :: OnApplicationConnected (IvyApplication *app)
{
  const char *appname;
  const char *host;
  appname = app->GetName();
  host = app->GetHost();

  printf("%s connected from %s\n", appname,  host);

}

void IvyTest :: OnApplicationDisconnected (IvyApplication *app)
{
  const char *appname;
  const char *host;
  appname = app->GetName ();
  host = app->GetHost();

  printf("%s disconnected from %s\n", appname,  host);
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




/*
 * Here starts the multithreaded example
 */

/**
 * This thread starts the Ivy Bus and the enters the IvyMainLoop
 */
void IvyTest::ivy_thread(IvyTest *test)
{
  test->Start();
}

/**
 * This thread sends a status message every second
 */
void IvyTest::message_thread(IvyTest *test)
{
  static int id = 1;
  static float time = 0;
  static uint mem = 30;
  static uint disk = 60;
  static uint door = 1;
  static uint err = 0;

  while(true){
    test->bus->SendMsg("%d COPILOT_STATUS %f %u %u %u %u",
        id, time, mem, disk, door, err);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    time += 1.0;
  }

}

int main() {
  IvyTest test;

  //Launch a thread
  std::thread t1(IvyTest::ivy_thread, &test);
  std::thread t2(IvyTest::message_thread, &test);

  //Wait for the ivy_thread to end
  t1.join();

  return 0;
}
