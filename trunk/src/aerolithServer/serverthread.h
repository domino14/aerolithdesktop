#ifndef _SERVERTHREAD_H_
#define _SERVERTHREAD_H_

#include <QThread>
#include "mainserver.h"

class ServerThread : public QThread
{
Q_OBJECT
 public:
  ServerThread(QString version);

 protected:
  void run();
  
 private:
  MainServer* mainServer;
  volatile bool shouldQuitThread;
  QString version;
  public slots:
  void stopThread();
  void startThread();
};

#endif
