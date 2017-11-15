#include "customtabwidget.h"
#include "customtextedit.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include <QIODevice>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QPlainTextEdit>
#include <QFontMetrics>
#include <QApplication>
#include <QFontDatabase>
#include <QFontDialog>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QMimeData>
#include <QShortcut>
#include <QDialog>
#include <QDebug>
#include <QEvent>
#include <QTimer>
#include <QMap>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    giRecentFilePos = 0;
    giCurrentTabIndex = 0;
    giCurrentFileIndex = 0;
    giTotalTabs = 0;
    giRecentAux = 0;
    gobFileNames.clear();
    giTabCharacters = 4;
    giTimerDelay = 250;
    gsThemeFile = "Default";
    gbIsOpenedFile = false;
    gbSaveCancelled = false;
    gbIsReloadFile = false;
    gbIsAutoreloadEnabled = false;
    gobSearchDialog = new SearchDialog(this);
    worker = new Worker;
    workerThread = new QThread;
    setAcceptDrops(true);
    loadConfig();
    gobTimer = new QTimer(this);
    gobMovie->setScaledSize(QSize(15,15));
    gsDefaultDir = QDir::homePath();
    giDefaultDirCounter = 0;


    QShortcut *menuBar_shortcut = new QShortcut(QKeySequence(tr("Ctrl+M")),this);

    QShortcut *gr1_shortcut = new QShortcut(QKeySequence(tr("Ctrl+1")),this);
    QShortcut *gr2_shortcut = new QShortcut(QKeySequence(tr("Ctrl+2")),this);
    QShortcut *gr3_shortcut = new QShortcut(QKeySequence(tr("Ctrl+3")),this);
    QShortcut *gr4_shortcut = new QShortcut(QKeySequence(tr("Ctrl+4")),this);
    QShortcut *gr5_shortcut = new QShortcut(QKeySequence(tr("Ctrl+5")),this);

    QShortcut *paste_gr1_shortcut = new QShortcut(QKeySequence(tr("Ctrl+Shift+1")),this);
    QShortcut *paste_gr2_shortcut = new QShortcut(QKeySequence(tr("Ctrl+Shift+2")),this);
    QShortcut *paste_gr3_shortcut = new QShortcut(QKeySequence(tr("Ctrl+Shift+3")),this);
    QShortcut *paste_gr4_shortcut = new QShortcut(QKeySequence(tr("Ctrl+Shift+4")),this);
    QShortcut *paste_gr5_shortcut = new QShortcut(QKeySequence(tr("Ctrl+Shift+5")),this);

    this->move(QApplication::desktop()->screen()->rect().center() - this->rect().center());

    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(main_slot_tabChanged(int)));
    connect(ui->tabWidget,SIGNAL(ctw_signal_tabMoved(int,int)),this,SLOT(main_slot_tabMoved(int,int)));
    connect(ui->menuOpen_Recent, SIGNAL(triggered(QAction*)), this, SLOT(main_slot_loadFileFromAction(QAction*)));
    connect(this,SIGNAL(main_signal_setTextEdit(QPlainTextEdit*)),gobSearchDialog,SLOT(search_slot_setTextEdit(QPlainTextEdit*)));
    connect(gobSearchDialog,SIGNAL(search_signal_getTextEditText()),this,SLOT(main_slot_getTextEditText()));
    connect(gobSearchDialog,SIGNAL(search_signal_resetCursor()),this,SLOT(main_slot_resetCursor()));
    connect(this,SIGNAL(main_signal_loadFile(QFile*)),worker,SLOT(worker_slot_loadFile(QFile*)));
    connect(worker,SIGNAL(worker_signal_appendText(QString)),this,SLOT(main_slot_appendText(QString)));
    connect(gobTimer, SIGNAL(timeout()), this, SLOT(on_actionReload_file_triggered()));
    connect(menuBar_shortcut,SIGNAL(activated()),this,SLOT(main_slot_showHideMenuBar()));
    connect(gr1_shortcut,SIGNAL(activated()),this,SLOT(main_slot_gr1()));
    connect(gr2_shortcut,SIGNAL(activated()),this,SLOT(main_slot_gr2()));
    connect(gr3_shortcut,SIGNAL(activated()),this,SLOT(main_slot_gr3()));
    connect(gr4_shortcut,SIGNAL(activated()),this,SLOT(main_slot_gr4()));
    connect(gr5_shortcut,SIGNAL(activated()),this,SLOT(main_slot_gr5()));
    connect(paste_gr1_shortcut,SIGNAL(activated()),this,SLOT(main_slot_pasteGr1()));
    connect(paste_gr2_shortcut,SIGNAL(activated()),this,SLOT(main_slot_pasteGr2()));
    connect(paste_gr3_shortcut,SIGNAL(activated()),this,SLOT(main_slot_pasteGr3()));
    connect(paste_gr4_shortcut,SIGNAL(activated()),this,SLOT(main_slot_pasteGr4()));
    connect(paste_gr5_shortcut,SIGNAL(activated()),this,SLOT(main_slot_pasteGr5()));
    connect(workerThread,SIGNAL(finished()),worker,SLOT(deleteLater()));
    connect(workerThread,SIGNAL(finished()),workerThread,SLOT(deleteLater()));
    worker->moveToThread(workerThread);
    workerThread->start();
    on_actionNew_Tab_triggered();    
}

