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
#include <QPushButton>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QCheckBox>
#include <QMessageBox>
#include <QMimeData>
#include <QShortcut>
#include <QScrollBar>
#include <QDialog>
#include <QDebug>
#include <QEvent>
#include <QTabBar>
#include <QTimer>
#include <QScreen>
#include <QMap>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    gbIsAutoreloadEnabled = false;
    gbIsOpenedFile = false;
    gbIsReloadFile = false;
    gbSaveCancelled = false;
    gbShowEraseAndSaveMessageBox = true;
    gfMaxFileSize = 1000.00;
    giCurrentFileIndex = 0;
    giCurrentTabIndex = 0;
    giDefaultDirCounter = 0;
    giOpenWithFlag = 0;
    giTimerDelay = 20;
    giTotalTabs = 0;
    gobFileNames.clear();
    gobRecentFiles.clear();
    gobMovie = new QMovie("://reloading.gif");
    gobMovie->setScaledSize(QSize(15,15));
    gobSearchDialog = new SearchDialog(this);
    gobTimer = new QTimer(this);
    gsDefaultDir = QDir::homePath();
    gsThemeFile = "Default";
    gsStatusBarColor = "orange";
    worker = new Worker;
    workerThread = new QThread;
    gobDownloadManager = new DownloadManager();
    //gobDownloadManager->execute("http://gtronick.com/versions/QNote_version.txt");
    setAcceptDrops(true);
    loadConfig();
    QShortcut *menuBar_shortcut = new QShortcut(QKeySequence(tr("Ctrl+M")),this);

    QShortcut *gr1_shortcut = new QShortcut(QKeySequence(tr("Ctrl+1")),this);
    QShortcut *gr2_shortcut = new QShortcut(QKeySequence(tr("Ctrl+2")),this);
    QShortcut *gr3_shortcut = new QShortcut(QKeySequence(tr("Ctrl+3")),this);
    QShortcut *gr4_shortcut = new QShortcut(QKeySequence(tr("Ctrl+4")),this);

    QShortcut *paste_gr1_shortcut = new QShortcut(QKeySequence(tr("F1")),this);
    QShortcut *paste_gr2_shortcut = new QShortcut(QKeySequence(tr("F2")),this);
    QShortcut *paste_gr3_shortcut = new QShortcut(QKeySequence(tr("F3")),this);
    QShortcut *paste_gr4_shortcut = new QShortcut(QKeySequence(tr("F4")),this);

    QShortcut *openFileLocation_shortCut = new QShortcut(QKeySequence(tr("F9")),this);

    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(main_slot_tabChanged(int)));
    connect(ui->tabWidget,SIGNAL(ctw_signal_tabMoved(int,int)),this,SLOT(main_slot_tabMoved(int,int)));
    connect(ui->menuOpen_Recent, SIGNAL(triggered(QAction*)), this, SLOT(main_slot_loadFileFromAction(QAction*)));
    connect(this,SIGNAL(main_signal_setTextEdit(QPlainTextEdit*)),gobSearchDialog,SLOT(search_slot_setTextEdit(QPlainTextEdit*)));
    connect(gobSearchDialog,SIGNAL(search_signal_getTextEditText()),this,SLOT(main_slot_getTextEditText()));
    connect(gobSearchDialog,SIGNAL(search_signal_resetCursor()),this,SLOT(main_slot_resetCursor()));
    connect(this,SIGNAL(main_signal_loadFile(QFile*)),worker,SLOT(worker_slot_loadFile(QFile*)));
    connect(worker,SIGNAL(worker_signal_appendText(QString)),this,SLOT(main_slot_appendText(QString)));
    connect(worker,SIGNAL(worker_signal_insertText(QString)),this,SLOT(main_slot_insertText(QString)));
    connect(gobTimer, SIGNAL(timeout()), this, SLOT(main_slot_tailFile()));
    connect(menuBar_shortcut,SIGNAL(activated()),this,SLOT(main_slot_showHideMenuBar()));
    connect(this,SIGNAL(main_signal_tailFile(QFile*)),worker,SLOT(worker_slot_tailFile(QFile*)));
    connect(this,SIGNAL(main_signal_setCurrentFileSize(int)),worker,SLOT(worker_slot_setCurrentFileSize(int)));

    connect(gr1_shortcut,SIGNAL(activated()),this,SLOT(main_slot_gr1()));
    connect(gr2_shortcut,SIGNAL(activated()),this,SLOT(main_slot_gr2()));
    connect(gr3_shortcut,SIGNAL(activated()),this,SLOT(main_slot_gr3()));
    connect(gr4_shortcut,SIGNAL(activated()),this,SLOT(main_slot_gr4()));

    connect(paste_gr1_shortcut,SIGNAL(activated()),this,SLOT(main_slot_pasteGr1()));
    connect(paste_gr2_shortcut,SIGNAL(activated()),this,SLOT(main_slot_pasteGr2()));
    connect(paste_gr3_shortcut,SIGNAL(activated()),this,SLOT(main_slot_pasteGr3()));
    connect(paste_gr4_shortcut,SIGNAL(activated()),this,SLOT(main_slot_pasteGr4()));

    connect(openFileLocation_shortCut,SIGNAL(activated()),this,SLOT(main_slot_openFileLocation()));

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
    //qDebug() << "Begin on_actionOpen_triggered";
    int liTempGobFileNamesSize = gobFileNames.size();

    giDefaultDirCounter ++;

    if(giDefaultDirCounter > 1) {
        gsDefaultDir = "";
        giDefaultDirCounter = 2;
    }

    if(this->giOpenWithFlag == 0){

        gobFileNames.append(QFileDialog::getOpenFileNames(this
                                            ,"Open File"
                                            ,gsDefaultDir
                                            ,tr("All Files (*);;Text Files (*.txt);;Log files (*.log)")));

    }

    gobFileNames = removeDuplicates(gobFileNames);

    if(!gobFileNames.isEmpty() && gobFileNames.size() > liTempGobFileNamesSize){
        QString lsFileName = gobFileNames.at(giCurrentFileIndex);
        loadFile(lsFileName);
    }

    this->giOpenWithFlag = 0;

    this->addRecentFiles();
    //qDebug() << "End on_actionOpen_triggered";
}

