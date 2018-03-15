#include "worker.h"
#include <QTextStream>

Worker::Worker(QObject *parent) : QObject(parent){}

void Worker::worker_slot_loadFile(QFile *file)
{
    QString line = "";
    QString text = "";
    QTextStream in(file);
    while (!in.atEnd()) {
      line = in.readLine();
      text.append(line + "\n");
    }
    in.flush();
    file->close();

    text.remove(text.lastIndexOf("\n"),1);
    emit worker_signal_appendText(text);
}
