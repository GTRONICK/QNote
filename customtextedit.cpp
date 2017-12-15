#include "customtextedit.h"
#include <QMimeData>
#include <QDebug>

CustomTextEdit::CustomTextEdit()
{
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    giFlag = 1;
}

void CustomTextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void CustomTextEdit::dropEvent(QDropEvent *event)
{
    emit this->customTextEdit_signal_processDropEvent(event);
}

void CustomTextEdit::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void CustomTextEdit::highlightCurrentLine()
{
    if(giFlag == 1){
        QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();

        if(extraSelections.count() > 0){
            extraSelections.removeLast();
        }

        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(this->palette().background().color().darker(110));

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
        setExtraSelections(extraSelections);
    }
}

void CustomTextEdit::cte_slot_disableHighLight()
{
    giFlag = 0;
}

void CustomTextEdit::cte_slot_enableHighLight()
{
    giFlag = 1;
}
