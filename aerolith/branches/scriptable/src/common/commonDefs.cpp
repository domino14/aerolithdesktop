#include "commonDefs.h"

extern const quint16 MAGIC_NUMBER;

void Utilities::getUniqueRandomNumbers(QList<quint32>&numbers, quint32 start, quint32 end, int numNums)
{
    // takes all the numbers between start and end, including start and end,
    // randomly shuffles, and returns the first numNums numbers of the shuffled array.


//    int size = end - start + 1;
//    if (size < 1) size = start - end + 1;
//    if (numNums > size) return;
//
//    QList <quint32> pool;
//    //   pool.resize(size);
//    for (int i = 0; i < size; i++)
//    {
//        pool << i + start;
//    }
//    int choose, temp;
//    for (int i = 0; i < numNums; i++)
//    {
//        choose = qrand() % size;
//        numbers << pool[choose];
//        size--;
//        temp = pool[choose];
//        pool[choose] = pool[size];
//        pool[size] = temp;
//    }

    int size = end - start + 1;
    if (size < 1) size = start - end + 1;
    if (numNums > size) return;

    QList <quint32> pool;
    for (int i = 0; i < size; i++)
        pool << i + start;

    shuffle(pool);

    for (int i = 0; i < numNums; i++)
        numbers << pool[i];

}

void Utilities::shuffle(QList<quint32>& toShuffle)
{
    for (quint32 i = toShuffle.length(); i > 1; i--)
    {
        // Pick a random element to swap with the i-th element.
        quint32 j = qrand() % i; // 0 <= j <= i-1 (0-based array)
        // Swap array elements.
        quint32 tmp = toShuffle[j];
        toShuffle[j] = toShuffle[i-1];
        toShuffle[i-1] = tmp;
    }
}

QString Utilities::getRootDir()
{
    // taken from Michael Thelen's Zyzzyva GPLed code, available at http://www.zyzzyva.net
    static QString rootDir;

    if (!rootDir.isEmpty())
        return rootDir;

    rootDir = qApp->applicationDirPath();
    QDir dir (rootDir);

    // Search in the application dir path first, then up directories until a
    // directory is found that contains an aerolith.top file.
    while (true) {
        if (dir.exists("aerolith.top")) {
            rootDir = dir.absolutePath();
            return rootDir;
        }
        if (!dir.cdUp())
            break;
    }

    return rootDir;


}

QString Utilities::hexPrintable(const QByteArray x)
{
    QString outStr;
    QTextStream out(&outStr);
    for (unsigned int i = 0; i < x.size(); i++)
    {
        out << QString::number((quint8)x.at(i), 16).toUpper().rightJustified(2, '0', true) + " ";
    }
    return outStr;
}


/////////////////////

PacketBuilder::PacketBuilder(QObject* parent) : QObject(parent), o(&packet, QIODevice::WriteOnly)
{
    o.setVersion(QDataStream::Qt_4_2);
}

void PacketBuilder::processRawPacketForSending(QByteArray rawPacket)
{
    o.device()->seek(0);
    packet.clear();
    o << MAGIC_NUMBER;
    o << (quint16)rawPacket.length();
    o.writeRawData(rawPacket.constData(), rawPacket.length());
}

void PacketBuilder::processForSending()
{
    // possibly add encryption to packet at some point?
    QByteArray tempPacket = packet;
    o.device()->seek(0);
    packet.clear();
    o << MAGIC_NUMBER;
    o << (quint16)tempPacket.length();
    o.writeRawData(tempPacket.constData(), tempPacket.length());
}

void PacketBuilder::resetPacket()
{
    o.device()->seek(0);
    packet.clear();
}

QByteArray PacketBuilder::getPacket()
{
    return packet;
}
