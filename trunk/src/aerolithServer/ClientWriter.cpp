#include "ClientWriter.h"

const quint16 MAGIC_NUMBER = 25348;
QByteArray block;
QDataStream out(&block, QIODevice::WriteOnly);

void initializeWriter()
{

  out.setVersion(QDataStream::Qt_4_2);
}

void writeHeaderData()
{
  out.device()->seek(0);
  block.clear();
  out << (quint16)MAGIC_NUMBER;
  out << (quint16)0;
}

void fixHeaderLength()
{
  out.device()->seek(sizeof(quint16));
  out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16));
}