MainWindow::~MainWindow()
{
    delete ui;
    workerThread->quit();
    workerThread->wait();
}

void MainWindow::on_actionOpen_triggered()
{
    giDefaultDirCounter ++;

    if(giDefaultDirCounter > 1) {
        gsDefaultDir = "";
        giDefaultDirCounter = 2;
    }

    giCurrentFileIndex = 0;

    gobFileNames = QFileDialog::getOpenFileNames(this
                                          ,"Open File"
                                          ,gsDefaultDir
                                          ,tr("All Files (*);;Text Files (*.txt);;Log files (*.log)"));

    if(!gobFileNames.isEmpty()){
        QString lsFileName = gobFileNames.at(giCurrentFileIndex);
        loadFile(lsFileName);
    }

    this->addRecentFiles();
}

bool MainWindow::on_actionSave_As_triggered()
{
    QString lsFileName;

    lsFileName = QFileDialog::getSaveFileName(this
                                              ,"Save File"
                                              ,""
                                              ,tr("Text Files (*.txt);;All Files (*)"));

    giCurrentTabIndex = ui->tabWidget->currentIndex();
    if(gobHash.value(giCurrentTabIndex) != lsFileName){
        gobHash.insert(giCurrentTabIndex,lsFileName);
    }

    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(ui->tabWidget->widget(giCurrentTabIndex));

    if(!saveFile(lsFileName,edit->toPlainText())) return false;

    ui->indicatorLabel->clear();
    gobIsModifiedTextHash.insert(giCurrentTabIndex,false);
    setCurrentTabNameFromFile(lsFileName);
    this->setStatusBarTextAsLink(lsFileName);

    if(!gobFileNames.contains(lsFileName)){
        gobFileNames.append(lsFileName);
        this->addRecentFiles();
    }

    return true;
}

bool MainWindow::on_actionSave_triggered()
{
    QString lsFileName;

    giCurrentTabIndex = ui->tabWidget->currentIndex();
    lsFileName = gobHash.value(giCurrentTabIndex);

    QFile file(lsFileName);

    if(file.exists()){

        QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(ui->tabWidget->widget(giCurrentTabIndex));
        if(!saveFile(lsFileName,edit->toPlainText())) return false;
        ui->indicatorLabel->clear();
        gobIsModifiedTextHash.insert(giCurrentTabIndex,false);
        setCurrentTabNameFromFile(lsFileName);
        if(!gobFileNames.contains(lsFileName)){
            gobFileNames.append(lsFileName);
            this->addRecentFiles();
        }

    }else{
        if(!on_actionSave_As_triggered()) return false;
    }


    return true;
}

bool MainWindow::saveFile(QString asFileName, QString asText)
{
    QFile file(asFileName);

    if(asFileName.isEmpty()) return false;

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::critical(this,"Error","File could not be opened");
        return false;
    }

    QTextStream out(&file);
    out << asText;
    file.close();



    return true;
}

bool MainWindow::saveConfig()
{

    QString configText = "[THEME]\n";
    configText = configText + "themeFile=" + gsThemeFile + "\n";
    configText = configText + "[FONT]" + "\n";
    configText = configText + "family=" + gobCurrentPlainTextEdit->fontInfo().family() + "\n";
    configText = configText + "size=" + QString::number(gobCurrentPlainTextEdit->fontInfo().style()) + "\n";
    configText = configText + "point=" + QString::number(gobCurrentPlainTextEdit->fontInfo().pointSize()) + "\n";
    configText = configText + "[RECENTS]" + "\n";
    configText = configText + "position=" + QString::number(giRecentFilePos) + "\n";
    configText = configText + "recentFiles=";

    for(QString lsFile:gobRecentFiles){
        configText = configText + "@@" + lsFile;
    }

    if(!saveFile("config.ini",configText)) return false;
    else return true;
}