bool MainWindow::on_actionSave_As_triggered()
{
    QString lsFileName;

    lsFileName = QFileDialog::getSaveFileName(this
                                              ,"Save File"
                                              ,""
                                              ,tr("Text Files (*.txt);;All Files (*)"));

    giCurrentTabIndex = ui->tabWidget->currentIndex();

    if(lsFileName != NULL && !lsFileName.isEmpty() && gobFilePathsHash.value(giCurrentTabIndex) != lsFileName){
        gobFileNames.removeAt(gobFileNames.indexOf(gobFilePathsHash.value(giCurrentTabIndex)));
        gobFilePathsHash.remove(giCurrentTabIndex);
        gobFilePathsHash.insert(giCurrentTabIndex,lsFileName);

        return saveFile(lsFileName);
    } else {
        return false;
    }
}

bool MainWindow::on_actionSave_triggered()
{
    QString lsFileName;

    giCurrentTabIndex = ui->tabWidget->currentIndex();
    lsFileName = gobFilePathsHash.value(giCurrentTabIndex);

    QFile file(lsFileName);

    if(file.exists()){
        return saveFile(lsFileName);
    }else{
        return on_actionSave_As_triggered();
    }
}

bool MainWindow::saveFile(QString asFileName, QString asText)
{
    if(asFileName.isEmpty()){

        return false;
    }

    QFile file(asFileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::critical(this,"Error","File could not be opened");
        return false;
    }

    QTextStream out(&file);
    out << asText;
    file.close();

    return true;
}

bool MainWindow::saveFile(QString asFileName)
{
    QPlainTextEdit* lobPlainTextEdit = qobject_cast<QPlainTextEdit*>(ui->tabWidget->widget(giCurrentTabIndex));
    if(!saveFile(asFileName,lobPlainTextEdit->toPlainText())) return false;
    ui->indicatorLabel->clear();
    if(gobIsModifiedTextHash.contains(giCurrentTabIndex)) {
        gobIsModifiedTextHash.remove(giCurrentTabIndex);
    }
    gobIsModifiedTextHash.insert(giCurrentTabIndex,false);
    setCurrentTabNameFromFile(asFileName);
    this->setStatusBarTextAsLink(asFileName);
    if(!gobFileNames.contains(asFileName)){
        gobFileNames.append(asFileName);
        giCurrentFileIndex ++;
        this->addRecentFiles();
    }

    return true;
}

