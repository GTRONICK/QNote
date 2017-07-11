#ifndef CUSTOMTEXTEDIT_H
#define CUSTOMTEXTEDIT_H

#include <QObject>
#include <QWidget>
#include <QPlainTextEdit>
#include <QDropEvent>
#include <QDragEnterEvent>

class CustomTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    CustomTextEdit();
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

public slots:
    void highlightCurrentLine();
    void cte_slot_disableHighLight();
    void cte_slot_enableHighLight();

signals:
    void customTextEdit_signal_processDropEvent(QDropEvent *event);

private:
    int giFlag;
};

#endif // CUSTOMTEXTEDIT_H