bool MainWindow::loadConfig()
{
    QString line;
    QStringList lobValues;
    QFile *lobConfigFile = new QFile("config.ini");
    if(!lobConfigFile->open(QFile::ReadOnly)){

        gsSavedFont = "Arial";
        giSavedFontStyle = 0;
        giSavedFontPointSize = 10;

        return false;
    }

    QTextStream lobInputStream(lobConfigFile);
    while (!lobInputStream.atEnd()) {
        line = lobInputStream.readLine();
        if(line.startsWith("themeFile")) gsThemeFile = line.split("=").at(1);
        else if(line.startsWith("family")) gsSavedFont = line.split("=").at(1);
        else if(line.startsWith("size")) giSavedFontStyle = line.split("=").at(1).toInt();
        else if(line.startsWith("point")) giSavedFontPointSize = line.split("=").at(1).toInt();
        else if(line.startsWith("position")) giRecentFilePos = line.split("=").at(1).toInt();
        else if(line.startsWith("recentFiles")) {
            QStringList lobRecentList= line.split("@@");
            lobRecentList.removeAt(0);
            for(QString lsRecentFile:lobRecentList){
                gobRecentFiles.append(lsRecentFile);
            }
            this->addRecentFiles();
        }
    }

    if(gsSavedFont.isEmpty() || gsSavedFont == "") {
        gsSavedFont = "Arial";
        giSavedFontStyle = 0;
        giSavedFontPointSize = 10;
    }

    QFile style(gsThemeFile);

    if(style.exists() && style.open(QFile::ReadOnly)) {
        QString styleContents = QLatin1String(style.readAll());
        style.close();
        this->setStyleSheet(styleContents);
    }

    lobInputStream.flush();
    lobConfigFile->close();

    return true;
}

void MainWindow::on_actionAbout_QNote_triggered()
{
    QMessageBox::about(this,"QNote 1.6.0",
                       "<style>"
                       "a:link {"
                           "color: orange;"
                           "background-color: transparent;"
                           "text-decoration: none;"
                       "}"
                       "</style>"
                       "<p>Simple text editor"
                       "<p> Jaime A. Quiroga P."
                       "<p><a href='http://www.gtronick.com'>GTRONICK</a>");
}

void MainWindow::on_actionAbout_QT_triggered()
{
    QMessageBox::aboutQt(this,"About QT5.8");
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    checkIfUnsaved(index);
}

void MainWindow::on_actionClose_Tab_triggered()
{
    checkIfUnsaved(giCurrentTabIndex);
}

void MainWindow::on_actionNew_Tab_triggered()
{
    giTotalTabs ++;
    QApplication::processEvents();
    CustomTextEdit *lobPlainTexEdit = new CustomTextEdit();
    lobPlainTexEdit->setPlaceholderText("Type Here...");
    lobPlainTexEdit->setFrameShape(QFrame::NoFrame);
    lobPlainTexEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    lobPlainTexEdit->setAcceptDrops(true);
    int fontWidth = QFontMetrics(lobPlainTexEdit->currentCharFormat().font()).averageCharWidth();
    lobPlainTexEdit->setTabStopWidth( giTabCharacters * fontWidth );
    QPalette p = lobPlainTexEdit->palette();
    p.setColor(QPalette::Highlight, QColor(qRgb(200,0,0)));
    p.setColor(QPalette::HighlightedText, QColor(qRgb(255,255,255)));
    lobPlainTexEdit->setPalette(p);
    connect(lobPlainTexEdit,SIGNAL(textChanged()),this,SLOT(main_slot_textChanged()));
    connect(lobPlainTexEdit,SIGNAL(customTextEdit_signal_processDropEvent(QDropEvent*)),this,SLOT(main_slot_processDropEvent(QDropEvent*)));
    connect(lobPlainTexEdit,SIGNAL(cursorPositionChanged()),this,SLOT(main_slot_currentLineChanged()));
    connect(this,SIGNAL(main_signal_refreshHighlight()),lobPlainTexEdit,SLOT(highlightCurrentLine()));
    connect(gobSearchDialog,SIGNAL(search_signal_disableHighLight()),lobPlainTexEdit,SLOT(cte_slot_disableHighLight()));
    connect(gobSearchDialog,SIGNAL(search_signal_enableHighLight()),lobPlainTexEdit,SLOT(cte_slot_enableHighLight()));
    QFont serifFont(gsSavedFont, giSavedFontPointSize, giSavedFontStyle);
    lobPlainTexEdit->setFont(serifFont);
    giCurrentTabIndex = this->ui->tabWidget->addTab(lobPlainTexEdit,"New " + QString::number(giTotalTabs));
    this->ui->tabWidget->setCurrentIndex(giCurrentTabIndex);
    gobHash.insert(giCurrentTabIndex,"");
    gobIsModifiedTextHash.insert(giCurrentTabIndex,false);
}