bool MainWindow::saveConfig()
{

    QString configText = "[THEME]\n";
    configText = configText + "themeFile=" + gsThemeFile + "\n";
    configText = configText + "statusBarColor=" + gsStatusBarColor + "\n";
    configText = configText + "[FONT]" + "\n";
    configText = configText + "family=" + gobCurrentPlainTextEdit->fontInfo().family() + "\n";
    configText = configText + "size=" + QString::number(gobCurrentPlainTextEdit->fontInfo().style()) + "\n";
    configText = configText + "point=" + QString::number(gobCurrentPlainTextEdit->fontInfo().pointSize()) + "\n";
    configText = configText + "[GEOMETRY]" + "\n";
    configText = configText + "width=" + QString::number(this->geometry().width()) + "\n";
    configText = configText + "height=" + QString::number(this->geometry().height()) + "\n";
    configText = configText + "x=" + QString::number(this->geometry().x()) + "\n";
    configText = configText + "y=" + QString::number(this->geometry().y()) + "\n";
    configText = configText + "maximized=" + QString("%1").arg(this->isMaximized()) + "\n";
    configText = configText + "[ALERTS]" + "\n";
    configText = configText + "showEraseAlert=" + QString("%1").arg(this->gbShowEraseAndSaveMessageBox) + "\n";
    configText = configText + "[FILESIZE]" + "\n";
    configText = configText + "maxFileSize=" + QString("%1").arg(gfMaxFileSize) + "\n";
    configText = configText + "[RELOAD]" + "\n";
    configText = configText + "delay=" + QString("%1").arg(giTimerDelay) + "\n";
    configText = configText + "[RECENTS]" + "\n";
    configText = configText + "recentFiles=";

    for(QString lsFile:gobRecentFiles){
        configText = configText + "@@" + lsFile;
    }

    if(!saveFile("QNote_config.ini",configText)) {
        QMessageBox::critical(this,"ERROR","Can't save configuration file!");
        return false;
    } else {
        return true;
    }
}

