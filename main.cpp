#include "mainwindow.h"

#include <QtWidgets/QApplication>
#include <QClipboard>
#include <QString>
#include <QTextStream>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <string.h>
#include <unistd.h>
#include <pqxx/pqxx>

#define STREAM
QTextStream cout(stdout);
QTextStream cin(stdin);
QTextStream cerr(stderr);

#include "password_item.h"
#include "database_manager.h"
#include "encryption.h"
#include "password_manager.h"
#include "autorisation.h"
//#include "cionout.h"
#ifndef STREAM
    #define STREAM
    QTextStream cout(stdout);
    QTextStream cin(stdin);
    QTextStream cerr(stderr);
#endif


int main(int argc, char *argv[])
{
    //db_name, table_name, password(database), user_name
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    DataBase db("passman");
    std::cout << "Connection" << std::endl;
    PassMan passman(db);

    for(int i = 0; i < 3; i++) {
        cout << "________Autorisation________\n"; cout.flush();
        bool autorisation = passman.Autorisation();
        if(autorisation) {
            break;
        }
        if(i == 2) {
            cout << "All 3 attempts are spent!\n"; cout.flush();
            return 1;
        }
    }
    passman.Menu();

    return a.exec();
}