void MainWindow::on_actionReload_file_triggered()
{
    QString lsFileName = gobHash.value(giCurrentTabIndex);

    if(checkFileExist(lsFileName)){

        gbIsReloadFile = true;

        if(!gbIsAutoreloadEnabled) checkIfUnsaved(giCurrentTabIndex);

        if(gbSaveCancelled == false){

            setCurrentTabNameFromFile(lsFileName);

            if(!gobFile->open(QIODevice::ReadOnly | QIODevice::Text)){
                QMessageBox::critical(this,"Error","File could not be opened");
                gbIsReloadFile = false;
            }
            emit main_signal_loadFile(gobFile);
        }else{
            gbSaveCancelled = false;
        }

    }else{
        gbIsReloadFile = false;
        if(gbIsAutoreloadEnabled){
            if(gobTimer->isActive()) gobTimer->stop();
            this->on_actionAuto_Reload_tail_f_toggled(false);
        }
    }
}

void MainWindow::on_actionGo_to_line_triggered()
{
    bool ok;
        int line_number = QInputDialog::getInt(this, tr("Go to"),
                                               tr("Go to line: "),
                                               1,
                                               1,
                                               gobCurrentPlainTextEdit->blockCount(),
                                               1,
                                               &ok);
    if(ok){
        int moves = line_number-(gobCurrentPlainTextEdit->textCursor().blockNumber() + 1);
        QTextCursor cursor = gobCurrentPlainTextEdit->textCursor();
        if(moves)
        {
            if(moves<0) cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, -moves);
            else cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, moves);

            cursor.movePosition(QTextCursor::StartOfLine);

            gobCurrentPlainTextEdit->setTextCursor(cursor);
        }
    }
}

void MainWindow::on_actionFind_Replace_triggered()
{
    gobSearchDialog->setVisible(true);
}

void MainWindow::main_slot_getTextEditText()
{
    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(ui->tabWidget->widget(giCurrentTabIndex));
    emit main_signal_setTextEdit(edit);
}

void MainWindow::main_slot_resetCursor()
{
    QPlainTextEdit* edit = qobject_cast<CustomTextEdit*>(ui->tabWidget->widget(giCurrentTabIndex));
    edit->moveCursor(QTextCursor::Start);
}

void MainWindow::main_slot_tabChanged(int aIndex)
{
    //qDebug() << "Begin main_slot_tabChanged, aIndex = " + QString::number(aIndex);
    giCurrentTabIndex = aIndex;
    this->setStatusBarTextAsLink(gobHash.value(aIndex));
    gobCurrentPlainTextEdit = qobject_cast<CustomTextEdit*>(ui->tabWidget->widget(giCurrentTabIndex));
    QFont serifFont(gsSavedFont, giSavedFontPointSize, giSavedFontStyle);
    gobCurrentPlainTextEdit->setFont(serifFont);
    if(gobIsModifiedTextHash.value(aIndex)){
        //qDebug() << "File modified";
        ui->indicatorLabel->setPixmap(QPixmap("://unsaved.png"));
    }else if(gbIsAutoreloadEnabled){
        ui->indicatorLabel->setToolTip("Auto Reload Active");
        this->ui->indicatorLabel->setMovie(gobMovie);
        gobTimer->start(giTimerDelay);
        ui->indicatorLabel->movie()->start();
    }else{
        if(!gbIsAutoreloadEnabled) ui->indicatorLabel->clear();
    }
    //qDebug() << "End main_slot_tabChanged";
}

