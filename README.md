# ivy-cpp
Ivy Bus implementation in C++11, original code cloned from https://svn.tls.cena.fr/svn/ivy/ivy-c++/trunk

**NOTE** This implementation assumes there is only one application running the *ivyMainLoop* - if that is not the case, proceed at your own risk.

this is the c++ api for ivy.

It's a wrapper over ivy-c, so ivy-c should be installed prior this.

It is api compatible with the native ivy c++ implementation for windows. This particuar version hasn't been tested on Windows, if you want to use Ivy-C++ on Windows, you should download the Windows branch from https://svn.tls.cena.fr/svn/ivy/ivy-c++/branches/windows/

make all produce :

- libIvy.so
	a shared lib whith his own mainloop for console app.

- testIvy 
	a test which demonstrate how to use Ivy and C++11
