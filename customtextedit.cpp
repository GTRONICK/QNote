#include "customtextedit.h"
#include <QMimeData>
#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QRegularExpression>
#include <QTextBlock>

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

void CustomTextEdit::keyPressEvent(QKeyEvent *e){

    QTextCursor cursor = this->textCursor();

    if(e->key() == Qt::Key_Tab && cursor.hasSelection()){

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(end, QTextCursor::KeepAnchor);
        QTextBlock endBlock = cursor.block();

        cursor.setPosition(start, QTextCursor::KeepAnchor);
        QTextBlock block = cursor.block();

        for(; block.isValid() && !(endBlock < block); block = block.next())
        {
            if (!block.isValid())
                continue;

            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.clearSelection();
            cursor.insertText("\t");
            cursor.movePosition(QTextCursor::NextBlock);
        }

    } else if(e->key() == Qt::Key_Backtab){

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(end, QTextCursor::KeepAnchor);
        QTextBlock endBlock = cursor.block();

        cursor.setPosition(start, QTextCursor::KeepAnchor);
        QTextBlock block = cursor.block();

        for(; block.isValid() && !(endBlock < block); block = block.next())
        {
            if (!block.isValid())
                continue;

            cursor.setPosition(block.position() + 1, QTextCursor::KeepAnchor);
            cursor.clearSelection();
            if(block.text().startsWith("\t") || block.text().startsWith(" ")) cursor.deletePreviousChar();
        }

    } else {
        QPlainTextEdit::keyPressEvent(e);
    }
}