void MainWindow::main_slot_tabMoved(int from, int to)
{
    QString lsTemporalValue = "";

    lsTemporalValue = gobHash.value(to);
    gobHash.insert(to,gobHash.value(from));
    gobHash.insert(from,lsTemporalValue);

    bool lbTemporalValue = gobIsModifiedTextHash.value(to);
    gobIsModifiedTextHash.insert(to,gobIsModifiedTextHash.value(from));
    gobIsModifiedTextHash.insert(from,lbTemporalValue);
}

void MainWindow::main_slot_textChanged()
{
    if(gbIsOpenedFile == false && gobIsModifiedTextHash.value(giCurrentTabIndex) == false && !gbIsAutoreloadEnabled){
        ui->indicatorLabel->setPixmap(QPixmap("://unsaved.png"));
        ui->indicatorLabel->setToolTip("Document unsaved");
        gobIsModifiedTextHash.insert(giCurrentTabIndex,true);
    }
}

void MainWindow::main_slot_appendText(QString asText)
{
    QString lsFileName;

    gbIsOpenedFile = true;
    gobCurrentPlainTextEdit->clear();
    gobCurrentPlainTextEdit->appendPlainText(asText);
    gobIsModifiedTextHash.insert(giCurrentTabIndex,false);
    gbIsOpenedFile = false;
    gobFile->close();

    if(gbIsReloadFile){
        gbIsReloadFile = false;
    }else{
        main_slot_resetCursor();
    }

    //Se carga el siguiente archivo seleccionado
    giCurrentFileIndex ++;
    if(giCurrentFileIndex < gobFileNames.length()){
        lsFileName = gobFileNames.at(giCurrentFileIndex);
        loadFile(lsFileName);
    }else{
        giCurrentFileIndex = 0;
        gobFileNames.clear();
    }
}

void MainWindow::main_slot_processDropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls()){
        gobFileNames.clear();
        giCurrentFileIndex = 0;
        QList<QUrl> urlList = mimeData->urls();

        for (int i = 0; i < urlList.size(); i++)
        {
            gobFileNames.append(urlList.at(i).toLocalFile());
        }
        if(!gobFileNames.isEmpty()){
            QString lsFileName = gobFileNames.at(giCurrentFileIndex);
            loadFile(lsFileName);
        }
        this->addRecentFiles();
        event->acceptProposedAction();
    }
}

void MainWindow::setCurrentTabNameFromFile(QString asFileName)
{
    QString extension = "";
    QString fileName = "";

    if(!QFileInfo(asFileName).completeSuffix().isEmpty()){
            extension = "."+QFileInfo(asFileName).completeSuffix();
        }

    fileName = QFileInfo(asFileName).baseName() + extension;

    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),fileName);
}

void MainWindow::checkIfUnsaved(int index)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Warning!");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);

    if(gobIsModifiedTextHash.value(index) == true){
        msgBox.setText(ui->tabWidget->tabText(index) + " has been modified.");
        int ret = msgBox.exec();

        switch (ret) {
          case QMessageBox::Save:
              if(!gbIsReloadFile && !gbIsAutoreloadEnabled && on_actionSave_triggered()) closeTab(index);
              else{
                  ui->indicatorLabel->clear();
                  on_actionSave_triggered();
              }
              break;
          case QMessageBox::Discard:
              if(!gbIsReloadFile && !gbIsAutoreloadEnabled) closeTab(index);
              else{
                  ui->indicatorLabel->clear();
              }
              break;
          case QMessageBox::Cancel:
              gbSaveCancelled = true;
              break;
          default:
              on_actionSave_triggered();
              if(!gbIsReloadFile && !gbIsAutoreloadEnabled) closeTab(index);
              break;
        }
    }else{
        if(!gbIsReloadFile && !gbIsAutoreloadEnabled) closeTab(index);
    }
}

bool MainWindow::checkFileExist(QString asFileName)
{
    gobFile = new QFile(asFileName);

    if(!asFileName.isEmpty() && gobFile->exists()){
        return true;
    }

    return false;
}

void MainWindow::closeTab(int index)
{
    QFile *lobFile;

    if(giTotalTabs > 1){
        lobFile = new QFile(gobHash.value(index));

        gobHash.remove(index);
        for(int i = index; i < giTotalTabs; i ++){
            gobHash.insert(i,gobHash.value(i + 1));
        }

        gobIsModifiedTextHash.remove(index);
        for(int i = index; i < giTotalTabs; i ++){
            gobIsModifiedTextHash.insert(i,gobIsModifiedTextHash.value(i + 1));
        }

        this->setStatusBarTextAsLink(gobHash.value(giCurrentTabIndex));
        if(lobFile->isOpen()){
            lobFile->flush();
            lobFile->~QFile();
        }

        giTotalTabs --;
        this->ui->tabWidget->removeTab(index);

    }else{
        if(!saveConfig()){
            QMessageBox::critical(this,"Warning!","The config file could not be saved");
        }
        QApplication::quit();
    }
}

