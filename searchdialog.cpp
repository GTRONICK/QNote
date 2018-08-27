#include "searchdialog.h"
#include "ui_searchdialog.h"
#include <QPlainTextEdit>
#include <QDebug>

/**
  Object's constructor.
*/
SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);
    gbReplaceClicked = false;
    gbSearchClicked = false;
    gbReplaceAllClicked = false;
}

void SearchDialog::focusOnSearchInputText()
{
    this->ui->seachDialog_searchLineEdit->setFocus();
}

/**
  Object's destructor.
*/
SearchDialog::~SearchDialog()
{
    delete ui;
}

/**
  Action triggered when the search button is clicked.
  This will rise a flag, which will be used in the
  setTextEdit method.
*/
void SearchDialog::on_searchDialog_searchButton_clicked()
{
    gbSearchClicked = true;
    emit search_signal_disableHighLight();
    emit search_signal_getTextEditText();
}

/**
  Action triggered when the replace button is clicked.
  This will rise a flag, which will be used in the
  setTextEdit method.
*/
void SearchDialog::on_searchDialog_replaceButton_clicked()
{
    gbReplaceClicked = true;
    emit(search_signal_getTextEditText());
}

/**
  Action triggered when the replace all button is clicked.
  This will rise a flag, which will be used in the
  setTextEdit method.
*/
void SearchDialog::on_searchDialog_replaceAllButton_clicked()
{
    gbReplaceAllClicked = true;
    emit(search_signal_getTextEditText());
}

/**
  Action triggered when the swap button is clicked.
  This will swap the text into the "search" line edit, with the
  text into the "reaplace all" line edit.
*/
void SearchDialog::on_gobSwapTextButton_clicked()
{
    QString lsReplace;
    lsReplace = this->ui->seachDialog_searchLineEdit->text();
    this->ui->seachDialog_searchLineEdit->setText(this->ui->searchDialog_replaceLineEdit->text());
    this->ui->searchDialog_replaceLineEdit->setText(lsReplace);
}

/**
  Creates a copy of the TextEdit object, and dependign on
  the clicked button, the according action modifications
  will be made. Then, the new object will be send in a signal
  to the main UI.
*/
void SearchDialog::search_slot_setTextEdit(QPlainTextEdit *textEdit)
{
    this->gobTextEdit = textEdit;

    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;

    if(gbReplaceAllClicked){
        gbReplaceAllClicked = false;
        gobTextEdit->extraSelections().clear();
        gobTextEdit->textCursor().clearSelection();


        emit(search_signal_resetCursor());
        gsFoundText = ui->seachDialog_searchLineEdit->text();
        while(gobTextEdit->find(ui->seachDialog_searchLineEdit->text())){
            selection.cursor = gobTextEdit->textCursor();
            extraSelections.append(selection);
        }
        gobTextEdit->setExtraSelections(extraSelections);

        QTextCursor cursor;

        for(int i=0; i < extraSelections.length(); i++){

            cursor = extraSelections.at(i).cursor;
            cursor.insertText(ui->searchDialog_replaceLineEdit->text());
        }

        gsFoundText = "";
        gobTextEdit->extraSelections().clear();
        extraSelections.clear();        

    }else if(gbSearchClicked){
        gbSearchClicked = false;
        QColor lineColor = QColor(Qt::yellow).lighter(100);
        selection.format.setBackground(lineColor);
        selection.format.setForeground(QColor(0,0,0));
        gobTextEdit->extraSelections().clear();

        if(gsFoundText != ui->seachDialog_searchLineEdit->text()){
            giLogCursorPos = 0;
            giOcurrencesFound = 0;
            emit(search_signal_resetCursor());
            gsFoundText = ui->seachDialog_searchLineEdit->text();
            while(gobTextEdit->find(ui->seachDialog_searchLineEdit->text())){
                giOcurrencesFound ++;
                selection.cursor = gobTextEdit->textCursor();
                extraSelections.append(selection);
            }
            extraSelections.append(selection);
            gobTextEdit->setExtraSelections(extraSelections);
            emit(search_signal_resetCursor());
            if(gobTextEdit->find(ui->seachDialog_searchLineEdit->text())){
                ui->searchDialog_replaceButton->setEnabled(true);
                giLogCursorPos = 1;
            }
            ui->ocurrencesCounterLabel->setText(QString("%1/%2").arg(giLogCursorPos).arg(giOcurrencesFound));
        }else{
            if(!gobTextEdit->find(ui->seachDialog_searchLineEdit->text())){
                emit(search_signal_resetCursor());
                giLogCursorPos = 0;
                if(gobTextEdit->find(ui->seachDialog_searchLineEdit->text())){
                    giLogCursorPos ++;
                }
            }else{
                ui->searchDialog_replaceButton->setEnabled(true);
                giLogCursorPos ++;
            }
            ui->ocurrencesCounterLabel->setText(QString("%1/%2").arg(giLogCursorPos).arg(giOcurrencesFound));
        }

    }else if(gbReplaceClicked){

        gbReplaceClicked = false;

        if(gsFoundText == ui->seachDialog_searchLineEdit->text()){

            gobTextEdit->textCursor().insertText(ui->searchDialog_replaceLineEdit->text());
        }

        ui->searchDialog_replaceButton->setEnabled(false);
    }
    emit search_signal_enableHighLight();
}


