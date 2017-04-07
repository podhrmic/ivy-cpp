# ivy-cpp
Ivy Bus implementation in C++11, original code cloned from https://svn.tls.cena.fr/svn/ivy/ivy-c++/trunk

## HOWTO
some paquage are necessary to compile all the stuff :
on ubuntu 14.10 :

apt-get install libqt4-dev-bin libglfw-dev libqt4-dev
 


this is the c++ api for ivy.

It's a wrapper over ivy-c, so ivy-c should be installed prior this.

It is api compatible with the native ivy c++ implementation for windows.

make all produce :

° libIvy.so
	a shared lib whith his own mainloop for console app.

° libIvy_Qt.so
	a shared lib which integrate in the Qt framework.


° libIvy_Xt.so
	a shared lib which integrate in the X toolkit mainloop

° libIvy_glfw.so
	a shared lib which integrate with glfw framework, in this case, there is a local
	ivy mainloop in a thread, and another gl thread which does the drawing, beware that
	you cannot directly call openGl within callback called by the ivy thread !!

° testIvyQt 
	a test which demonstrate how to use Ivy and Qt

° testIvyXt 
	a test which demonstrate how to use Ivy and Xt