void MainWindow::loadFile(QString asFileName)
{
    //qDebug() << "Begin loadFile, asFileName: " << asFileName;
    if(!asFileName.isEmpty()){
        gobFile = new QFile(asFileName);
        double liFileSize = gobFile->size()/MAX_SIZE;
        //Crea nuevo Tab
        if(gobFile->exists() && liFileSize < (MAX_SIZE/1000000.0)){
            on_actionNew_Tab_triggered();
            gobHash.insert(giCurrentTabIndex,asFileName);
            setCurrentTabNameFromFile(asFileName);
            main_slot_tabChanged(giCurrentTabIndex);

            if(!gobFile->open(QIODevice::ReadOnly | QIODevice::Text)){
                QMessageBox::critical(this,"Error","File could not be opened");
                return;
            }

            gobCurrentPlainTextEdit = qobject_cast<CustomTextEdit*>(ui->tabWidget->widget(giCurrentTabIndex));
            emit(main_signal_loadFile(gobFile));

        }else{
            QMessageBox::warning(this,"File not found","The file " + asFileName + " does not exist or is too heavy.");
            gobFileNames.removeAt(giCurrentFileIndex);
        }
    }
    //qDebug() << "End loadFile";
}

void MainWindow::setFileNameFromCommandLine(QStringList asFileNames)
{
    gobFileNames = asFileNames;
    this->on_actionOpen_triggered();
}

