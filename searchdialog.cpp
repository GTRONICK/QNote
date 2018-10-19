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
    giSearchCondition = 0;
    giSearchFlag = 0;
}

void SearchDialog::focusOnSearchInputText()
{
    this->ui->seachDialog_searchLineEdit->setFocus();
}

void SearchDialog::setSearchText(QString asText)
{
    this->ui->seachDialog_searchLineEdit->setText(asText);
}

int SearchDialog::adjustExtraSelections(QPlainTextEdit *aobTextEdit, QString asTextToSearch)
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::yellow).lighter(100);
    selection.format.setBackground(lineColor);
    selection.format.setForeground(QColor(0,0,0));
    aobTextEdit->extraSelections().clear();

    while(aobTextEdit->find(asTextToSearch,giSearchCondition)){
        selection.cursor = aobTextEdit->textCursor();
        extraSelections.append(selection);
    }
    extraSelections.append(selection);
    aobTextEdit->setExtraSelections(extraSelections);
    return extraSelections.size() > 0 ? extraSelections.size() -1 : 0;
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

void SearchDialog::resetSearch()
{
    giCurrentOcurrence = 0;
    giOcurrencesFound = 0;
    giOcurrencesFound = this->adjustExtraSelections(gobTextEdit,ui->seachDialog_searchLineEdit->text());
    emit(search_signal_resetCursor());
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

    if(gbReplaceAllClicked){
        gbReplaceAllClicked = false;
        gobTextEdit->extraSelections().clear();
        gobTextEdit->textCursor().clearSelection();


        emit(search_signal_resetCursor());
        this->adjustExtraSelections(gobTextEdit,ui->seachDialog_searchLineEdit->text());
        gobTextEdit->extraSelections().removeLast();

        QTextCursor cursor;

        for(int i=0; i < gobTextEdit->extraSelections().length() - 1; i++){
            cursor = gobTextEdit->extraSelections().at(i).cursor;
            cursor.insertText(ui->searchDialog_replaceLineEdit->text());
        }

        gobTextEdit->extraSelections().clear();
        giSearchFlag = 0;

    }else if(gbSearchClicked){
        gbSearchClicked = false;

        if(giSearchFlag == 0) {
            emit(search_signal_resetCursor());
            this->resetSearch();
            giSearchFlag = 1;
        }

        if(gobTextEdit->find(ui->seachDialog_searchLineEdit->text(),giSearchCondition)){
            giCurrentOcurrence ++;
        } else {
            emit(search_signal_resetCursor());
            giCurrentOcurrence = 0;
            if(gobTextEdit->find(ui->seachDialog_searchLineEdit->text(),giSearchCondition)) {
                giCurrentOcurrence ++;
            } else {
                giSearchFlag = 0;
                this->resetSearch();
            }
        }

        ui->ocurrencesCounterLabel->setText(QString("%1/%2").arg(giCurrentOcurrence).arg(giOcurrencesFound));

    }else if(gbReplaceClicked){
        gbReplaceClicked = false;
        if(gobTextEdit->textCursor().selectedText() == ui->seachDialog_searchLineEdit->text()){
            gobTextEdit->textCursor().insertText(ui->searchDialog_replaceLineEdit->text());
        }
    }
    emit search_signal_enableHighLight();
}



void SearchDialog::on_caseSentive_checkBox_stateChanged(int arg1)
{
    if(arg1 == 2) {
        giSearchCondition = giSearchCondition | QTextDocument::FindCaseSensitively;
    } else {
        giSearchCondition = giSearchCondition ^ QTextDocument::FindCaseSensitively;
    }
}

void SearchDialog::on_wholeWords_checkBox_stateChanged(int arg1)
{
    if(arg1 == 2) {
        giSearchCondition = giSearchCondition | QTextDocument::FindWholeWords;
    } else {
        giSearchCondition = giSearchCondition ^ QTextDocument::FindWholeWords;
    }
}

void SearchDialog::on_backward_checkBox_stateChanged(int arg1)
{
    if(arg1 == 2) {
        giSearchCondition = giSearchCondition | QTextDocument::FindBackward;
    } else {
        giSearchCondition = giSearchCondition ^ QTextDocument::FindBackward;
    }
}
