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



#include "../api.h"

#include <sdp/gui/datamodel/mainframe.h>
#include <sdp/gui/datamodel/cache.h>
#include <sdp/gui/datamodel/progress.h>
#include <sdp/gui/datamodel/config.h>
#include <sdp/gui/datamodel/parametermanager.h>
#include <sdp/gui/datamodel/databasemanager.h>
#include <sdp/gui/datamodel/system.h>
#include <sdp/gui/datamodel/logger.h>
#include <sdp/gui/datamodel/job.h>
#include <sdp/gui/datamodel/fancywidgets.h>
#include <sdp/gui/datamodel/macros.h>
#include <sdp/gui/datamodel/panels.h>
#include <sdp/gui/datamodel/utils.h>
#include "ui_mainframe.h"
#include <QtGui>


namespace SDP {
namespace Qt4 {


MainFrame::Session::Session() :
		registered(false) {}


MainFrame::MainFrame(QWidget* parent) :
		QWidget(parent), __ui(new Ui::MainFrame),
		__parameterMgr(new ParameterManager), __env(new Environment),
		__cache(new Cache) {

	setObjectName("SDP-MainFrame");

	SDPASSERT(Logger::instancePtr());
	Logger* log = Logger::instancePtr();

	__ui->setupUi(this);

	FancyPanel* ePanel = new FancyPanel(this);
	QVBoxLayout* eLayout = new QVBoxLayout(__ui->widgetTopLeft);
	eLayout->setMargin(0);
	eLayout->setSpacing(0);
	eLayout->addWidget(ePanel);

	FancyButton* logo = new FancyButton(QPixmap(":images/Logo_short_RGB_72_OVS_horiz.png"), ePanel);
	logo->setCheckable(false);
	logo->setButtonCapability(FancyButton::Dead);
	ePanel->addFancyButton(logo);

	__statusBar = new FancyPanel(__ui->widgetStatusBar, FancyPanel::Horizontal, 30);
//	__statusBar->setCustomTheme(QColor(130, 130, 130), QColor(130, 130, 130), QColor(49, 49, 49));
//	__statusBar->setCustomTheme(QColor(31, 31, 31), QColor(58, 58, 58), QColor(33, 33, 33));
	QVBoxLayout* stLayout = new QVBoxLayout(__ui->widgetStatusBar);
	stLayout->setMargin(0);
	stLayout->addWidget(__statusBar);

	__statusLabel = new FancyLabel(__statusBar);
//	__statusLabel->setStyleSheet("background-color: rgb(142, 142, 142);");
	QSpacerItem* spacer = new QSpacerItem(40, 30, QSizePolicy::Expanding, QSizePolicy::Minimum);
	QHBoxLayout* hlayout = new QHBoxLayout(__statusBar);
//	hlayout->setMargin(4);
	hlayout->addWidget(__statusLabel);
	hlayout->addSpacerItem(spacer);
//	__statusLabel->hide();
	__inAnim = new QPropertyAnimation(__statusLabel, "geometry");
	__outAnim = new QPropertyAnimation(__statusLabel, "geometry");
	__bgAnim = new QPropertyAnimation(__statusLabel, "backgroundColor");

	FancyPanel* panel = new FancyPanel(__ui->scrollAreaPanel);
	__configButton = new FancyButton(QPixmap(":images/applications-system.png"), tr("Settings"), panel);
	__recentButton = new FancyButton(QPixmap(":images/recent.png"), tr("History"), panel);
	__activityButton = new FancyButton(QPixmap(":images/gnome_system_monitor.png"), tr("Activity"), panel);
	__triggerButton = new FancyButton(QPixmap(":images/seismograph.png"), "Trigger", panel);
	__dispatchButton = new FancyButton(QPixmap(":images/import-export-icon.png"), tr("Dispatch"), panel);
	__detectionButton = new FancyButton(QPixmap(":images/search.png"), tr("Detect"), panel);
	__helpButton = new FancyButton(QPixmap(":images/help.png"), tr("Help"), panel);

	FancyButton* separator = new FancyButton(panel);
	separator->setMinimumHeight(10);
	separator->setMaximumHeight(10);
	separator->setCursor(Qt::ArrowCursor);

	FancyButton* separator2 = new FancyButton(panel);
	separator2->setMinimumHeight(10);
	separator2->setMaximumHeight(10);
	separator2->setCursor(Qt::ArrowCursor);

	FancyButton* separator3 = new FancyButton(panel);
	separator3->setMinimumHeight(10);
	separator3->setMaximumHeight(10);
	separator3->setCursor(Qt::ArrowCursor);

	__configButton->setToolTip(tr("Configure application"));
	__recentButton->setToolTip(tr("Go thru application's archived objects"));
	__activityButton->setToolTip(tr("Monitor application current activities"));
	__triggerButton->setToolTip(tr("Manage detection triggers"));
	__dispatchButton->setToolTip(tr("Dispatch miniSEED data into a SeisComP Data Structure"));
	__detectionButton->setToolTip(tr("Configure an Arclink or datafile detection script"));
	__helpButton->setToolTip(tr("Go thru application's help"));

	connect(__configButton, SIGNAL(clicked()), this, SLOT(panelButtonClicked()));
	connect(__recentButton, SIGNAL(clicked()), this, SLOT(panelButtonClicked()));
	connect(__activityButton, SIGNAL(clicked()), this, SLOT(panelButtonClicked()));
	connect(__triggerButton, SIGNAL(clicked()), this, SLOT(panelButtonClicked()));
	connect(__dispatchButton, SIGNAL(clicked()), this, SLOT(panelButtonClicked()));
	connect(__detectionButton, SIGNAL(clicked()), this, SLOT(panelButtonClicked()));
	connect(__helpButton, SIGNAL(clicked()), this, SLOT(panelButtonClicked()));

	__buttons.insert(pSettings, __configButton);
	__buttons.insert(pRecent, __recentButton);
	__buttons.insert(pActivity, __activityButton);
	__buttons.insert(pTrigger, __triggerButton);
	__buttons.insert(pDispatch, __dispatchButton);
	__buttons.insert(pDetection, __detectionButton);
	__buttons.insert(pHelp, __helpButton);

	panel->addFancyButton(__configButton);
	panel->addFancyButton(separator);
	panel->addFancyButton(__recentButton);
	panel->addFancyButton(__activityButton);
	panel->addFancyButton(__triggerButton);
	panel->addFancyButton(separator2);
	panel->addFancyButton(__dispatchButton);
	panel->addFancyButton(__detectionButton);
	panel->addFancyButton(separator3);
	panel->addFancyButton(__helpButton);

	__ui->scrollAreaPanel->setWidget(panel);

	//! Now that buttons are initialized and ready, load the configuration
	//! interface so that panels leaning on it could get information from
	//! the user run configuration...

	//! Initialize the configuration reader by using default filepath
	QString configFile;
	const QString userConfigFile = __env->userDir() + QDir::separator() + "sdp.cfg";
	const QString defaultConfigFile = __env->configDir() + QDir::separator() + "sdp.cfg";
	if ( Utils::fileExists(userConfigFile) )
		configFile = userConfigFile;
	else if ( Utils::fileExists(defaultConfigFile) )
	    configFile = defaultConfigFile;

	if ( !configFile.isEmpty() ) {
		log->addMessage(Logger::INFO, __func__, "Loading application configuration from " + configFile, false);
		__mainConfig = new Config(configFile);
		newStatusMessage("Loaded configuration file " + configFile + ".");
		__parameterMgr->registerParameter("PROJECT_CONFIG_FILE", QVariant::fromValue(configFile));
	}
	else {
		__mainConfig = new Config;
		newStatusMessage("Application started without pre-configured presets...");
	}

	//! Initialize the database by using default filepath
	//! INFO: If the file is not found, the database manager will create tables
	//!       and consequently use'em...
	QString dbFile;
	const QString userDbFile = __env->userShareDir() + QDir::separator() + "sdp.sqlite";
	const QString defaultDbFile = __env->shareDir() + QDir::separator() + "sdp.sqlite";
	if ( Utils::fileExists(userDbFile) )
		dbFile = userDbFile;
	else
		dbFile = defaultDbFile;

	__dbMgr.reset(new DatabaseManager(dbFile));
	__parameterMgr->registerParameter("PROJECT_DB_FILE", QVariant::fromValue(dbFile));

	__hdr = new FancyHeaderFrame(this);

	QVBoxLayout* hdrLayout = new QVBoxLayout(__ui->widgetHeader);
	hdrLayout->setMargin(0);
	hdrLayout->setSpacing(0);
	hdrLayout->addWidget(__hdr);

	//! Initiate panels in an ordered way : some panels lean on each others
	//! variables. e.g.: the SettingsPanel should be first, it offers
	//! registered values of global dirs and other stuff...
	__config = new SettingsPanel(this);
	__recent = new RecentPanel(this);
	__activity = new ActivityPanel(this);
	__trigger = new TriggerPanel(this);
	__detection = new DetectionPanel(this);
	__dispatch = new DispatchPanel(this);
	__help = new HelpPanel(this);

	__panels.insert(pSettings, __config);
	__panels.insert(pRecent, __recent);
	__panels.insert(pActivity, __activity);
	__panels.insert(pTrigger, __trigger);
	__panels.insert(pDispatch, __dispatch);
	__panels.insert(pDetection, __detection);
	__panels.insert(pHelp, __help);

	QHBoxLayout* mainLayout = new QHBoxLayout(__ui->frameContainer);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(__config);
	mainLayout->addWidget(__recent);
	mainLayout->addWidget(__activity);
	mainLayout->addWidget(__trigger);
	mainLayout->addWidget(__dispatch);
	mainLayout->addWidget(__detection);
	mainLayout->addWidget(__help);

	connect(__activity, SIGNAL(detectionJobTerminated(DetectionJob*)), __trigger, SLOT(createTrigger(DetectionJob*)));
	connect(__activity, SIGNAL(editDetectionJob(DetectionJob*)), __detection, SLOT(load(DetectionJob*)));
	connect(__activity, SIGNAL(editDispatchJob(DispatchJob*)), __dispatch, SLOT(load(DispatchJob*)));
	connect(__trigger, SIGNAL(triggerStatusModified(int, QList<QString>)), __recent, SLOT(triggerStatusModified(int, QList<QString>)));
	connect(__trigger, SIGNAL(newDetectionTriggers(QString, QList<QString>)), __recent, SLOT(addDetectionTriggers(QString, QList<QString>)));

	panel->activateFancyButton(__recentButton);
	setActivePanel(pRecent);
}


MainFrame::~MainFrame() {

	SDPASSERT(Logger::instancePtr());

	Progress prog(this,Qt::FramelessWindowHint);
	//! Save objects generated from this instance inside the database, starting
	//! with jobs because other objects lean onto them.
	Logger::instancePtr()->addMessage(Logger::INFO, __func__, "Waiting for panels to commit objects into database.");
	prog.show("Committing jobs into database");
	__activity->commitJobsIntoDB();
	prog.show("Committing triggers into database");
	__trigger->commitTriggersIntoDB();
	prog.hide();

	Logger::instancePtr()->addMessage(Logger::INFO, __func__, "Destroying main user interface.");

	delete __mainConfig;
}


void MainFrame::setActivePanel(Panel p) {

	//! Hide all panels first, this will prevent two of them to be visible
	//! at the same time, because the iteration may fall on the panel to show
	//! before hiding the visible one and forcing the window to resize itself
	//! to fit the visible panels together... and that's messy as hell..
	//! TODO: Investigate how slow systems react to this hack, see if Qt
	//! repainting engine is handling this properly...
	for (PanelList::iterator it = __panels.begin();
	        it != __panels.end(); ++it)
		(*it)->setVisible(false);

	for (PanelList::iterator it = __panels.begin();
	        it != __panels.end(); ++it) {
		if ( it.key() == p ) {
			(*it)->setVisible(true);
			__hdr->setTitle((*it)->header());
			__hdr->setDescription((*it)->description());
		}
	}
}


bool MainFrame::readConfiguration() {
	SDPASSERT(__mainConfig);
	return true;
}


bool MainFrame::clearForExit() {

	QString msg;
	if ( __activity->status() != ActivityPanel::Idle )
	    msg += "Activity monitor seems to be running.<br/>";
	if ( __activity->remainingJobs() != 0 )
	    msg += QString("%1 job(s) still active.")
	        .arg(QString::number(__activity->remainingJobs()));

	if ( !msg.isEmpty() ) {

		if ( QMessageBox::question(this, tr("Information"),
		    QString("%1<br/>Are you sure ?").arg(msg),
		    QMessageBox::Cancel | QMessageBox::Yes,
		    QMessageBox::Yes) != QMessageBox::Yes )
			return false;
		else
			return true;
	}

	return true;
}


void MainFrame::loadInventory() {

	SDPASSERT(Logger::instancePtr());
	Logger* log = Logger::instancePtr();

	if ( !__dbMgr->isOpened() ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to access to the database.");
		return;
	}

	//! Dispatch non-executed detections in 'ActivityPanel' and executed ones
	//! in 'RecentPanel'.
	DatabaseManager::DetectionList l = __dbMgr->detections();
	log->addMessage(Logger::INFO, __func__, QString("Found %1 previous detection job(s) inside database.")
	    .arg(QString::number(l.size())), true);
	for (int i = 0; i < l.size(); ++i) {
		if ( l.at(i)->runStartTime().isNull() && l.at(i)->runEndTime().isNull() )
			__activity->addJob(l.at(i));
		else if ( l.at(i)->runStartTime().isValid() && l.at(i)->runEndTime().isValid() )
		    __recent->loadJob(l.at(i));
	}

	//! Dispatch non-executed dispatch runs in 'ActivityPanel' and executed ones
	//! in 'RecentPanel'.
	DatabaseManager::DispatchList l2 = __dbMgr->dispatchs();
	log->addMessage(Logger::INFO, __func__, QString("Found %1 previous dispatch job(s) inside database.")
	    .arg(QString::number(l2.size())), true);
	for (int i = 0; i < l2.size(); ++i) {
		if ( l2.at(i)->runStartTime().isNull() && l2.at(i)->runEndTime().isNull() )
			__activity->addJob(l2.at(i));
		else if ( l2.at(i)->runStartTime().isValid() && l2.at(i)->runEndTime().isValid() )
		    __recent->loadJob(l2.at(i));
	}

	//! Fetch Triggers that are waiting for validation
	DatabaseManager::TriggerList tl = __dbMgr->unreviewedTriggers();
	log->addMessage(Logger::DEBUG, __func__, QString("Found %1 un-reviewed trigger(s) inside database.")
	    .arg(QString::number(tl.size())), true);
	for (int i = 0; i < tl.size(); ++i)
		__trigger->loadTrigger(tl.at(i));
}


void MainFrame::newJobToManage() {
	SDPASSERT(__activityButton);
	__activityButton->animate(100);
}


void MainFrame::newTriggerToInspect() {
	SDPASSERT(__triggerButton);
	__triggerButton->animate(100);
}


void MainFrame::newStatusMessage(const QString& msg) {

	SDPASSERT(__statusLabel);

	QFontMetrics m(__statusLabel->font());

	QRect start = __statusLabel->rect();
	start.setTopLeft(QPoint(4, 4));
	start.setWidth(0);
	start.setHeight(m.height() * 1.5);

	QRect end = __statusLabel->rect();
	end.setTopLeft(QPoint(4, 4));
	end.setWidth(m.width(msg) * 1.2);
	end.setHeight(m.height() * 1.5);

	__statusLabel->show();

	__inAnim->setDuration(500);
	__inAnim->setStartValue(start);
	__inAnim->setEndValue(end);
	__statusLabel->setText(msg);
	__inAnim->start();

	__bgAnim->setDuration(2000);
	__bgAnim->setStartValue(QColor(142, 142, 142));
	__bgAnim->setEndValue(QColor(205, 205, 205));
//	__bgAnim->start();

	QTimer::singleShot(4000, this, SLOT(resetStatusMessage()));
}


void MainFrame::resetStatusMessage() {

	SDPASSERT(__statusLabel);

	QFontMetrics m(__statusLabel->font());

	QRect start = __statusLabel->rect();
	start.setTopLeft(QPoint(4, 4));
	start.setWidth(m.width(__statusLabel->text()) * 1.2);
	start.setHeight(m.height() * 1.5);

	QRect end = __statusLabel->rect();
	end.setTopLeft(QPoint(4, 4));
	end.setWidth(0);
	end.setHeight(m.height() * 1.5);

	__outAnim->setDuration(500);
	__outAnim->setStartValue(start);
	__outAnim->setEndValue(end);
	__outAnim->start();

	__statusLabel->hide();
}


void MainFrame::panelButtonClicked() {

	QObject* sender = QObject::sender();
	SDPASSERT(sender);

	for (ButtonList::iterator it = __buttons.begin();
	        it != __buttons.end(); ++it) {
		if ( (*it) == sender )
		    setActivePanel(it.key());
	}
}


} // namespace Qt4
} // namespace SDP