bool MainWindow::loadConfig()
{
    QString line;
    QScreen *screen = QGuiApplication::primaryScreen();
    int liIsMaximized = 0;
    QFile *lobConfigFile = new QFile("QNote_config.ini");

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
        else if(line.startsWith("statusBarColor")) gsStatusBarColor = line.split("=").at(1);
        else if(line.startsWith("family")) gsSavedFont = line.split("=").at(1);
        else if(line.startsWith("maximized")) {

            if(line.split("=").at(1).toInt() == 1 ) {
                this->resize(585,453);
                this->move(screen->availableGeometry().center() - this->rect().center());
                this->showMaximized();
                liIsMaximized = 1;
            }
        }
        else if(line.startsWith("size")) giSavedFontStyle = line.split("=").at(1).toInt();
        else if(line.startsWith("point")) giSavedFontPointSize = line.split("=").at(1).toInt();
        else if(line.startsWith("width")  && liIsMaximized == 0) this->resize(line.split("=").at(1).toInt(),this->geometry().height());
        else if(line.startsWith("height")  && liIsMaximized == 0) this->resize(this->geometry().width(),line.split("=").at(1).toInt());
        else if(line.startsWith("x")  && liIsMaximized == 0) this->move(line.split("=").at(1).toInt(),this->geometry().y());
        else if(line.startsWith("y")  && liIsMaximized == 0) this->move(this->geometry().x(),line.split("=").at(1).toInt());
        else if(line.startsWith("showEraseAlert")) gbShowEraseAndSaveMessageBox = line.split("=").at(1).toInt();
        else if(line.startsWith("maxFileSize")) gfMaxFileSize = line.split("=").at(1).toFloat();
        else if(line.startsWith("delay")) giTimerDelay = line.split("=").at(1).toInt();
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
    QMessageBox::about(this,"QNote 1.7.2",
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

void MainWindow::on_actionClose_Tab_triggered()
{
    checkIfUnsaved(giCurrentTabIndex);
}

void MainWindow::on_actionNew_Tab_triggered()
{
    //qDebug() << "Begin on_actionNew_Tab_triggered";
    disableAutoReload();
    giTotalTabs ++;
    QApplication::processEvents();
    CustomTextEdit *lobPlainTexEdit = new CustomTextEdit();
    lobPlainTexEdit->setPlaceholderText("Type Here...");
    lobPlainTexEdit->setFrameShape(QFrame::NoFrame);
    lobPlainTexEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    lobPlainTexEdit->setAcceptDrops(true);
    int fontWidth = QFontMetrics(lobPlainTexEdit->currentCharFormat().font()).averageCharWidth();
    lobPlainTexEdit->setTabStopDistance(TABCHARS * fontWidth );
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
    gobFilePathsHash.insert(giCurrentTabIndex,"");
    gobIsModifiedTextHash.insert(giCurrentTabIndex,false);
    //qDebug() << "End on_actionNew_Tab_triggered";
}

void MainWindow::on_actionReload_file_triggered()
{
    QString lsFileName = gobFilePathsHash.value(giCurrentTabIndex);

    if(checkFileExist(lsFileName)){
        QFile *lobFile = new QFile(lsFileName);
        gbIsReloadFile = true;

        if(!gbIsAutoreloadEnabled) checkIfUnsaved(giCurrentTabIndex);

        if(gbSaveCancelled == false){

            setCurrentTabNameFromFile(lsFileName);

            if(!lobFile->open(QIODevice::ReadOnly | QIODevice::Text)){
                QMessageBox::critical(this,"Error","File could not be opened");
                gbIsReloadFile = false;
            }
            emit main_signal_loadFile(lobFile);
        }else{
            gbSaveCancelled = false;
        }

    }else{
        gbIsReloadFile = false;
        if(gobTimer->isActive()){
            gobTimer->blockSignals(true);
            gobTimer->stop();
            disableAutoReload();
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
        if(moves){
            if(moves<0) cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, -moves);
            else cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, moves);

            cursor.movePosition(QTextCursor::StartOfLine);

            gobCurrentPlainTextEdit->setTextCursor(cursor);
        }
    }
}

void MainWindow::on_actionFind_Replace_triggered()
{
    gobSearchDialog->focusOnSearchInputText();
    gobSearchDialog->setSearchText(this->gobCurrentPlainTextEdit->textCursor().selectedText());
    gobSearchDialog->setVisible(true);
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
    }else{
        this->setStyleSheet("");
    }

    emit main_signal_refreshHighlight();
}

void MainWindow::on_actionSystem_theme_triggered()
{
    this->setStyleSheet("");
    gsThemeFile = "";
    emit main_signal_refreshHighlight();
}

void MainWindow::on_actionAuto_Reload_tail_f_toggled(bool arg1)
{
    QString lsFileName = gobFilePathsHash.value(giCurrentTabIndex);
    gbIsAutoreloadEnabled = arg1;

    if(arg1 && checkFileExist(lsFileName)){
        gobCurrentPlainTextEdit->clear();
        QFile *lobFile = new QFile(lsFileName);
        emit main_signal_setCurrentFileSize(lobFile->size());
        checkIfUnsaved(giCurrentTabIndex);
        ui->indicatorLabel->setToolTip("Auto Reload Active");
        this->ui->indicatorLabel->setMovie(gobMovie);
        gobTimer->blockSignals(false);
        gobTimer->start(giTimerDelay);
        ui->indicatorLabel->movie()->start();
        lockTextEditor();
    }else{
        gbIsReloadFile = false;
        gobCurrentPlainTextEdit->setReadOnly(false);
        gobCurrentPlainTextEdit->setTextInteractionFlags(Qt::TextEditorInteraction);
        ui->indicatorLabel->setToolTip("");
        if(gobTimer->isActive()) gobTimer->stop();
        if(ui->indicatorLabel->movie() != NULL) ui->indicatorLabel->movie()->stop();
        ui->indicatorLabel->clear();

        disconnect(ui->actionAuto_Reload_tail_f,SIGNAL(toggled(bool)),this,SLOT(on_actionAuto_Reload_tail_f_toggled(bool)));
        ui->actionAuto_Reload_tail_f->setChecked(false);
        connect(ui->actionAuto_Reload_tail_f,SIGNAL(toggled(bool)),this,SLOT(on_actionAuto_Reload_tail_f_toggled(bool)));

        if(!checkFileExist(lsFileName)){
            if(showCustomMessage("ERROR","The file can't be loaded","Reload")){
               gbIsAutoreloadEnabled = true;
               gobCurrentPlainTextEdit->clear();
               ui->actionAuto_Reload_tail_f->setChecked(true);
            }
        }
    }
}

