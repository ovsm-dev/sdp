/**************************************************************************
 *                                                                        *
 *  Copyright (C) 2015 OVSM/IPGP                                          *
 *                                                                        *
 *  This file is part of Seismic Data Playback 'SDP'.                     *
 *                                                                        *
 *  SDP is free software: you can redistribute it and/or modify           *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  SDP is distributed in the hope that it will be useful,                *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with SDP. If not, see <http://www.gnu.org/licenses/>.           *
 *                                                                        *
 **************************************************************************/


#include "mainwindow.h"
#include "../../lib/sdp/gui/api.h"
#include <sdp/gui/datamodel/mainframe.h>
#include <sdp/gui/datamodel/logger.h>
#include <sdp/gui/datamodel/splashscreen.h>
#include "ui_mainwindow.h"
#include "../../lib/sdp/gui/datamodel/ui_about.h"
#include <QtGui>


namespace SDP {
namespace Qt4 {


SeismicDataPlayback::SeismicDataPlayback(QWidget* parent) :
		QMainWindow(parent), __ui(new Ui::SeismicDataPlayback),
		__settings("IPGP", "SeismicDataPlayback") {

	setObjectName("SeismicDataPlayback");

	__ui->setupUi(this);
	__ui->statusBar->hide();

	Splashscreen splash(this,Qt::FramelessWindowHint);
	splash.show("Initiating MainFrame...");

	//! Initialize logger first... Mainframe requires it...
	Logger* logger = new Logger(this, objectName());
	logger->hide();

	//! Find out why QSharedPointer seems a bit buggy on some systems and throws
	//! false positive when initialized
	__mainFrame.reset(new MainFrame(__ui->centralWidget));


	splash.show("Preparing user interface...");

	QVBoxLayout* mainLayout = new QVBoxLayout(__ui->centralWidget);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(__mainFrame.data());

	connect(__ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	connect(__ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
	connect(__ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(__ui->actionLog, SIGNAL(triggered()), logger, SLOT(show()));


#ifndef Q_OS_MAC
	setWindowIcon(QIcon(":images/Logo_short_RGB_72_OVS_horiz.png"));
#endif

	__minimizeAction = new QAction(tr("M&inimize"), this);
	connect(__minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

	__maximizeAction = new QAction(tr("M&aximize"), this);
	connect(__maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

	__restoreAction = new QAction(tr("R&estore"), this);
	connect(__restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

	__quitAction = new QAction(tr("Q&uit"), this);
	connect(__quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

	__trayIconMenu = new QMenu(this);
	__trayIconMenu->addAction(__minimizeAction);
	__trayIconMenu->addAction(__maximizeAction);
	__trayIconMenu->addAction(__restoreAction);
	__trayIconMenu->addSeparator();
	__trayIconMenu->addAction(__quitAction);

	__trayIcon = new QSystemTrayIcon(this);
	__trayIcon->setContextMenu(__trayIconMenu);
	__trayIcon->setIcon(QIcon(":images/Logo_short_RGB_72_OVS_horiz.png"));
	__trayIcon->show();

	splash.show("Loading inventory...");
	__mainFrame->loadInventory();

	//! Force hide() and let the timer do its thing...
	splash.hide();
}


SeismicDataPlayback::~SeismicDataPlayback() {

	__settings.beginGroup(objectName());
	__settings.setValue("geometry", saveGeometry());
	__settings.setValue("state", saveState());
	__settings.endGroup();
}


void SeismicDataPlayback::showEvent(QShowEvent*) {

	__settings.beginGroup(objectName());
	restoreGeometry(__settings.value("geometry").toByteArray());
	restoreState(__settings.value("state").toByteArray());
	__settings.endGroup();
}


void SeismicDataPlayback::closeEvent(QCloseEvent* event) {
	(__mainFrame->clearForExit()) ? event->accept() : event->ignore();
}


void SeismicDataPlayback::about() {

	QPixmap logo(":images/Logo_short_RGB_72_OVS_horiz.png");
	logo = logo.scaled(120, 120, Qt::KeepAspectRatio, Qt::FastTransformation);

	QDialog d(this);
	QScopedPointer<Ui::About> ui(new Ui::About);
	ui->setupUi(&d);
	ui->labelLogo->setPixmap(logo);
	//! Seal the GNU license container and prevent edition
	ui->textEdit->setReadOnly(true);
	ui->labelAppName->setText(PROJECT_NAME);
	ui->labelVersion->setText(PROJECT_VERSION);
	ui->labelShortDescription->setText(tr("A simple script manager."));
	ui->textEditFullDescription->setText(
	    tr("SeismicDataPlayback is a simple GUI for event detection and data dispatch.<br/>"
		    "The main goal of this application is to help setup miscellaneous scripts "
		    "that will be run by a tasks manager. <i>Dispatch</i> scripts are written in bash, "
		    "and require <b>msrouter</b> and/or <b>msmod</b> (which are compiled by the installer) "
		    "to route data, whereas <i>detection</i> scripts are written in <b>python</b> "
		    "and require <b>python-obspy</b> package in order to work properly."));
	ui->textEditFullDescription->setReadOnly(true);

	d.exec();
}


} // namespace Qt4
} // namespace SDP
