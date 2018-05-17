#include "worker.h"
#include <QTextStream>
#include <QDebug>

Worker::Worker(QObject *parent) : QObject(parent){
    giFileSize = 0;
}

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

void Worker::worker_slot_tailFile(QFile *file)
{
    if(file->size() > giFileSize) {
        int liDiff = file->size() - giFileSize;
        if(file->open(QIODevice::ReadOnly | QIODevice::Text)) {
            if(file->seek(giFileSize)) {
                QString lsText = QString(file->read(liDiff));
                emit worker_signal_insertText(lsText);
                giFileSize = file->size();
            }
            file->close();
        }
    } else {
        giFileSize = file->size();
    }
}

void Worker::worker_slot_setCurrentFileSize(int aiFileSize)
{
    this->giFileSize = aiFileSize;
}