void MainWindow::on_actionAuto_Reload_delay_triggered()
{
    bool lbOk;
    int liDelay = QInputDialog::getInt(this, tr("Auto Reload delay (ms)"),
                                 tr("milliseconds:"), giTimerDelay, 10, 5000, 1, &lbOk);
    if (lbOk) giTimerDelay = liDelay;
    if(gobTimer->isActive()){
        gobTimer->stop();
        gobTimer->start(giTimerDelay);
    }
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

void MainWindow::on_actionErase_and_save_triggered()
{
    int ret = QMessageBox::Ok;

    if(this->gbShowEraseAndSaveMessageBox){
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Warning!");
        msgBox.setText("Do you want to erase and save the file contents?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        QCheckBox *lobCheckBox = new QCheckBox("Don't show again");
        msgBox.setCheckBox(lobCheckBox);
        connect(lobCheckBox,SIGNAL(toggled(bool)),this,SLOT(main_slot_dontShowAgain(bool)));
        ret = msgBox.exec();
    }

    if(ret == QMessageBox::Ok){
        if(this->gbIsAutoreloadEnabled){
            this->ui->actionAuto_Reload_tail_f->setChecked(false);
        }

        gobCurrentPlainTextEdit = qobject_cast<CustomTextEdit*>(ui->tabWidget->widget(giCurrentTabIndex));
        gobCurrentPlainTextEdit->clear();
        this->on_actionSave_triggered();

        if(!this->gbIsAutoreloadEnabled){
            this->ui->actionAuto_Reload_tail_f->setChecked(true);
        }
    }
}

void MainWindow::on_actionReset_alerts_triggered()
{
    this->gbShowEraseAndSaveMessageBox = true;
}

void MainWindow::on_actionSet_Maximun_file_size_triggered()
{
    bool lbOk;
    float liMaxFileSize = QInputDialog::getDouble(this, tr("Maximun File Size (MB)"),
                                 tr("Mega Bytes:"),gfMaxFileSize, 0.01, 1000, 2, &lbOk);
    if (lbOk) gfMaxFileSize = liMaxFileSize;
}

void MainWindow::on_actionTo_UPERCASE_triggered()
{
    QTextCursor lobCursor = gobCurrentPlainTextEdit->textCursor();
    QString lsText = lobCursor.selectedText().toUpper();
    lobCursor.insertText(lsText);
}

void MainWindow::on_actionTo_lowercase_triggered()
{
    QTextCursor lobCursor = gobCurrentPlainTextEdit->textCursor();
    QString lsText = lobCursor.selectedText().toLower();
    lobCursor.insertText(lsText);
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    checkIfUnsaved(index);
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
    giCurrentTabIndex = aIndex;    
    this->setStatusBarTextAsLink(gobFilePathsHash.value(aIndex));
    gobCurrentPlainTextEdit = qobject_cast<CustomTextEdit*>(ui->tabWidget->widget(giCurrentTabIndex));
    QFont serifFont(gsSavedFont, giSavedFontPointSize, giSavedFontStyle);
    gobCurrentPlainTextEdit->setFont(serifFont);
    if(gobIsModifiedTextHash.value(aIndex)){
        ui->indicatorLabel->setPixmap(QPixmap("://unsaved.png"));
    }else if(gbIsAutoreloadEnabled){
        emit main_signal_setCurrentFileSize(0);
        ui->indicatorLabel->setToolTip("Auto Reload Active");
        this->ui->indicatorLabel->setMovie(gobMovie);
        gobTimer->start(giTimerDelay);
        ui->indicatorLabel->movie()->start();
        lockTextEditor();
        gobCurrentPlainTextEdit->clear();
    }else{
        if(!gbIsAutoreloadEnabled) ui->indicatorLabel->clear();
        gobCurrentPlainTextEdit->setReadOnly(false);
        gobCurrentPlainTextEdit->setTextInteractionFlags(Qt::TextEditorInteraction);
    }
}

void MainWindow::main_slot_tabMoved(int from, int to)
{
    QString lsTemporalValue = gobFilePathsHash.value(to);
    gobFilePathsHash.insert(to,gobFilePathsHash.value(from));
    gobFilePathsHash.insert(from,lsTemporalValue);

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
    gobCurrentPlainTextEdit->setUndoRedoEnabled(false);

    QString lsFileName;

    gbIsOpenedFile = true;
    gobCurrentPlainTextEdit->clear();
    gobCurrentPlainTextEdit->appendPlainText(asText);
    gobIsModifiedTextHash.insert(giCurrentTabIndex,false);
    gbIsOpenedFile = false;

    if(gbIsReloadFile){
        gbIsReloadFile = false;
    }else{
        main_slot_resetCursor();
    }

    gobCurrentPlainTextEdit->setUndoRedoEnabled(true);

    if(giCurrentFileIndex < gobFileNames.length()){
        lsFileName = gobFileNames.at(giCurrentFileIndex);
        loadFile(lsFileName);
    }
}

void MainWindow::main_slot_insertText(QString asText)
{
    gbIsOpenedFile = true;
    QScrollBar *lobBar = gobCurrentPlainTextEdit->verticalScrollBar();

    if(lobBar->value() == lobBar->maximum()) {      //Si estamos al final del archivo
        gobCurrentPlainTextEdit->insertPlainText(asText);
        lockTextEditor();
    } else {        //Si estamos en un punto intermedio y queremos hacer scroll
        gobCurrentPlainTextEdit->insertPlainText(asText);
    }

    gobIsModifiedTextHash.insert(giCurrentTabIndex,false);
    gbIsOpenedFile = false;

    if(gbIsReloadFile){
        gbIsReloadFile = false;
    }
}

void MainWindow::main_slot_processDropEvent(QDropEvent *event)
{
//    qDebug() << "Begin main_slot_processDropEvent";

    const QMimeData* mimeData = event->mimeData();
    int liTempGobFileNamesSize = gobFileNames.size();

    if (mimeData->hasUrls()){

        QList<QUrl> urlList = mimeData->urls();

        for (int i = 0; i < urlList.size(); i++) {
            if(!gobFileNames.contains(urlList.at(i).toLocalFile())){
                gobFileNames.append(urlList.at(i).toLocalFile());
            }
        }
        if(!gobFileNames.isEmpty() && gobFileNames.size() > liTempGobFileNamesSize){

            QString lsFileName = gobFileNames.at(giCurrentFileIndex);

            loadFile(lsFileName);

        }
        addRecentFiles();
        event->acceptProposedAction();
    }

//    qDebug() << "End main_slot_processDropEvent";
}

void MainWindow::main_slot_currentLineChanged()
{
    int liLine = gobCurrentPlainTextEdit->textCursor().blockNumber() + 1;
    int liCol = gobCurrentPlainTextEdit->textCursor().columnNumber() + 1;
    ui->lineNumberLabel->setText(QString("Line: %1, Col: %2").arg(liLine).arg(liCol));
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

void MainWindow::main_slot_dontShowAgain(bool abValue)
{
    this->gbShowEraseAndSaveMessageBox = !abValue;
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

void MainWindow::main_slot_openFileLocation()
{
    if(!gobFilePathsHash.value(giCurrentTabIndex).isNull() && !gobFilePathsHash.value(giCurrentTabIndex).isEmpty()) {
        on_statusBar_linkActivated(gobFilePathsHash.value(giCurrentTabIndex));
    }
}

/**
  Retorna el texto del status bar a su estado anterior. Se usa
  después de haber mostrado un texto temporalmente.
*/
void MainWindow::main_slot_resetStatusBarText()
{
    this->setStatusBarTextAsLink(gobFilePathsHash.value(giCurrentTabIndex));
}

void MainWindow::main_slot_loadFileFromAction(QAction *aobAction)
{
    if(aobAction->text().compare("Clear list") != 0 && !gobFileNames.contains(aobAction->text())) {
            gobFileNames.append(aobAction->text());
            loadFile(aobAction->text());
    } else if(aobAction->text().compare("Clear list") == 0) {
        ui->menuOpen_Recent->clear();
        gobRecentFiles.clear();
    }
}

void MainWindow::main_slot_tailFile()
{
    QString lsFileName = gobFilePathsHash.value(giCurrentTabIndex);

    if(checkFileExist(lsFileName)){
        QFile *lobFile = new QFile(lsFileName);
        gbIsReloadFile = true;

        if(!gbIsAutoreloadEnabled) checkIfUnsaved(giCurrentTabIndex);

        if(gbSaveCancelled == false){

            setCurrentTabNameFromFile(lsFileName);
            emit main_signal_tailFile(lobFile);
        }else{
            gbSaveCancelled = false;
        }

    }else{
        gbIsReloadFile = false;
        if(gobTimer->isActive()){
            gobTimer->blockSignals(true);
            gobTimer->stop();
            gbIsAutoreloadEnabled = false;
            ui->actionAuto_Reload_tail_f->setChecked(false);
        }
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
    msgBox.setText("Do you want to save your changes?");
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
    QFile *lobFile = new QFile(asFileName);

    if(!asFileName.isEmpty() && lobFile->exists()){
        return true;
    }

    return false;
}
/*
int MainWindow::checkFileSize(QString asFileName)
{
    double liFileSize = QFile(asFileName).size();
    int returnValue = QMessageBox::Ok;

    if(liFileSize/1000000.00 > (gfMaxFileSize)){
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Maximum file size exceeded");
        msgBox.setText("The file: \n" + asFileName + "\nIs too big. Do you want to open it anyway?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        returnValue = msgBox.exec();
    }

    return returnValue;
}
*/
void MainWindow::closeTab(int index)
{
    QFile *lobFile;

    if(giTotalTabs > 1){
        lobFile = new QFile(gobFilePathsHash.value(index));
        gobFileNames.removeAt(gobFileNames.indexOf(gobFilePathsHash.value(index)));
        giCurrentFileIndex --;
        gobFilePathsHash.remove(index);
        for(int i = index; i < giTotalTabs; i ++){
            gobFilePathsHash.insert(i,gobFilePathsHash.value(i + 1));
        }

        gobIsModifiedTextHash.remove(index);
        for(int i = index; i < giTotalTabs; i ++){
            gobIsModifiedTextHash.insert(i,gobIsModifiedTextHash.value(i + 1));
        }

        this->setStatusBarTextAsLink(gobFilePathsHash.value(giCurrentTabIndex));
        if(lobFile->isOpen()){
            lobFile->close();
            lobFile = NULL;
        }

        giTotalTabs --;
        this->ui->tabWidget->removeTab(index);

    }else{

        giTotalTabs = 0;
        giCurrentTabIndex = 0;
        giCurrentFileIndex = 0;
        gobFileNames.clear();
        gobIsModifiedTextHash.clear();
        gobFilePathsHash.clear();
        on_actionNew_Tab_triggered();
        this->ui->tabWidget->removeTab(index);
    }
}

void MainWindow::loadFile(QString asFileName)
{
    //qDebug() << "Begin loadFile, asFileName: " << asFileName;

    QFile *lobFile = new QFile(asFileName);

    if(!asFileName.isEmpty() && lobFile->exists()){

        disableAutoReload();

        emit main_signal_setCurrentFileSize(lobFile->size());

        if(gobFilePathsHash.value(giCurrentTabIndex) != NULL && gobFilePathsHash.value(giCurrentTabIndex) != ""){
            on_actionNew_Tab_triggered();
        }

        gobFilePathsHash.insert(giCurrentTabIndex,asFileName);
        setCurrentTabNameFromFile(asFileName);
        main_slot_tabChanged(giCurrentTabIndex);

        if(!lobFile->open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox::critical(this,"Error","File could not be opened");
            return;
        }

        gobCurrentPlainTextEdit = qobject_cast<CustomTextEdit*>(ui->tabWidget->widget(giCurrentTabIndex));
        giCurrentFileIndex ++;      //Aumenta el cursor para leer el siguiente archivo en el mapa global de archivos
        emit main_signal_loadFile(lobFile);

    } else {
        QMessageBox::warning(this,"Error!","The file " + asFileName + " can't be opened.");
        gobFileNames.removeAt(giCurrentFileIndex);
    }
    //qDebug() << "End loadFile";
}

void MainWindow::setFileNameFromCommandLine(QStringList asFileNames)
{
    gobFileNames = asFileNames;
    this->giOpenWithFlag = 1;
    this->on_actionOpen_triggered();
}

void MainWindow::setStatusBarTextAsLink(QString asText)
{
    ui->statusBar->setText(
                "<style>"
                "a:link {"
                    "color:"+ gsStatusBarColor +";"
                    "background-color: transparent;"
                    "text-decoration: none;"
                "}"
                "</style>"
                "<a href=\"" + asText + "\">" + asText + "</a>");
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
        saveConfig();
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

    for(int i = 0; i < gobFileNames.size(); i++){

        if(!gobRecentFiles.contains(gobFileNames.at(i))) {

            if(gobRecentFiles.size() < MAX_RECENT) {
                gobRecentFiles.append(" ");
            }

            int liLastArrayPosition = gobRecentFiles.size() - 1;
            for(int j = liLastArrayPosition; j > 0; j--) {
                gobRecentFiles.move(j - 1,j);
            }

            gobRecentFiles.replace(0,gobFileNames.at(i));
        }
    }

    ui->menuOpen_Recent->clear();

    int i = 0;

    for(i = 0; i < gobRecentFiles.size(); i++){
        ui->menuOpen_Recent->addAction(gobRecentFiles.at(i));
    }

    ui->menuOpen_Recent->addSeparator();
    ui->menuOpen_Recent->addAction("Clear list");
}

void MainWindow::disableAutoReload()
{
    if(gbIsAutoreloadEnabled) this->on_actionAuto_Reload_tail_f_toggled(false);
}

QStringList MainWindow::removeDuplicates(QStringList aobList)
{
    //qDebug() << "Begin removeDuplicates, aobList = " << aobList;
    QStringList returnList = aobList;
    int liFound = 0;

    for(int i = 0; i < returnList.size(); i++) {
        for(int j = 0; j < returnList.size(); j++) {
            if(returnList.at(i) == returnList.at(j)) {
                liFound ++;
                if(liFound > 1) {
                    returnList.removeAt(j);
                    liFound = 1;
                }
            }
        }

        liFound = 0;
    }
    //qDebug() << "End removeDuplicates, returnList = " << returnList;

    return returnList;
}

void MainWindow::lockTextEditor()
{
    gobCurrentPlainTextEdit->setReadOnly(true);
    QTextCursor lobCursor = gobCurrentPlainTextEdit->textCursor();
    lobCursor.setPosition(gobCurrentPlainTextEdit->toPlainText().size());
    gobCurrentPlainTextEdit->setTextCursor(lobCursor);
    gobCurrentPlainTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
    gobCurrentPlainTextEdit->verticalScrollBar()->setValue(gobCurrentPlainTextEdit->verticalScrollBar()->maximum());
}

bool MainWindow::showCustomMessage(QString asTitle, QString asText, QString asCustomButtonText)
{
    QMessageBox *lobMesgBox = new QMessageBox(this);
    lobMesgBox->setWindowTitle(asTitle);
    lobMesgBox->setText(asText);
    QPushButton *lobButton = new QPushButton(asCustomButtonText,lobMesgBox);
    lobMesgBox->addButton(lobButton,QMessageBox::AcceptRole);
    lobMesgBox->addButton(QMessageBox::Cancel);
    lobMesgBox->exec();
    return lobMesgBox->clickedButton() == lobButton ? true : false;
}

void MainWindow::on_actionErase_and_save_2_triggered()
{
    int ret = QMessageBox::Ok;

    if(this->gbShowEraseAndSaveMessageBox){
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Warning!");
        msgBox.setText("Do you want to erase and save the file contents?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        QCheckBox *lobCheckBox = new QCheckBox("Don't show again");
        msgBox.setCheckBox(lobCheckBox);
        connect(lobCheckBox,SIGNAL(toggled(bool)),this,SLOT(main_slot_dontShowAgain(bool)));
        ret = msgBox.exec();
    }

    if(ret == QMessageBox::Ok){
        disableAutoReload();
        gobCurrentPlainTextEdit = qobject_cast<CustomTextEdit*>(ui->tabWidget->widget(giCurrentTabIndex));
        gobCurrentPlainTextEdit->clear();
        this->on_actionSave_triggered();
    }
}