void MainWindow::setStatusBarTextAsLink(QString asText)
{
    ui->statusBar->setText("<a href=\"" + asText + "\">" + asText + "</a>");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool lbClose = true;

    for(int i = giTotalTabs - 1; i >= 0; i --){
        checkIfUnsaved(i);
        if(gbSaveCancelled){
            i = 0;
            gbSaveCancelled = false;
            lbClose = false;
        }
    }
    if(lbClose){
        event->accept();
    }else{
        event->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls()){

        QList<QUrl> urlList = mimeData->urls();

        for (int i = 0; i < urlList.size(); i++)
        {
            gobCurrentPlainTextEdit->appendPlainText(urlList.at(i).toLocalFile());
        }

        event->acceptProposedAction();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::on_actionWord_wrap_toggled(bool arg1)
{
    if(arg1){
        gobCurrentPlainTextEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    }else{
        gobCurrentPlainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    }
}

void MainWindow::on_actionLoad_theme_triggered()
{
    gsThemeFile = QFileDialog::getOpenFileName(this, tr("Open Style"),
                                                        "",
                                                        tr("Style sheets (*.qss);;All files(*.*)"));

    if(gsThemeFile != NULL && gsThemeFile != ""){
        QFile styleFile(gsThemeFile);
        styleFile.open(QFile::ReadOnly);
        QString StyleSheet = QLatin1String(styleFile.readAll());
        this->setStyleSheet(StyleSheet);
    }

    emit main_signal_refreshHighlight();
}

void MainWindow::on_actionSystem_theme_triggered()
{
    this->setStyleSheet("");
    emit main_signal_refreshHighlight();
}

void MainWindow::on_actionAuto_Reload_tail_f_toggled(bool arg1)
{
    QString lsFileName = gobHash.value(giCurrentTabIndex);
    gbIsAutoreloadEnabled = arg1;

    if(arg1 && checkFileExist(lsFileName)){
        checkIfUnsaved(giCurrentTabIndex);
        ui->indicatorLabel->setToolTip("Auto Reload Active");
        this->ui->indicatorLabel->setMovie(gobMovie);
        gobTimer->start(giTimerDelay);
        ui->indicatorLabel->movie()->start();
    }else{
        ui->indicatorLabel->setToolTip("");
        if(gobTimer->isActive()) gobTimer->stop();
        if(ui->indicatorLabel->movie() != NULL) ui->indicatorLabel->movie()->stop();
        ui->indicatorLabel->clear();
        if(!checkFileExist(lsFileName)){
            disconnect(ui->actionAuto_Reload_tail_f,SIGNAL(toggled(bool)),this,SLOT(on_actionAuto_Reload_tail_f_toggled(bool)));
            ui->actionAuto_Reload_tail_f->setChecked(false);
            connect(ui->actionAuto_Reload_tail_f,SIGNAL(toggled(bool)),this,SLOT(on_actionAuto_Reload_tail_f_toggled(bool)));
            QMessageBox::critical(this,"ERROR","The file can't be loaded");
        }
        gbIsAutoreloadEnabled = false;
        gbIsReloadFile = false;
    }
}

void MainWindow::on_actionAuto_Reload_delay_triggered()
{
    bool lbOk;
    int liDelay = QInputDialog::getInt(this, tr("Auto Reload delay (ms)"),
                                 tr("milliseconds:"), 250, 100, 5000, 1, &lbOk);
    if (lbOk) giTimerDelay = liDelay;
}

void MainWindow::main_slot_currentLineChanged()
{
    int liLine = gobCurrentPlainTextEdit->textCursor().blockNumber() + 1;
    int liCol = gobCurrentPlainTextEdit->textCursor().columnNumber();
    ui->lineNumberLabel->setText(QString("Line: %1, Col: %2").arg(liLine).arg(liCol));
}

void MainWindow::on_actionFont_triggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(
                    &ok, QFont(gsSavedFont, giSavedFontPointSize), this);
    if (ok) {
        gobCurrentPlainTextEdit->setFont(font);
        giSavedFontPointSize = gobCurrentPlainTextEdit->fontInfo().pointSize();
        giSavedFontStyle = gobCurrentPlainTextEdit->fontInfo().style();
        gsSavedFont = gobCurrentPlainTextEdit->fontInfo().family();
    }
}

/**
  Ajusta el comportamiento de la ventana principal, de modo que
  la mantiene encima de las demás en caso de activarse esta opcion.
  @param checked True: Mantiene la ventana encima de las demás. False:
  La ventana se oculta cuando pierde el foco.
*/
void MainWindow::on_actionAlways_on_top_triggered(bool checked)
{
    Qt::WindowFlags flags = this->windowFlags();
    if (checked) {
        this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    }else {
        this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
    }

    this->show();
}

/**
  Muestra la barra de menú en caso de que esta se
  encuentre oculta. La oculta si está visible.
*/
void MainWindow::main_slot_showHideMenuBar()
{
    if(ui->menuBar->isVisible()){
        this->ui->menuBar->hide();
        this->ui->tabWidget->tabBar()->hide();
    }else{
        this->ui->menuBar->show();
        this->ui->tabWidget->tabBar()->show();
    }
}

/**
  Asigna el texto seleccionado al buffer 1. Si no hay nada
  seleccionado, se limpia el buffer.
*/
void MainWindow::main_slot_gr1()
{
    gsGr1 = gobCurrentPlainTextEdit->textCursor().selectedText();
    if(!gsGr1.isEmpty()){
        ui->statusBar->setText("Group #1 assigned.");
    }
    else {
        ui->statusBar->setText("Group #1 cleared.");
    }

    QTimer::singleShot(1500,this,SLOT(main_slot_resetStatusBarText()));
}

/**
  Asigna el texto seleccionado al buffer 2. Si no hay nada
  seleccionado, se limpia el buffer.
*/
void MainWindow::main_slot_gr2()
{
    gsGr2 = gobCurrentPlainTextEdit->textCursor().selectedText();
    if(!gsGr2.isEmpty()){
        ui->statusBar->setText("Group #2 assigned.");
    }
    else {
        ui->statusBar->setText("Group #2 cleared.");
    }

    QTimer::singleShot(1500,this,SLOT(main_slot_resetStatusBarText()));
}

/**
  Asigna el texto seleccionado al buffer 3. Si no hay nada
  seleccionado, se limpia el buffer.
*/
void MainWindow::main_slot_gr3()
{
    gsGr3 = gobCurrentPlainTextEdit->textCursor().selectedText();
    if(!gsGr3.isEmpty()){
        ui->statusBar->setText("Group #3 assigned.");
    }
    else {
        ui->statusBar->setText("Group #3 cleared.");
    }

    QTimer::singleShot(1500,this,SLOT(main_slot_resetStatusBarText()));
}

/**
  Asigna el texto seleccionado al buffer 4. Si no hay nada
  seleccionado, se limpia el buffer.
*/
void MainWindow::main_slot_gr4()
{
    gsGr4 = gobCurrentPlainTextEdit->textCursor().selectedText();
    if(!gsGr4.isEmpty()){
        ui->statusBar->setText("Group #4 assigned.");
    }
    else {
        ui->statusBar->setText("Group #4 cleared.");
    }

    QTimer::singleShot(1500,this,SLOT(main_slot_resetStatusBarText()));
}

/**
  Asigna el texto seleccionado al buffer 5. Si no hay nada
  seleccionado, se limpia el buffer.
*/
void MainWindow::main_slot_gr5()
{
    gsGr5 = gobCurrentPlainTextEdit->textCursor().selectedText();
    if(!gsGr5.isEmpty()){
        ui->statusBar->setText("Group #5 assigned.");
    }
    else {
        ui->statusBar->setText("Group #5 cleared.");
    }

    QTimer::singleShot(1500,this,SLOT(main_slot_resetStatusBarText()));
}

/**
  Pega el texto contenido en el buffer 1
*/
void MainWindow::main_slot_pasteGr1()
{
    gobCurrentPlainTextEdit->textCursor().insertText(gsGr1);
}

/**
  Pega el texto contenido en el buffer 2
*/
void MainWindow::main_slot_pasteGr2()
{
    gobCurrentPlainTextEdit->textCursor().insertText(gsGr2);
}

/**
  Pega el texto contenido en el buffer 3
*/
void MainWindow::main_slot_pasteGr3()
{
    gobCurrentPlainTextEdit->textCursor().insertText(gsGr3);
}

/**
  Pega el texto contenido en el buffer 4
*/
void MainWindow::main_slot_pasteGr4()
{
    gobCurrentPlainTextEdit->textCursor().insertText(gsGr4);
}

/**
  Pega el texto contenido en el buffer 5
*/
void MainWindow::main_slot_pasteGr5()
{
    gobCurrentPlainTextEdit->textCursor().insertText(gsGr5);
}

/**
  Retorna el texto del status bar a su estado anterior. Se usa
  después de haber mostrado un texto temporalmente.
*/
void MainWindow::main_slot_resetStatusBarText()
{
    this->setStatusBarTextAsLink(gobHash.value(giCurrentTabIndex));
}

/**
  Función que se dispara cuando se hace click en un enlace, en la barra de estado.
  Abre el directorio padre del archivo especificado en el texto.
  @param link - Texto del enlace
*/
void MainWindow::on_statusBar_linkActivated(const QString &link)
{
    if(!QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(link).absolutePath())))
    {
        QMessageBox::critical(this,"ERROR","Cannot open containing folder.");
    }
}

