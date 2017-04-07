#include <iostream>
#include <thread>
#include <chrono>

#include "Ivycpp.h"
#include "IvyApplication.h"

class IvyTest : public IvyApplicationCallback, public IvyMessageCallback {
public:
  Ivy *bus;
  void Start();
  void OnApplicationConnected(IvyApplication *app);
  void OnApplicationDisconnected(IvyApplication *app);
  void OnApplicationCongestion(IvyApplication *app);
  void OnApplicationDecongestion(IvyApplication *app);
  void OnApplicationFifoFull(IvyApplication *app);
  void OnMessage(IvyApplication *app, int argc, const char **argv);

  static void OnWORLD_ENV ( IvyApplication *app, void *user_data, int argc, const char **argv );
  IvyTest();

private:
   static void ivyAppConnCb( IvyApplication *app ) {};
   static void ivyAppDiscConnCb( IvyApplication *app ) {};
};

IvyTest::IvyTest() {
  bus = new Ivy( "TestIvy", "TestIvy READY",
           BUS_APPLICATION_CALLBACK(  ivyAppConnCb, ivyAppDiscConnCb ),false);
}

void IvyTest::Start()
{
  bus->BindMsg( "(.*)", this );
  bus->BindMsg("^(\\S*) WORLD_ENV (\\S*) (\\S*) (\\S*) (\\S*) (\\S*) (\\S*)",
        new IvyMessageCallbackFunction(OnWORLD_ENV,NULL));
  bus->start(NULL);
  bus->ivyMainLoop();
}

// this is a callback for received messages - we ll use it for logging
void IvyTest :: OnMessage(IvyApplication *app, int argc, const char **argv)
{
  int i;
  //printf ("%s sent ",app->GetName());
  for  (i = 0; i < argc; i++)
      printf(" '%s'",argv[i]);
  printf("\n");
}

// this message processes time stamp (GPS message
//
void IvyTest::OnWORLD_ENV ( IvyApplication *app, void *user_data, int argc, const char **argv )
{
  printf ("XXXXXXXXXXXXXXX\n");

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





IvyTest test;

void ivy_thread()
{
  test.Start();
}

// here we send status messages on the ivy bus
void msg_thread()
{
  static int id = 1;
  static float time = 0;
  static uint mem = 30;
  static uint disk = 60;
  static uint door = 1;
  static uint err = 0;

  while(true){
    test.bus->SendMsg("%d COPILOT_STATUS %f %u %u %u %u",
                         id, time, mem, disk, door, err);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      time += 1.0;
  }

}

int main() {
  //test.Start();

  //Launch a thread
  std::thread t1(ivy_thread);
  std::thread t2(msg_thread);

  //Join the thread with the main thread
  t1.join();
  //t1.detach();

  return 0;
}
