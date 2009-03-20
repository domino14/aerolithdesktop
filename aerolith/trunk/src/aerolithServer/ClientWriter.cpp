//    Copyright 2007, 2008, 2009, Cesar Del Solar  <delsolar@gmail.com>
//    This file is part of Aerolith.
//
//    Aerolith is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Aerolith is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Aerolith.  If not, see <http://www.gnu.org/licenses/>.

#include "ClientWriter.h"

extern const quint16 MAGIC_NUMBER = 25349;
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