/**
  Agrega los archivos recientes al menu de archivos recientes.
*/
void MainWindow::addRecentFiles()
{
    //qDebug() << "Begin addRecentFiles()";

//    if(!gobFileNames.isEmpty()){
//        QString lsFileName = gobFileNames.at(giCurrentFileIndex);
//        loadFile(lsFileName);
//    }

    if(gobRecentFiles.size() >= 10) giRecentAux = 11;
    if(giRecentFilePos >= 10) giRecentFilePos = 0;

    for(int i = 0; i < gobFileNames.size(); i++){

        if(!gobRecentFiles.contains(gobFileNames.at(i)) && giRecentAux <= 10) {
            gobRecentFiles.append(gobFileNames.at(i));
            giRecentFilePos ++;
            giRecentAux ++;
            if(giRecentFilePos >= 10) giRecentFilePos = 0;
        } else if(!gobRecentFiles.contains(gobFileNames.at(i))) {
            gobRecentFiles.replace(giRecentFilePos,gobFileNames.at(i));
            giRecentFilePos ++;
            giRecentAux ++;
        }
    }

    ui->menuOpen_Recent->clear();

    for(int i = 0; i < gobRecentFiles.size(); i++){
        QAction *lobAction = new QAction(gobRecentFiles.at(i), this);
        ui->menuOpen_Recent->insertAction(ui->menuOpen_Recent->actions().at(i),lobAction);
    }

    //qDebug() << "End addRecentFiles()";
}

void MainWindow::main_slot_loadFileFromAction(QAction *aobAction)
{
    //qDebug() << "Begin main_slot_loadFileFromAction, aobAction = " + aobAction->text();
    loadFile(aobAction->text());
    //qDebug() << "End main_slot_loadFileFromAction";
}
