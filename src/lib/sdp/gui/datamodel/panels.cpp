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


#include <sdp/gui/datamodel/panels.h>

#include "../api.h"
#include <sdp/gui/datamodel/config.h>
#include <sdp/gui/datamodel/system.h>
#include <sdp/gui/datamodel/archiveobjects.h>
#include <sdp/gui/datamodel/job.h>
#include <sdp/gui/datamodel/trigger.h>
#include <sdp/gui/datamodel/cache.h>
#include <sdp/gui/datamodel/macros.h>
#include <sdp/gui/datamodel/progress.h>
#include <sdp/gui/datamodel/mainframe.h>
#include <sdp/gui/datamodel/parametermanager.h>
#include <sdp/gui/datamodel/databasemanager.h>
#include <sdp/gui/datamodel/fancywidgets.h>
#include <sdp/gui/datamodel/bashhighlighter.h>
#include <sdp/gui/datamodel/syntaxhighlighter.h>
#include <sdp/gui/datamodel/logger.h>
#include <sdp/gui/datamodel/utils.h>
#include <sdp/gui/datamodel/qroundprogressbar/QRoundProgressBar.h>
#include <sdp/gui/datamodel/subpanels.h>

#include "ui_entity.h"
#include "ui_enqueue.h"
#include "ui_commit.h"
#include "ui_settingspanel.h"
#include "ui_recentpanel.h"
#include "ui_activitypanel.h"
#include "ui_detectionpanel.h"
#include "ui_triggerpanel.h"
#include "ui_dispatchpanel.h"

#include <QtGui>
#include <QtConcurrentRun>



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

/** @brief Table headers enum */
enum RecentTableHeaders {
	thCTIME, thTYPE, thRUNSTART, thRUNEND, thRUNSTATUS, thINFORMATION,
	thCOMMENT, thID, thSIZE
};

/**
 * @brief Table headers enum values
 *        The user has to make sure that those values matches (in order) the
 *        defined StationTableHeaders enum so it can properly be setup.
 *        When the name of an element has to be retrieved, the user needs only
 *        instantiate the vector containing the names:
 *        @code
 *                std::string elementName = RecentTableHeaders[thCTIME];
 *                or
 *                std::string elementName = RecentHeadersString[thCTIME];
 *        @encode
 */
const char* RecentTableHeaders[] = { "Creation time", "Type", "Run start",
    "Run end", "Run status", "Information", "Comment", "ID", "Size" };
const std::vector<const char*> RecentHeadersString(RecentTableHeaders,
    RecentTableHeaders + sizeof(RecentTableHeaders) / sizeof(RecentTableHeaders[0]));

/**
 * @brief  Retrieves header position from header vector.
 * @param  e the STableHeader enum value
 * @return The header's position
 * @note   Valid header position are okay above zero, fake underneath.
 */
int getHeaderPosition(const enum RecentTableHeaders& e) {
	char* str;
	for (size_t i = 0; i < RecentHeadersString.size(); ++i) {
		str = (char*) RecentHeadersString.at(i);
		if ( str == RecentTableHeaders[e] )
		    return (int) i;
	}
	return -1;
}



enum TriggerTableHeaders {
	tCTIME, tTTIME, tSTATUS, tSITES, tID
};
const char* TriggerTableHeaders[] = { "Creation time", "Trigger time", "Status", "Sites", "ID" };
const std::vector<const char*> TriggerHeadersString(TriggerTableHeaders,
    TriggerTableHeaders + sizeof(TriggerTableHeaders) / sizeof(TriggerTableHeaders[0]));
int getHeaderPosition(const enum TriggerTableHeaders& e) {
	char* str;
	for (size_t i = 0; i < TriggerHeadersString.size(); ++i) {
		str = (char*) TriggerHeadersString.at(i);
		if ( str == TriggerTableHeaders[e] )
		    return (int) i;
	}
	return -1;
}

enum ActivityTableHeaders {
	aCTIME, aTYPE, aINFO, aCOMMENT, aSTATUS, aID
};
const char* ActivityTableHeaders[] = { "Creation time", "Type", "Information", "Comment", "Status", "ID" };
const std::vector<const char*> ActivityHeadersString(ActivityTableHeaders,
    ActivityTableHeaders + sizeof(ActivityTableHeaders) / sizeof(ActivityTableHeaders[0]));
int getHeaderPosition(const enum ActivityTableHeaders& e) {
	char* str;
	for (size_t i = 0; i < ActivityHeadersString.size(); ++i) {
		str = (char*) ActivityHeadersString.at(i);
		if ( str == ActivityTableHeaders[e] )
		    return (int) i;
	}
	return -1;
}



//! INFO: Define some output constants used for script generation
static QString const ENDL = "\n";
static QString const TAB = "    ";
static QString const xmlTAB = "  ";


/**
 * @brief Translates generic return codes into string (text) equivalent.
 * @param retCode return code value
 * @return String translation of code
 */
QString translateExitCode(const int& retCode) {

	QString str = QString("[%1] ").arg(retCode);
	switch ( retCode ) {
		case -1:
			str.append("Failed to run");
			break;
		case 0:
			str.append("Exited normally");
			break;
		case 1:
			str.append("Error");
			break;
		default:
			str.append("Unspecified error");
			break;
	}
	return str;
}


class AnimInfoWidget : public QWidget {
	public:
		explicit AnimInfoWidget(QWidget* parent, const QString& mv,
		                        const QString& text, const QFont& f) :
				QWidget(parent) {

			QMovie* movie = new QMovie(mv, QByteArray(), this);
			movie->setScaledSize(QSize(20, 10));
			QLabel* label = new QLabel(this);
			label->setMovie(movie);
			movie->start();
			txt = new QLabel(text, this);
			txt->setFont(f);

			QHBoxLayout* l = new QHBoxLayout(this);
			l->setAlignment(Qt::AlignCenter);
//			l->setSpacing(0);
			l->setMargin(0);
			l->addWidget(label);
			l->addWidget(txt);
		}
		~AnimInfoWidget() {}
		void setInfoText(const QString& str) {
			txt->setText(str);
		}
	private:
		QLabel* txt;
};

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



namespace SDP {
namespace Qt4 {


class TableItem : public QTableWidgetItem {
	public:
		explicit TableItem(const QString& s, void* o) :
				QTableWidgetItem(s), object(o) {}
		void* object;
};


/**
 * @brief Removes row(s) from QTableWidget in an ordered way...
 * @param table QTableWidget pointer
 * @param list Preselected list of TableItem pointers
 */
void removeTableRow(QTableWidget* table, const QList<TableItem*>& list) {

	QMap<int, int> rowsMap;
	for (int i = 0; i < list.count(); i++)
		rowsMap[list.at(i)->row()] = -1; //garbage value

	QList<int> rowsList = rowsMap.uniqueKeys();
	qSort(rowsList);

	//! Now go through your table and delete rows in descending order
	//! as content would shift up and hence cannot do it in ascending
	//! order with ease.
	for (int i = rowsList.count() - 1; i >= 0; i--)
		table->removeRow(rowsList.at(i));
}


SettingsPanel::SettingsPanel(QWidget* parent) :
		PanelWidget(parent), __ui(new Ui::SettingsPanel) {

	__ui->setupUi(this);

	setHeader(tr("Settings"));
	setDescription(tr("Configure application's variables."));

	QGradientStops gradientPoints;
	gradientPoints << QGradientStop(0, Qt::green) << QGradientStop(0.5, Qt::yellow)
	    << QGradientStop(1, Qt::red);

	QPalette p1;
	p1.setBrush(QPalette::Base, QColor(213, 208, 206, 255));
	p1.setColor(QPalette::Text, QColor(31, 31, 31, 255));
	p1.setColor(QPalette::Shadow, Qt::green);

	QVBoxLayout* l1 = new QVBoxLayout(__ui->frameDiskspace);
	l1->setMargin(0);
	__diskSpace = new QRoundProgressBar(__ui->frameDiskspace);
	__diskSpace->setPalette(p1);
	__diskSpace->setNullPosition(QRoundProgressBar::PositionRight);
	__diskSpace->setBarStyle(QRoundProgressBar::StyleDonut);
	__diskSpace->setDataColors(gradientPoints);
	l1->addWidget(__diskSpace);
	__diskSpace->setValue(0);

	QVBoxLayout* l2 = new QVBoxLayout(__ui->frameRunDirSize);
	l2->setMargin(0);
	__runDirSize = new QRoundProgressBar(__ui->frameRunDirSize);
	__runDirSize->setPalette(p1);
	__runDirSize->setNullPosition(QRoundProgressBar::PositionRight);
	__runDirSize->setBarStyle(QRoundProgressBar::StyleDonut);
	__runDirSize->setDataColors(gradientPoints);
	l2->addWidget(__runDirSize);
	__runDirSize->setValue(0);

	init();

	//! Load configuration... Variables from this panel will be replaced by
	//! the ones specified by the user from configuration file.
	loadConfiguration();

	//! Check diskfree and size of run dir each 10sec
	__timer.start(10000, this);
}


SettingsPanel::~SettingsPanel() {
	__timer.stop();
}


void SettingsPanel::init() {

	SDPASSERT(Environment::instancePtr());
	SDPASSERT(Logger::instancePtr());
	SDPASSERT(ParameterManager::instancePtr());

	Environment* e = Environment::instancePtr();
	Logger* log = Logger::instancePtr();
	ParameterManager* pm = ParameterManager::instancePtr();

	QStringList headers = QStringList() << "Name" << "Value";

	__ui->tableWidgetVariables->setColumnCount(headers.size());
	__ui->tableWidgetVariables->setHorizontalHeaderLabels(headers);
	__ui->tableWidgetVariables->setStyleSheet("QTableView {selection-background-color: rgb(0,0,0,100);}");

#ifdef Q_OS_MAC
	QFont f(__ui->tableWidgetVariables->font());
	f.setPointSize(11);
	__ui->tableWidgetVariables->setFont(f);
#endif

	//! Global variables used throughout the application
	__vars << EntityVariable("ROOT_DIR", EntityVariable::evPATH, e->installDir(), "", tr("The root directory of the application"));
	__vars << EntityVariable("BIN_DIR", EntityVariable::evPATH, e->binDir(), "", tr("The directory in which binaries are installed"));
	__vars << EntityVariable("CONFIG_DIR", EntityVariable::evPATH, e->configDir(), "", tr("The directory in which app's configuration files are stored"));
	__vars << EntityVariable("DATA_DIR", EntityVariable::evPATH, e->shareDir(), "", tr("The directory in which sessions data will be stored"));
	__vars << EntityVariable("LOG_DIR", EntityVariable::evPATH,
	    QString("%1%2%3").arg(e->shareDir()).arg(QDir::separator()).arg("log"),
	    "", tr("The directory in which the logs will be created"));
	__vars << EntityVariable("RUN_DIR", EntityVariable::evPATH,
	    QString("%1%2%3").arg(e->shareDir()).arg(QDir::separator()).arg("runs"),
	    "", tr("The directory in which individual runs will be stored. Any trigger "
		    "resulting from a run will be stored first inside its sub-folder "
		    "and afterwards copied into the TRIGGER_DIR"));
	__vars << EntityVariable("TRIGGER_DIR", EntityVariable::evPATH,
	    QString("%1%2%3").arg(e->shareDir()).arg(QDir::separator()).arg("triggers"),
	    "settings.dir.triggers",
	    tr("The directory in which triggers will be archived"));
	__vars << EntityVariable("XML_ARCHIVE_DIR", EntityVariable::evPATH,
	    QString("%1%2%3").arg(e->shareDir()).arg(QDir::separator()).arg("archive"),
	    "settings.dir.xmlarchive",
	    tr("The directory in which XML files for scdispatch will be stored"));
	__vars << EntityVariable("ENV_BIN", EntityVariable::evBIN, "Unknown env binary", "settings.bin.env",
	    tr("The env bin location which is mandatory to assign the proper "
		    "environment settings for scripts"));
	__vars << EntityVariable("BASH_BIN", EntityVariable::evBIN, "Unknown bash binary", "settings.bin.bash",
	    tr("The bash bin location which is mandatory to perform dispatch "
		    "operation with miniSEED files by using msrouter or msmod"));
	__vars << EntityVariable("PYTHON_BIN", EntityVariable::evBIN, "Unknown python binary", "settings.bin.python",
	    tr("The python bin location which is mandatory to perform stream analysis "
		    "with ObsPy library"));
	__vars << EntityVariable("SEISCOMP_BIN", EntityVariable::evBIN, "Unknown seiscomp binary", "settings.bin.seiscomp",
	    tr("The SeisComP bin location which is mandatory for importing triggers "
		    "into a SeisComP3 installation"));
	__vars << EntityVariable("MSROUTER_BIN", EntityVariable::evBIN,
	    QString("%1%2%3").arg(e->binDir()).arg(QDir::separator()).arg("msrouter"),
	    "settings.bin.msrouter",
	    tr("The msrouter bin location which is mandatory for performing "
		    "dispatch operations"));
	__vars << EntityVariable("MSMOD_BIN", EntityVariable::evBIN,
	    QString("%1%2%3").arg(e->binDir()).arg(QDir::separator()).arg("msmod"),
	    "settings.bin.msmod",
	    tr("The msmod bin location which is mandatory for performing "
		    "customized dispatch operations (retagging data headers)"));
	__vars << EntityVariable("RDSEED_BIN", EntityVariable::evBIN, "Unknown rdseed binary",
	    "settings.bin.rdseed",
	    tr("The rdseed bin which is recommended when the user wants to use a "
		    "binary dataless for loading stations in detection's inventory."));
	__vars << EntityVariable("TRIGGER_FILE", EntityVariable::evSTRING,
	    QString("@JOB_RUN_DIR@%1triggers.txt").arg(QDir::separator()), "",
	    tr("The file in which detection run trigger(s) will be stored. "
		    "(@JOB_RUN_DIR@ will be replaced at runtime with the run directory "
		    "of the detection of interest)"));
	__vars << EntityVariable("TRIGGER_INFO_FILE", EntityVariable::evSTRING,
	    QString("@JOB_RUN_DIR@%1trigger-@TRIGGER_DATETIME@.txt").arg(QDir::separator()),
	    "", tr("The file in which detection run trigger(s) information will be stored. "
		    "(@JOB_RUN_DIR@ and @TRIGGER_DATETIME@ will be replaced at runtime "
		    "with run director and trigger time of the detection of interest)"));
	__vars << EntityVariable("LOC_METHOD_ID", EntityVariable::evSTRING, "sdp", "settings.loc.methodID",
	    tr("Location method ID that will be added inside SC3ML file"));
	__vars << EntityVariable("LOC_EARTH_MODEL_ID", EntityVariable::evSTRING, "@EVENT_SCODE@", "",
	    tr("Location earth model ID that will be added inside SC3ML file"));
	__vars << EntityVariable("AGENCY", EntityVariable::evSTRING, "Unknown", "settings.loc.agency", tr("The name of your agency"));
	__vars << EntityVariable("AUTHOR", EntityVariable::evSTRING, "Unknown", "settings.loc.author", tr("The author of the detection(s)"));
	__vars << EntityVariable("SC_DISPATCH_EXTRA_ARG", EntityVariable::evSTRING, "", "settings.scdispatchArguments", tr("Extra arguments to pass to scdispatch when importing triggers"));
	__vars << EntityVariable("TRIGGER_PREFIX", EntityVariable::evSTRING, "trigger-", "settings.triggerPrefix", tr("The default trigger prefix when stored in run folder"));
	__vars << EntityVariable("ARCLINK_USER", EntityVariable::evSTRING, "script@sdp", "settings.arclink.user", tr("The arclink user"));
	__vars << EntityVariable("ARCLINK_PASSWORD", EntityVariable::evSTRING, "", "settings.arclink.password", tr("The arclink user's password"));

	//! Populate the table with variables and values, also, register them
	//! in the mean time...
	__ui->tableWidgetVariables->setRowCount(__vars.size());
	for (int i = 0; i < __vars.size(); ++i) {

		QString tt = QString("<table width=300>");
		switch ( __vars.at(i).type ) {
			case EntityVariable::evSTRING:
				tt += QString("<tr><td><b>Type</b>:</td><td>text</td></tr>");
				break;
			case EntityVariable::evBIN:
				tt += QString("<tr><td><b>Type</b>:</td><td>filepath</td></tr>");
				break;
			case EntityVariable::evPATH:
				tt += QString("<tr><td><b>Type</b>:</td><td>path</td></tr>");
				break;
		}
		if ( __vars.at(i).config.isEmpty() )
			tt += QString("<tr><td><b>Config. arg.</b>:</td><td><i>not defined</i></td></tr>");
		else
			tt += QString("<tr><td><b>Config. arg.</b>:</td><td>%1</td></tr>").arg(__vars.at(i).config);
		tt += QString("<tr><td><b>Default</b>:</td><td>%1</td></tr></table>").arg(__vars.at(i).value);
		tt += QString("<tr><td><b>Information</b>:</td><td>%1</td></tr></table>").arg(__vars.at(i).tooltip);

		QTableWidgetItem* var = new QTableWidgetItem(__vars.at(i).name);
		QTableWidgetItem* val = new QTableWidgetItem(__vars.at(i).value);
		pm->registerParameter(__vars.at(i).name, __vars.at(i).value);
		var->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		var->setToolTip(tt);
		val->setToolTip(tt);
		__ui->tableWidgetVariables->setItem(i, 0, var);
		__ui->tableWidgetVariables->setItem(i, 1, val);
	}

	__ui->tableWidgetVariables->horizontalHeader()->setStretchLastSection(true);
	__ui->tableWidgetVariables->resizeColumnsToContents();

	pm->registerParameter("Config-MaxThreads", QVariant::fromValue(__ui->spinBoxThreads->value()));
	pm->registerParameter("Config-DefaultLatitude", QVariant::fromValue(__ui->doubleSpinBoxLatitude->value()));
	pm->registerParameter("Config-DefaultLongitude", QVariant::fromValue(__ui->doubleSpinBoxLongitude->value()));
	pm->registerParameter("Config-Filter-Enabled", QVariant::fromValue(__ui->checkBoxFilter->isChecked()));
	pm->registerParameter("Config-Filter-Name", QVariant::fromValue(__ui->comboBoxFilter->currentText()));
	pm->registerParameter("Config-Filter-FreqMin", QVariant::fromValue(__ui->doubleSpinBoxFilterMin->value()));
	pm->registerParameter("Config-Filter-FreqMax", QVariant::fromValue(__ui->doubleSpinBoxFilterMax->value()));

	pm->registerParameter("Config-DiskspaceOK", QVariant::fromValue(true));
	pm->registerParameter("Config-RunDirSizeOK", QVariant::fromValue(true));

	connect(__ui->spinBoxThreads, SIGNAL(editingFinished()), this, SLOT(saveSettings()));
	connect(__ui->doubleSpinBoxLatitude, SIGNAL(editingFinished()), this, SLOT(saveSettings()));
	connect(__ui->doubleSpinBoxLongitude, SIGNAL(editingFinished()), this, SLOT(saveSettings()));
	connect(__ui->tableWidgetVariables, SIGNAL(itemChanged(QTableWidgetItem*)),
	    this, SLOT(tableSettingsEdited(QTableWidgetItem*)));
	connect(__ui->checkBoxFilter, SIGNAL(toggled(bool)), __ui->comboBoxFilter, SLOT(setEnabled(bool)));
	connect(__ui->checkBoxFilter, SIGNAL(toggled(bool)), __ui->doubleSpinBoxFilterMin, SLOT(setEnabled(bool)));
	connect(__ui->checkBoxFilter, SIGNAL(toggled(bool)), __ui->doubleSpinBoxFilterMax, SLOT(setEnabled(bool)));
	connect(__ui->checkBoxFilter, SIGNAL(stateChanged(int)), this, SLOT(buttonToggled(int)));
	connect(__ui->doubleSpinBoxFilterMin, SIGNAL(editingFinished()), this, SLOT(saveSettings()));
	connect(__ui->doubleSpinBoxFilterMax, SIGNAL(editingFinished()), this, SLOT(saveSettings()));
	connect(__ui->comboBoxFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(buttonToggled(int)));

	//! Check base binaries from the local system
	//! TODO: maybe add a version check for each binary... food for thoughts sherlock...
	__procs << ProcessCheck("envProc", "ENV_BIN", "which env");
	__procs << ProcessCheck("bashProc", "BASH_BIN", "which bash");
	__procs << ProcessCheck("pythonProc", "PYTHON_BIN", "which python");
	__procs << ProcessCheck("seisProc", "SEISCOMP_BIN", "which seiscomp");
	__procs << ProcessCheck("rdseedProc", "RDSEED_BIN", "which rdseed");

	for (PCList::const_iterator it = __procs.constBegin();
	        it != __procs.constEnd(); ++it) {

		QProcess* proc = new QProcess(this);
		proc->setObjectName((*it).name);
		connect(proc, SIGNAL(finished(int)), this, SLOT(procTerminated(int)));

		proc->start((*it).command);
		if ( !proc->waitForStarted() )
		    log->addMessage(Logger::CRITICAL, __func__, QString("Failed to start process %1 [%2]")
		        .arg((*it).command).arg(proc->errorString()));
	}
}


void SettingsPanel::timerEvent(QTimerEvent*) {

	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(Cache::instancePtr());
	ParameterManager* pm = ParameterManager::instancePtr();
	Cache* cache = Cache::instancePtr();

	quint64 runDirSize = Utils::dirSize(pm->parameter("RUN_DIR").toString());
	quint64 dirSizeMB = runDirSize / (1024 * 1024);

	__ui->labelCurrentRunDirSize->setText(
	    (dirSizeMB > 1024) ? QString("%1GB used").arg(dirSizeMB / 1024)
	        : QString("%1MB used").arg(dirSizeMB));
	__runDirSize->setValue(static_cast<int>(Utils::percentageOfSomething(
	    __ui->spinBoxMaxRundirSpace->value() * 1024, dirSizeMB)));

	quint64 diskfreeB = Utils::userAvailableFreeSpace();
	quint64 diskfreeMB = diskfreeB / (1024 * 1024);

	__ui->labelAvailableDiskspace->setText(
	    (diskfreeMB > 1024) ? QString("%1GB available").arg(diskfreeMB / 1024)
	        : QString("%1MB available").arg(diskfreeMB));
	quint64 minDiskfreeMB = __ui->spinBoxMinDiskspace->value() * 1024;
	if ( diskfreeMB > minDiskfreeMB )
		__diskSpace->setValue(0);
	else
		__diskSpace->setValue(static_cast<int>(Utils::percentageOfSomething(
		    minDiskfreeMB, diskfreeMB)));

	quint32 cacheB = cache->byteSize();
	quint32 cacheKB = cache->byteSize() / 1024;
	quint32 cacheMB = cache->byteSize() / (1024 * 1024);
	QString cacheStr;
	if ( cacheB < 1024 )
	    cacheStr = QString::number(cacheB) + "B";
	if ( cacheKB < 1024 && cacheB > 1024 )
	    cacheStr = QString::number(cacheKB) + "KB";
	if ( cacheMB < 1024 && cacheKB > 1024 )
	    cacheStr = QString::number(cacheMB) + "MB";
	__ui->labelCacheCount->setText(QString::number(cache->count()));
	__ui->labelCacheSize->setText(cacheStr);

	pm->setParameter("Config-DiskspaceOK", QVariant::fromValue(diskfreeMB > minDiskfreeMB));
	pm->setParameter("Config-RunDirSizeOK", QVariant::fromValue(
	    static_cast<quint64>(__ui->spinBoxMaxRundirSpace->value()) * 1024 > dirSizeMB));
}


bool SettingsPanel::saveParameters() {

	SDPASSERT(ParameterManager::instancePtr());

	ParameterManager* pm = ParameterManager::instancePtr();

	for (int i = 0; i < __ui->tableWidgetVariables->rowCount(); ++i)
		pm->setParameter(__ui->tableWidgetVariables->item(i, 0)->text(),
		    QVariant::fromValue(__ui->tableWidgetVariables->item(i, 1)->text()));

	pm->setParameter("Config-MaxThreads", QVariant::fromValue(__ui->spinBoxThreads->value()));
	pm->setParameter("Config-DefaultLatitude", QVariant::fromValue(__ui->doubleSpinBoxLatitude->value()));
	pm->setParameter("Config-DefaultLongitude", QVariant::fromValue(__ui->doubleSpinBoxLongitude->value()));
	pm->setParameter("Config-Filter-Enabled", QVariant::fromValue(__ui->checkBoxFilter->isChecked()));
	pm->setParameter("Config-Filter-Name", QVariant::fromValue(__ui->comboBoxFilter->currentText()));
	pm->setParameter("Config-Filter-FreqMin", QVariant::fromValue(__ui->doubleSpinBoxFilterMin->value()));
	pm->setParameter("Config-Filter-FreqMax", QVariant::fromValue(__ui->doubleSpinBoxFilterMax->value()));

	return true;
}


bool SettingsPanel::loadConfiguration() {

	SDPASSERT(MainFrame::instancePtr());
	SDPASSERT(Logger::instancePtr());

	Config* cfg = MainFrame::instancePtr()->config();
	Logger* log = Logger::instancePtr();

	try {
		__ui->doubleSpinBoxLatitude->setValue(cfg->getDouble("settings.defaultLatitude"));
	} catch ( ... ) {}

	try {
		__ui->doubleSpinBoxLongitude->setValue(cfg->getDouble("settings.defaultLongitude"));
	} catch ( ... ) {}

	try {
		__ui->spinBoxThreads->setValue(cfg->getInt("settings.maxThreads"));
	} catch ( ... ) {}

	try {
		__ui->checkBoxFilter->setChecked(cfg->getBool("settings.detection.filter.enabled"));
	} catch ( ... ) {}

	try {
		__ui->doubleSpinBoxFilterMin->setValue(cfg->getDouble("settings.detection.filter.freqmin"));
	} catch ( ... ) {}

	try {
		__ui->doubleSpinBoxFilterMax->setValue(cfg->getDouble("settings.detection.filter.freqmax"));
	} catch ( ... ) {}

	QMap<QString, QString> map;
	for (int i = 0; i < __vars.size(); ++i) {
		//! Ignore entities with empty configuration value, this means we
		//! should not get a value from the configuration file for the
		//! entity of interest...
		if ( __vars.at(i).config.isEmpty() ) continue;
		try {
			map.insert(__vars.at(i).name, cfg->getString(__vars.at(i).config).replace('~', QDir::homePath()));
		} catch ( ... ) {}
	}

	for (QMap<QString, QString>::const_iterator it = map.constBegin();
	        it != map.constEnd(); ++it) {

		if ( it.value().isEmpty() ) continue;

		QList<QTableWidgetItem*> itms = __ui->tableWidgetVariables->findItems(it.key(), Qt::MatchExactly);
		if ( itms.size() ) {

			QColor c = Qt::black;
			for (int i = 0; i < __vars.size(); ++i) {
				if ( __vars.at(i).name == it.key() ) {
					if ( __vars.at(i).type == EntityVariable::evBIN ) {
						__vars[i].checked = true;
						if ( !Utils::fileExists(it.value()) ) {
							c = Qt::red;
							log->addMessage(Logger::WARNING, __func__, "Didn't found binary "
							    + __vars.at(i).name + " : " + it.value());
						}
					}
				}
			}

			__ui->tableWidgetVariables->item(itms[0]->row(), 1)->setText(it.value());
			__ui->tableWidgetVariables->item(itms[0]->row(), 1)->setForeground(c);
		}
	}

	return true;
}


void SettingsPanel::saveSettings() {
	saveParameters();
}


void SettingsPanel::tableSettingsEdited(QTableWidgetItem* item) {

	saveSettings();

	//! Update value in entity aswell, check that specified binary if type
	//! is set to that really exists...
	QTableWidgetItem* name = __ui->tableWidgetVariables->item(item->row(), 0);
	if ( !name ) return;
	item->setText(item->text().replace("~", QDir::homePath()));
	for (int i = 0; i < __vars.size(); ++i) {
		if ( __vars.at(i).name != name->text() ) continue;
		__vars[i].value = item->text();
		if ( __vars.at(i).type == EntityVariable::evBIN ) {
			if ( !Utils::fileExists(item->text()) )
				item->setForeground(Qt::red);
			else
				item->setForeground(Qt::black);
		}
	}
}


void SettingsPanel::procTerminated(int retcode) {

	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(Logger::instancePtr());

	QObject* obj = QObject::sender();
	SDPASSERT(obj);

	QProcess* proc = qobject_cast<QProcess*>(obj);
	if ( !proc ) return;

	QString s = QString::fromAscii(proc->readAll());
	QString v;

	for (PCList::const_iterator it = __procs.constBegin();
	        it != __procs.constEnd(); ++it) {
		if ( (*it).name == proc->objectName() ) {
			v = (*it).variable;
			break;
		}
	}

	QList<QTableWidgetItem*> itms = __ui->tableWidgetVariables->findItems(v, Qt::MatchExactly);
	if ( !itms.size() ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Failed to identify variable " + v + " in settings.");
		return;
	}

	bool checked = false;
	for (int i = 0; i < __vars.size(); ++i) {
		if ( __vars.at(i).name == v ) {
			checked = __vars.at(i).checked;
			break;
		}
	}

	if ( !checked ) {
		if ( retcode == 0 ) {
			ParameterManager::instancePtr()->setParameter(v, QVariant::fromValue(s));
			__ui->tableWidgetVariables->item(itms[0]->row(), 1)->setText(s.remove("\n"));
		}
		else
			__ui->tableWidgetVariables->item(itms[0]->row(), 1)->setForeground(Qt::red);
	}

	proc->deleteLater();
}


void SettingsPanel::buttonToggled(int) {

	if ( !__ui->comboBoxFilter->currentText().contains("bandpass") ) {
		__ui->labelFreqMin->setText("Freq. :");
		__ui->labelFreqMax->hide();
		__ui->doubleSpinBoxFilterMax->hide();
	}
	else {
		__ui->labelFreqMin->setText("Freq. min. :");
		__ui->labelFreqMax->show();
		__ui->doubleSpinBoxFilterMax->show();
	}
	saveParameters();
}


InteractiveTree::InteractiveTree(QWidget* parent) :
		QTreeWidget(parent) {}


InteractiveTree::~InteractiveTree() {}


void InteractiveTree::keyPressEvent(QKeyEvent* event) {

	if ( event->key() == Qt::Key_Up ) {
		if ( indexAbove(currentIndex()).isValid() ) {
			setCurrentIndex(indexAbove(currentIndex()));
			emit clicked(currentIndex());
		}
	}

	if ( event->key() == Qt::Key_Down ) {
		if ( indexBelow(currentIndex()).isValid() ) {
			setCurrentIndex(indexBelow(currentIndex()));
			emit clicked(currentIndex());
		}
	}

	if ( event->key() == Qt::Key_Left ) {
		if ( currentItem()->isExpanded() )
		    this->collapseItem(currentItem());
	}

	if ( event->key() == Qt::Key_Right || event->key() == Qt::Key_Enter ) {
		if ( !currentItem()->isExpanded() )
		    this->expandItem(currentItem());
	}

	if ( event->key() == Qt::Key_Delete )
	    emit deleteSelected();
}


InteractiveTable::InteractiveTable(QWidget* parent) :
		QTableWidget(parent) {}


InteractiveTable::~InteractiveTable() {}


void InteractiveTable::keyPressEvent(QKeyEvent* event) {

	int tmpRow = currentRow();
	int tmpCol = currentColumn();

	if ( event->key() == Qt::Key_Up ) {
		tmpRow = qMax(0, tmpRow - 1);
		clearSelection();
		setCurrentCell(tmpRow, tmpCol);
		emit clicked(currentIndex());
	}

	if ( event->key() == Qt::Key_Down ) {
		tmpRow = qMin(rowCount() - 1, tmpRow + 1);
		clearSelection();
		setCurrentCell(tmpRow, tmpCol);
		emit clicked(currentIndex());
	}

	if ( event->key() == Qt::Key_Delete )
	    emit deleteSelected();
}


RecentPanel::RecentPanel(QWidget* parent) :
		PanelWidget(parent), __ui(new Ui::RecentPanel), __jobSelected(NULL) {

	__ui->setupUi(this);

	setHeader(tr("Recent"));
	setDescription(tr("Go thru application jobs history and results."));

	initInteractiveTree();

	QFont tf(__ui->textEditObjects->font());
#ifdef Q_OS_MAC
	tf.setFamily("Monaco");
	tf.setPointSize(11);
#else
	tf.setFamily("Monospace");
#endif
	tf.setFixedPitch(true);
	__ui->textEditObjects->setFont(tf);

	__ui->splitterInformation->setSizes(QList<int>() << 0 << 3 << 1);
	__ui->splitterMain->setSizes(QList<int>() << 3 << 1);

	//! Disable search feature for now
	connect(__ui->checkBoxActivate, SIGNAL(clicked(bool)), __ui->dateTimeEditFrom, SLOT(setEnabled(bool)));
	connect(__ui->checkBoxActivate, SIGNAL(clicked(bool)), __ui->dateTimeEditTo, SLOT(setEnabled(bool)));
	__ui->frame->hide();

	FancyPanel* toolbar = new FancyPanel(this, FancyPanel::Horizontal, 65, FancyPanel::Gray);
	__reprocessButton = new FancyButton(QPixmap(":images/redo.png"), tr("Re-process"), toolbar);
	__removeButton = new FancyButton(QPixmap(":images/clear.png"), tr("Clear"), toolbar);

	__reprocessButton->setCheckable(false);
	__removeButton->setCheckable(false);

	__reprocessButton->setToolTip(tr("Sends selected job(s) into the activity "
		"manager so that it/they could be processed again"));
	__removeButton->setToolTip(tr("Remove selected job(s) from the application "
		"database. All files related to it will be removed aswell."));

	__reprocessButton->setEnabled(false);
	__removeButton->setEnabled(false);

	toolbar->addFancyButton(__reprocessButton);
	toolbar->addFancyButton(__removeButton);

	connect(__reprocessButton, SIGNAL(clicked()), this, SLOT(reprocessJobs()));
	connect(__removeButton, SIGNAL(clicked()), this, SLOT(removeJobs()));

	QVBoxLayout* l = new QVBoxLayout(__ui->widgetToolbar);
	l->setContentsMargins(0, 0, 0, 4);
	l->addWidget(toolbar);

	QFont tbf(toolbar->font());
	tbf.setPointSize(10);
#ifdef Q_OS_MAC
	tbf.setPointSize(13);
#endif
	toolbar->setFont(tbf);
}


RecentPanel::~RecentPanel() {}


bool RecentPanel::saveParameters() {
	return true;
}


void RecentPanel::loadJob(Job* job) {

	if ( !job ) return;

	QList<QTreeWidgetItem*> l = __tree->findItems(job->id(), Qt::MatchExactly);
	if ( !l.isEmpty() ) return;

	if ( __parents.contains(job->id()) ) return;

	QString sizeString = "-";
	if ( job->type() == Detection ) {
		quint64 runDirSize = Utils::dirSize(job->runDir());
		quint64 dirSizeMB = runDirSize / (1024 * 1024);
		sizeString = (dirSizeMB > 1024) ? QString("%1GB").arg(dirSizeMB / 1024)
		    : QString("%1MB").arg(dirSizeMB);
	}

	QTreeWidgetItem* obj = new QTreeWidgetItem(__tree);
	obj->setText(getHeaderPosition(thCTIME), job->creationTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	obj->setTextAlignment(getHeaderPosition(thCTIME), Qt::AlignLeft);
	obj->setText(getHeaderPosition(thTYPE), (*job->type()).string);
	obj->setTextAlignment(getHeaderPosition(thTYPE), Qt::AlignHCenter | Qt::AlignVCenter);
	obj->setText(getHeaderPosition(thRUNSTART), job->runStartTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	obj->setText(getHeaderPosition(thRUNEND), job->runEndTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	obj->setText(getHeaderPosition(thRUNSTATUS), translateExitCode(job->runExitCode()));
	obj->setText(getHeaderPosition(thINFORMATION), job->information());
	obj->setText(getHeaderPosition(thCOMMENT), job->comment());
	obj->setText(getHeaderPosition(thID), job->id());
	obj->setData(getHeaderPosition(thID), Qt::UserRole, Utils::VariantPtr<Job>::asQVariant(job));
	obj->setTextAlignment(getHeaderPosition(thID), Qt::AlignHCenter | Qt::AlignVCenter);
	obj->setText(getHeaderPosition(thSIZE), sizeString);

	for (int i = 0; i < static_cast<int>(RecentHeadersString.size()); ++i)
		obj->setToolTip(i, job->tooltip());

	if ( DetectionJob* det = dynamic_cast<DetectionJob*>(job) ) {

		det->loadTriggers();

		QFont f(obj->font(getHeaderPosition(thCTIME)));
		f.setItalic(true);
//		f.setBold(true);

		QTreeWidgetItem* committed = new QTreeWidgetItem(obj);
		committed->setForeground(getHeaderPosition(thCTIME), Qt::darkGreen);
		committed->setText(getHeaderPosition(thCTIME), "Triggers committed");
		committed->setToolTip(getHeaderPosition(thCTIME), "Triggers successfully committed for job " + job->id());
//		committed->setTextAlignment(getHeaderPosition(thCTIME), Qt::AlignRight);
		committed->setFont(getHeaderPosition(thCTIME), f);

		QTreeWidgetItem* accepted = new QTreeWidgetItem(obj);
		accepted->setForeground(getHeaderPosition(thCTIME), Qt::blue);
		accepted->setText(getHeaderPosition(thCTIME), "Triggers accepted");
		accepted->setToolTip(getHeaderPosition(thCTIME), "Triggers accepted but not yet committed for job " + job->id());
//		accepted->setTextAlignment(getHeaderPosition(thCTIME), Qt::AlignRight);
		accepted->setFont(getHeaderPosition(thCTIME), f);

		QTreeWidgetItem* rejected = new QTreeWidgetItem(obj);
		rejected->setForeground(getHeaderPosition(thCTIME), Qt::red);
		rejected->setText(getHeaderPosition(thCTIME), "Triggers rejected");
		rejected->setToolTip(getHeaderPosition(thCTIME), "Triggers rejected for job " + job->id());
//		rejected->setTextAlignment(getHeaderPosition(thCTIME), Qt::AlignRight);
		rejected->setFont(getHeaderPosition(thCTIME), f);

		QTreeWidgetItem* awaiting = new QTreeWidgetItem(obj);
		awaiting->setForeground(getHeaderPosition(thCTIME), Qt::gray);
		awaiting->setText(getHeaderPosition(thCTIME), "Triggers awaiting revision");
		awaiting->setToolTip(getHeaderPosition(thCTIME), "Triggers awaiting manual revision for job " + job->id());
//		awaiting->setTextAlignment(getHeaderPosition(thCTIME), Qt::AlignRight);
		awaiting->setFont(getHeaderPosition(thCTIME), f);

		obj->addChild(committed);
		obj->addChild(accepted);
		obj->addChild(rejected);
		obj->addChild(awaiting);

		//! Job's family
		__parents.insert(job->id(), Family(obj, obj, accepted, rejected, committed, awaiting));

		for (int i = 0; i < det->triggers().size(); ++i) {

			Trigger* trig = det->triggers().at(i);
			if ( !trig ) continue;

			addTriggerRow(job, trig);
		}
	}

	else if ( DispatchJob* dis = dynamic_cast<DispatchJob*>(job) ) {
		__parents.insert(dis->id(), Family(obj, obj));

		QString tooltip;
		tooltip += "<p><div id=\"dispatchinfo\">";
		tooltip += "<b>Dispatch&nbsp;tool</b>:&nbsp;" + dis->tool() + "<br/>";
		tooltip += "<b>Run&nbsp;dir</b>:&nbsp;" + dis->runDir() + "<br/>";
		tooltip += "<b>MiniSEED&nbsp;dir</b>:&nbsp;" + dis->msdDir() + "<br/>";
		tooltip += "<b>MiniSEED&nbsp;pattern</b>:&nbsp;" + dis->msdPattern() + "<br/>";
		tooltip += "<b>SDS&nbsp;dir</b>:&nbsp;" + dis->sdsDir() + "<br/>";
		tooltip += "<b>SDS&nbsp;pattern</b>:&nbsp;" + dis->sdsPattern() + "<br/>";
		tooltip += "<b>Information</b>:&nbsp;" + dis->information() + "<br/>";
		tooltip += "<b>Comment</b>:&nbsp;" + dis->comment() + "<br/>";
		tooltip += "</div></p>";

		for (int j = 0; j < __tree->columnCount(); ++j)
			obj->setToolTip(j, tooltip);
	}
}


void RecentPanel::addTriggerRow(Job* job, Trigger* trig) {

	if ( !job ) return;
	if ( !trig ) return;
	if ( !__parents.contains(job->id()) ) loadJob(job);
	if ( __parents.contains(trig->id()) ) return;

	QTreeWidgetItem* parent = __parents.value(job->id()).parentItem;
	QTreeWidgetItem* accepted = __parents.value(job->id()).acceptedItem;
	QTreeWidgetItem* awaiting = __parents.value(job->id()).awaitingItem;
	QTreeWidgetItem* rejected = __parents.value(job->id()).rejectedItem;
	QTreeWidgetItem* committed = __parents.value(job->id()).committedItem;

	QString ttip;
	ttip += "<p><div id=\"picture\">";

	bool showAllTriggers =
	    (trig->stations().size() < 3) ? true : false;
	QString stations;
	for (int j = 0; j < trig->stations().size(); ++j) {
		stations.append(trig->stations().at(j).networkCode + "."
		    + trig->stations().at(j).code + ", ");
		if ( showAllTriggers ) {
			ttip += QString("<a><img src=\"%1\" title=\"trigger\" alt=\"trigger\" width=\"300\" border=\"0\" /></a><br />")
			    .arg(trig->stations().at(j).snapshotFile);
		}
		else {
			if ( j < 3 )
				ttip += QString("<a><img src=\"%1\" title=\"trigger\" alt=\"trigger\" width=\"300\" border=\"0\" /></a><br />")
				    .arg(trig->stations().at(j).snapshotFile);
			else if ( j < 4 )
			    ttip += "<a><b>Select origin to display more triggers...</b></a>";
		}
	}
	ttip += "</div></p>";

	QTreeWidgetItem* row = NULL;
	switch ( trig->status() ) {
		case Committed:
			row = new QTreeWidgetItem(committed);
			committed->addChild(row);
			break;
		case Accepted:
			row = new QTreeWidgetItem(accepted);
			accepted->addChild(row);
			break;
		case Rejected:
			row = new QTreeWidgetItem(rejected);
			rejected->addChild(row);
			break;
		case WaitingForRevision:
			row = new QTreeWidgetItem(awaiting);
			awaiting->addChild(row);
			break;
		default:
			row = new QTreeWidgetItem(awaiting);
			awaiting->addChild(row);
			break;
	}

	//! Row family
	__parents.insert(trig->id(), Family(row, parent, accepted, rejected, committed, awaiting));

	for (int z = 0; z < __tree->columnCount(); ++z)
		row->setText(z, "-");

	row->setText(getHeaderPosition(thCTIME), trig->originTime().toString("OTyyyy-MM-dd HH:mm:ss.zzz"));
	row->setTextAlignment(getHeaderPosition(thCTIME), Qt::AlignRight);
	//! Store pointer inside the CTIME column 'cause the cast will fail
	//! when trying to differentiate between Job and ArchivedRun...
	row->setData(getHeaderPosition(thCTIME), Qt::UserRole,
	    Utils::VariantPtr<Trigger>::asQVariant(trig));
	row->setText(getHeaderPosition(thID), trig->id());
	row->setText(getHeaderPosition(thINFORMATION), stations);

	for (int j = 0; j < __tree->columnCount(); ++j)
		row->setToolTip(j, ttip);
}


void RecentPanel::loadJobFromRunDir(Job* job) {

	SDPASSERT(job);
	SDPASSERT(Logger::instancePtr());

	Logger* log = Logger::instancePtr();

	QList<QTreeWidgetItem*> l = __tree->findItems(job->id(), Qt::MatchExactly);
	if ( !l.isEmpty() ) return;

	QString sizeString;
	if ( job->type() == Detection ) {
		quint64 runDirSize = Utils::dirSize(job->runDir());
		quint64 dirSizeMB = runDirSize / (1024 * 1024);
		sizeString = (dirSizeMB > 1024) ? QString("%1GB").arg(dirSizeMB / 1024)
		    : QString("%1MB").arg(dirSizeMB);
	}

	QTreeWidgetItem* obj = new QTreeWidgetItem(__tree);
	obj->setText(getHeaderPosition(thCTIME), job->creationTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	obj->setTextAlignment(getHeaderPosition(thCTIME), Qt::AlignLeft);
	obj->setText(getHeaderPosition(thTYPE), (*job->type()).string);
	obj->setTextAlignment(getHeaderPosition(thTYPE), Qt::AlignHCenter | Qt::AlignVCenter);
	obj->setText(getHeaderPosition(thRUNSTART), job->runStartTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	obj->setText(getHeaderPosition(thRUNEND), job->runEndTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	obj->setText(getHeaderPosition(thRUNSTATUS), translateExitCode(job->runExitCode()));
	obj->setText(getHeaderPosition(thINFORMATION), job->information());
	obj->setText(getHeaderPosition(thCOMMENT), job->comment());
	obj->setText(getHeaderPosition(thID), job->id());
	obj->setData(getHeaderPosition(thID), Qt::UserRole, Utils::VariantPtr<Job>::asQVariant(job));
	obj->setTextAlignment(getHeaderPosition(thID), Qt::AlignHCenter | Qt::AlignVCenter);
	obj->setText(getHeaderPosition(thSIZE), sizeString);

	for (int i = 0; i < static_cast<int>(RecentHeadersString.size()); ++i)
		obj->setToolTip(i, job->tooltip());

	log->addMessage(Logger::DEBUG, __func__, QString("Loading job data from dir %1")
	    .arg(job->runDir()), false);

	ArchivedRun* run = ArchivedRun::loadRun(job);
	if ( !run ) {
		log->addMessage(Logger::DEBUG, __func__, QString("No run folder found for job %1 [%2]")
		    .arg(job->id()).arg(job->runDir()), false);
		return;
	}

	QTreeWidgetItem* th = new QTreeWidgetItem(obj);
	th->setText(getHeaderPosition(thCTIME), "Triggers");
	th->setToolTip(getHeaderPosition(thCTIME), "Triggers reported by job " + job->id());
	th->setTextAlignment(getHeaderPosition(thCTIME), Qt::AlignRight);

	QFont f(th->font(getHeaderPosition(thCTIME)));
	f.setItalic(true);
	th->setFont(getHeaderPosition(thCTIME), f);

	obj->addChild(th);

	for (int i = 0; i < run->origins().size(); ++i) {

		QString ttip;
		ttip += "<p><div id=\"picture\">";

		bool showAllTriggers =
		    (run->origins().at(i)->stations().size() < 3) ? true : false;
		QString stations;
		for (int j = 0; j < run->origins().at(i)->stations().size(); ++j) {
			stations.append(run->origins().at(i)->stations().at(j)->network() + "."
			    + run->origins().at(i)->stations().at(j)->code() + ", ");
			if ( showAllTriggers ) {
				ttip += QString("<a><img src=\"%1\" title=\"trigger\" alt=\"trigger\" width=\"300\" border=\"0\" /></a><br />")
				    .arg(run->origins().at(i)->stations().at(j)->pixmap());
			}
			else {
				if ( j < 3 )
					ttip += QString("<a><img src=\"%1\" title=\"trigger\" alt=\"trigger\" width=\"300\" border=\"0\" /></a><br />")
					    .arg(run->origins().at(i)->stations().at(j)->pixmap());
				else if ( j < 4 )
				    ttip += "<a><b>Select origin to display more triggers...</b></a>";
			}
		}
		ttip += "</div></p>";

		QTreeWidgetItem* row = new QTreeWidgetItem(th);

		for (int z = 0; z < __tree->columnCount(); ++z)
			row->setText(z, "-");

		row->setText(getHeaderPosition(thCTIME), run->origins().at(i)->time().toString("OTyyyy-MM-dd HH:mm:ss.zzz"));
		row->setTextAlignment(getHeaderPosition(thCTIME), Qt::AlignRight);
		//! Store pointer inside the CTIME column 'cause the cast will fail
		//! when trying to differentiate between Job and ArchivedRun...
		row->setData(getHeaderPosition(thCTIME), Qt::UserRole,
		    Utils::VariantPtr<ArchivedOrigin>::asQVariant(run->origins().at(i)));
		row->setText(getHeaderPosition(thID), "OT" + run->origins().at(i)->time().toString(Qt::ISODate));
		row->setText(getHeaderPosition(thINFORMATION), stations);

		for (int j = 0; j < __tree->columnCount(); ++j)
			row->setToolTip(j, ttip);

		th->addChild(row);
	}
}


void RecentPanel::removeJob(Job* job) {

	if ( !job ) return;

	SDPASSERT(Logger::instancePtr());
	SDPASSERT(TriggerPanel::instancePtr());
	SDPASSERT(ActivityPanel::instancePtr());
	SDPASSERT(Cache::instancePtr());

	Logger* log = Logger::instancePtr();
	Cache* cache = Cache::instancePtr();

	if ( !Utils::removeDir(job->runDir()) )
	    log->addMessage(Logger::WARNING, __func__,
	        QString("Failed to remove job %1 run dir %2").arg(job->id()).arg(job->runDir()));

	if ( DetectionJob* det = dynamic_cast<DetectionJob*>(job) ) {

		//! Remove children of job and job itself
		for (int i = 0; i < det->triggers().size(); ++i) {
			if ( __parents.contains(det->triggers().at(i)->id()) )
			    __parents.remove(det->triggers().at(i)->id());
		}

		//! Remove eventual triggers pending validation
		TriggerPanel::instancePtr()->removeTriggers(job->id());

		//! Remove job from Activity Panel if it is there (although it
		//! shouldn't be in both Recent and Activity at the same time)
		//! TODO: investigate this logic further more... segfault occurred once...
		ActivityPanel::instancePtr()->removeJobs(QList<QString>() << job->id());

		//! Cascade delete everything related to job
		cache->removeObject(det->id());
	}
	else if ( DispatchJob* dis = dynamic_cast<DispatchJob*>(job) ) {
		//! Delete dispatch
		cache->removeObject(dis->id());
	}

	if ( __parents.contains(job->id()) )
	    __parents.remove(job->id());
}


void RecentPanel::removeJobs(QList<QString> jbs) {

	RecentItems itms = jobs();
	for (int i = 0; i < itms.size(); ++i) {
		if ( !jbs.contains(itms.at(i).job->id()) ) continue;
		if ( __parents.contains(itms.at(i).job->id()) )
		    __parents.remove(itms.at(i).job->id());
		delete itms[i].item;
	}

	if ( __tree->topLevelItemCount() == 0 ) {
		__ui->webViewObjects->load(QUrl());
		__ui->textEditObjects->clear();
		__ui->labelObjectName->setText("-");
		__ui->labelStartTime->setText("-");
		__ui->labelEndTime->setText("-");
		__ui->labelDuration->setText("-");
	}
}


void RecentPanel::reprocessJobs() {

	RecentItems items = treeSelection();
	if ( items.isEmpty() ) return;

	SDPASSERT(Logger::instancePtr());
	SDPASSERT(ActivityPanel::instancePtr());
	SDPASSERT(TriggerPanel::instancePtr());

	Progress prog(this, Qt::FramelessWindowHint);
	prog.show("Resetting job(s)");
	for (int i = 0; i < items.size(); ++i) {
		Logger::instancePtr()->addMessage(Logger::INFO, __func__, "Setting job "
		    + items.at(i).job->id() + " to be re-processed");
		__parents.remove(items.at(i).job->id());
		items.at(i).job->reset();
		TriggerPanel::instancePtr()->removeTriggers(items.at(i).job->id());
		ActivityPanel::instancePtr()->addJob(items.at(i).job);
		delete items[i].item;
	}
	prog.hide();
}


void RecentPanel::removeJobs() {

	Progress prog(this, Qt::FramelessWindowHint);
	RecentItems items = treeSelection();
	if ( items.isEmpty() ) return;

	bool yesToAll = false;
	bool noToAll = false;
	for (int i = 0; i < items.size(); ++i) {

		if ( !yesToAll && !noToAll ) {

			QMessageBox::StandardButton b = QMessageBox::question(this, tr("Delete job"),
			    QString("You are about to delete %1 job permanently from the inventory?<br/>"
				    "(All related files will be deleted)<br/>Are you sure ?")
			        .arg(items.size()), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::No);

			if ( b == QMessageBox::Yes ) {
				prog.show("Removing job " + items.at(i).job->id());
				__parents.remove(items.at(i).job->id());
				removeJob(items.at(i).job);
				delete items[i].item;
			}
			else if ( b == QMessageBox::YesToAll ) {
				prog.show("Removing job " + items.at(i).job->id());
				__parents.remove(items.at(i).job->id());
				removeJob(items.at(i).job);
				delete items[i].item;
				yesToAll = true;
			}
			else if ( b == QMessageBox::No )
				continue;
			else if ( b == QMessageBox::NoToAll ) {
				noToAll = true;
				continue;
			}
		}
		else {
			if ( yesToAll && !noToAll ) {
				prog.show("Removing job " + items.at(i).job->id());
				__parents.remove(items.at(i).job->id());
				removeJob(items.at(i).job);
				delete items[i].item;
			}
			if ( !yesToAll && noToAll )
			    continue;
		}
	}

	if ( __tree->topLevelItemCount() == 0 ) {
		__ui->webViewObjects->load(QUrl());
		__ui->textEditObjects->clear();
		__ui->labelObjectName->setText("-");
		__ui->labelStartTime->setText("-");
		__ui->labelEndTime->setText("-");
		__ui->labelDuration->setText("-");
	}

	prog.hide();
}


void RecentPanel::triggerStatusModified(int tm, QList<QString> list) {

	TriggerMessage t = static_cast<TriggerMessage>(tm);

	for (int i = 0; i < list.size(); ++i) {
		if ( !__parents.contains(list.at(i)) ) continue;
		Family f = __parents.value(list.at(i));
		switch ( t ) {
			case tmAccepted:
				f.rejectedItem->removeChild(f.object);
				f.awaitingItem->removeChild(f.object);
				f.committedItem->removeChild(f.object);
				f.acceptedItem->addChild(f.object);
				break;
			case tmRejected:
				f.awaitingItem->removeChild(f.object);
				f.committedItem->removeChild(f.object);
				f.acceptedItem->removeChild(f.object);
				f.rejectedItem->addChild(f.object);
				break;
			case tmCommitted:
				f.rejectedItem->removeChild(f.object);
				f.awaitingItem->removeChild(f.object);
				f.acceptedItem->removeChild(f.object);
				f.committedItem->addChild(f.object);
				break;
			case tmAwaiting:
				f.rejectedItem->removeChild(f.object);
				f.committedItem->removeChild(f.object);
				f.acceptedItem->removeChild(f.object);
				f.awaitingItem->addChild(f.object);
				break;
			case tmDeleted:
				f.rejectedItem->removeChild(f.object);
				f.committedItem->removeChild(f.object);
				f.acceptedItem->removeChild(f.object);
				f.awaitingItem->removeChild(f.object);
				__parents.remove(list.at(i));
				break;
		}
	}
}


void RecentPanel::addDetectionTriggers(QString jobID, QList<QString> list) {

	SDPASSERT(Cache::instancePtr());
	SDPASSERT(Logger::instancePtr());
	Logger* log = Logger::instancePtr();

	DetectionJob* job = Cache::instancePtr()->getObject<DetectionJob*>(jobID);
	if ( !job ) {
		log->addMessage(Logger::WARNING, __func__, "Failed to find object " + jobID
		    + " in cache, skipping adding triggers to its tree branch");
		return;
	}

	for (int i = 0; i < list.size(); ++i) {
		Trigger* trig = Cache::instancePtr()->getObject<Trigger*>(list.at(i));
		if ( !trig ) {
			log->addMessage(Logger::WARNING, __func__, "Failed to find object "
			    + list.at(i) + " in cache, skipping adding trigger to parent "
			    + jobID + " branch");
			continue;
		}

		addTriggerRow(job, trig);
	}
}


void RecentPanel::headerMenu(const QPoint&) {

	QMenu menu(this);
	for (HeaderActions::const_iterator it = __actions.constBegin();
	        it != __actions.constEnd(); ++it)
		menu.addAction((*it).second);
	menu.exec(QCursor::pos());
}


void RecentPanel::showHideHeaderItems() {

	for (HeaderActions::const_iterator it = __actions.constBegin();
	        it != __actions.constEnd(); ++it)
		((*it).second->isChecked()) ?
		    __tree->showColumn((*it).first) : __tree->hideColumn((*it).first);
}


void RecentPanel::objectSelectionChanged() {

	QList<QTreeWidgetItem*> l = __tree->selectedItems();
	__removeButton->setEnabled(!l.isEmpty());
	__reprocessButton->setEnabled(!l.isEmpty());

	if ( l.isEmpty() ) {
		__ui->webViewObjects->load(QUrl());
		__ui->textEditObjects->clear();
		__ui->labelObjectName->setText("-");
		__ui->labelStartTime->setText("-");
		__ui->labelEndTime->setText("-");
		__ui->labelDuration->setText("-");
	}
}


void RecentPanel::objectSelected(QTreeWidgetItem* item, const int&) {

	__ui->webViewObjects->load(QUrl());
	__ui->textEditObjects->clear();
	__ui->labelObjectName->setText("-");
	__ui->labelStartTime->setText("-");
	__ui->labelEndTime->setText("-");
	__ui->labelDuration->setText("-");

	Job* job = Utils::VariantPtr<Job>::asPtr(item->data(getHeaderPosition(thID), Qt::UserRole));

	if ( job ) {
		displayJobOutput(job);
		return;
	}

	Trigger* trig = Utils::VariantPtr<Trigger>::asPtr(item->data(getHeaderPosition(thCTIME), Qt::UserRole));

	if ( trig ) {
		displayTriggerInformation(trig);
		return;
	}
}


void RecentPanel::initInteractiveTree() {

	QFont f;
	f.setPointSize(9);
#ifdef Q_OS_MAC
	f.setPointSize(11);
#endif

	QVBoxLayout* l = new QVBoxLayout(__ui->widgetTree);
	l->setSpacing(0);
	l->setMargin(0);

	__tree = new InteractiveTree(__ui->widgetTree);
	__tree->setObjectName(QString::fromUtf8("treeWidgetObjects"));
	__tree->setFont(f);
	__tree->setMinimumSize(QSize(0, 250));
	__tree->setContextMenuPolicy(Qt::CustomContextMenu);
	__tree->setFrameShape(QFrame::NoFrame);
	__tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
	__tree->setAlternatingRowColors(true);
	__tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	__tree->setTextElideMode(Qt::ElideRight);
	__tree->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	__tree->setIndentation(20);
	__tree->setSortingEnabled(true);
	__tree->setAnimated(true);
	__tree->setWordWrap(false);
	__tree->header()->setCascadingSectionResizes(true);
	__tree->header()->setFont(f);

	QStringList headers;
	for (size_t i = 0; i < RecentHeadersString.size(); ++i) {
		headers << RecentHeadersString[i];
		QAction* a = new QAction(tr(RecentHeadersString[i]), this);
		a->setCheckable(true);
		a->setChecked(true);
		connect(a, SIGNAL(triggered()), this, SLOT(showHideHeaderItems()));
		__actions.append(PairedAction(static_cast<int>(i), a));
	}

	__tree->setColumnCount(RecentHeadersString.size());
	__tree->setHeaderLabels(headers);
	__tree->sortByColumn(0, Qt::AscendingOrder);
	__tree->setStyleSheet("QTableView {selection-background-color: rgb(0,0,0,100);}");
	__tree->header()->setResizeMode(QHeaderView::ResizeToContents);

	for (size_t i = 0; i < RecentHeadersString.size(); ++i)
		__tree->headerItem()->setTextAlignment(i, Qt::AlignVCenter | Qt::AlignHCenter);

	__tree->header()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(__tree->header(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(headerMenu(const QPoint&)));
	connect(__tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(objectSelected(QTreeWidgetItem*, const int&)));
	connect(__tree, SIGNAL(deleteSelected()), this, SLOT(removeJobs()));
	connect(__tree, SIGNAL(itemSelectionChanged()), this, SLOT(objectSelectionChanged()));

	l->addWidget(__tree);
}


RecentItems RecentPanel::treeSelection() {

	QList<QTreeWidgetItem*> l = __tree->selectedItems();
	if ( l.isEmpty() ) return RecentItems();

	RecentItems items;
	for (int i = 0; i < l.size(); ++i) {

		QString id = l.at(i)->text(getHeaderPosition(thID));

		if ( !__parents.contains(id) ) continue;
		if ( !__parents.value(id).object ) continue;

		Job* job = Utils::VariantPtr<Job>::asPtr(l.at(i)->data(getHeaderPosition(thID), Qt::UserRole));
		if ( !job ) continue;

		RecentItem ri;
		ri.item = __parents[id].object;
		ri.job = job;
		items << ri;
	}

	return items;
}


RecentItems RecentPanel::jobs() {

	RecentItems items;
	for (int i = 0; i < __tree->topLevelItemCount(); ++i) {

		Job* job = Utils::VariantPtr<Job>::asPtr(__tree->topLevelItem(i)->data(getHeaderPosition(thID), Qt::UserRole));
		if ( !job ) continue;

		RecentItem ri;
		ri.item = __tree->topLevelItem(i);
		ri.job = job;
		items << ri;
	}

	return items;
}


void RecentPanel::displayJobOutput(Job* job) {

	if ( !job ) return;

	__ui->splitterInformation->setSizes(QList<int>() << 3 << 1 << 1);
	__ui->textEditObjects->clear();
	__ui->labelObjectName->setText(job->id());
	__ui->labelStartTime->setText(job->runStartTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	__ui->labelEndTime->setText(job->runEndTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	__ui->labelDuration->setText(Utils::elapsedTime(job->runStartTime(), job->runEndTime()));

	for (Job::StandardOutput::const_iterator it = job->stdOutput().constBegin();
	        it != job->stdOutput().constEnd(); ++it) {
		if ( (*it).first == Job::Info ) {
			__ui->textEditObjects->append(
			    QString("<font color=\"black\">%1</font>").arg((*it).second));
		}
		else
			__ui->textEditObjects->append(
			    QString("<font color=\"red\">%1</font>").arg((*it).second));
	}
}


void RecentPanel::displayTriggerInformation(Trigger* trig) {

	SDPASSERT(Cache::instancePtr());
	DetectionJob* job = Cache::instancePtr()->getObject<DetectionJob*>(trig->jobID());

	if ( !job ) return;

	__ui->splitterInformation->setSizes(QList<int>() << 0 << 3 << 1);
	__ui->webViewObjects->load(QUrl::fromLocalFile(trig->resumePage()));
	__ui->textEditObjects->clear();
	__ui->labelObjectName->setText(trig->id());
	__ui->labelStartTime->setText(job->runStartTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	__ui->labelEndTime->setText(job->runEndTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	__ui->labelDuration->setText(Utils::elapsedTime(job->runStartTime(), job->runEndTime()));

	for (Job::StandardOutput::const_iterator it = job->stdOutput().constBegin();
	        it != job->stdOutput().constEnd(); ++it) {
		if ( (*it).first == Job::Info ) {
			__ui->textEditObjects->append(
			    QString("<font color=\"black\">%1</font>")
			        .arg((*it).second));
		}
		else
			__ui->textEditObjects->append(
			    QString("<font color=\"red\">%1</font>")
			        .arg((*it).second));
	}
}


void RecentPanel::displayOriginOutput(ArchivedOrigin* o) {

	__ui->webViewObjects->load(QUrl::fromLocalFile(o->webpage()));
	__ui->textEditObjects->clear();
	__ui->labelObjectName->setText(QString("Origin %1").arg(o->time().toString(Qt::ISODate)));
	__ui->labelStartTime->setText(o->run()->job()->runStartTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	__ui->labelEndTime->setText(o->run()->job()->runEndTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	__ui->labelDuration->setText(Utils::elapsedTime(o->run()->job()->runStartTime(), o->run()->job()->runEndTime()));

	for (Job::StandardOutput::const_iterator it = o->run()->job()->stdOutput().constBegin();
	        it != o->run()->job()->stdOutput().constEnd(); ++it) {
		if ( (*it).first == Job::Info ) {
			__ui->textEditObjects->append(
			    QString("<font color=\"black\">%1</font>")
			        .arg((*it).second));
		}
		else
			__ui->textEditObjects->append(
			    QString("<font color=\"red\">%1</font>")
			        .arg((*it).second));
	}
}


ActivityPanel::ActivityPanel(QWidget* parent) :
		PanelWidget(parent), __ui(new Ui::ActivityPanel), __jobSelected(NULL),
		__status(Idle) {

	__ui->setupUi(this);

	setHeader(tr("Activity"));
	setDescription(tr("Manage application's processes."));

	initInteractiveTable();

	QFont tf(__ui->textEditJobs->font());
#ifdef Q_OS_MAC
	tf.setFamily("Monaco");
	tf.setPointSize(11);
#else
	tf.setFamily("Monospace");
#endif
	tf.setFixedPitch(true);
	__ui->textEditJobs->setFont(tf);

	__ui->frameJobInformation->hide();

	QVBoxLayout* l = new QVBoxLayout(__ui->widgetToolbar);
	l->setContentsMargins(0, 0, 0, 4);
	FancyPanel* toolbar = new FancyPanel(this, FancyPanel::Horizontal, 65, FancyPanel::Gray);
	toolbar->setSelectionMode(FancyPanel::SingleFreeSelection);

	__runButton = new FancyButton(QPixmap(":images/play.png"), tr("Run"), toolbar);
	__stopButton = new FancyButton(QPixmap(":images/stop.png"), tr("Stop"), toolbar);
	__removeButton = new FancyButton(QPixmap(":images/clear.png"), tr("Clear"), toolbar);
	__resetButton = new FancyButton(QPixmap(":images/reset.png"), tr("Reset"), toolbar);
	__editButton = new FancyButton(QPixmap(":images/edit.png"), tr("Edit script"), toolbar);
	__archiveButton = new FancyButton(QPixmap(":images/archive.png"), tr("Auto archive"), toolbar);

	__runButton->setCheckable(false);
	__stopButton->setCheckable(false);
	__removeButton->setCheckable(false);
	__resetButton->setCheckable(false);
	__resetButton->setEnabled(false);
	__editButton->setCheckable(false);
	__editButton->setEnabled(false);
	__archiveButton->setCheckable(true);

	__runButton->setToolTip(tr("Starts the queue. Processes are launched from top to bottom. "
		"If one or many processes are selected, only the one(s) selected will be started."));
	__stopButton->setToolTip(tr("Stops the queue. If one or many processes are selected, "
		"only the one(s) selected will be stopped."));
	__removeButton->setToolTip(tr("Clears the queue. If one or many processes are selected, "
		"only the one(s) selected will be deleted."));
	__resetButton->setToolTip(tr("Resets selected job"));
	__editButton->setToolTip(tr("Sends back script into dispatch or detection panel so that the user can edit it"));
	__archiveButton->setToolTip(tr("When this button is checked, terminated jobs "
		"will be automatically transfered into the history section of the application."));

	toolbar->addFancyButton(__runButton);
	toolbar->addFancyButton(__stopButton);
	toolbar->addFancyButton(__removeButton);
	toolbar->addFancyButton(__resetButton);
	toolbar->addFancyButton(__editButton);
	toolbar->addFancyButton(__archiveButton);

	connect(__runButton, SIGNAL(clicked()), this, SLOT(startQueue()));
	connect(__stopButton, SIGNAL(clicked()), this, SLOT(stopQueue()));
	connect(__removeButton, SIGNAL(clicked()), this, SLOT(removeJobs()));
	connect(__resetButton, SIGNAL(clicked()), this, SLOT(resetJobs()));
	connect(__editButton, SIGNAL(clicked()), this, SLOT(editJob()));
	connect(__archiveButton, SIGNAL(clicked()), this, SLOT(archiveJobs()));

	l->addWidget(toolbar);

	__timer = new QTimer(this);
	connect(__timer, SIGNAL(timeout()), this, SLOT(startJobs()));

	SDPASSERT(ParameterManager::instancePtr());

	//! Copy the maximum simultaneous thread execution number...
	//! This value will be incremented and decremented by signals from jobs's
	//! process starting and terminating.
	__remainingSlots = ParameterManager::instancePtr()->parameter("Config-MaxThreads").toInt();
}


ActivityPanel::~ActivityPanel() {

	if ( __timer->isActive() )
	    __timer->stop();
}


void ActivityPanel::addJob(Job* j) {

	SDPASSERT(j);

	if ( __queue.contains(j) ) return;

	//! Assign the new job pointer to the TableItem object
	TableItem* itm = new TableItem(j->creationTime().toString("yyyy-MM-dd HH:mm:ss.zzz"), (void*) j);
	//! Vice-versa, [...] this small hack will be useful whenever we need to
	//! retrieve one from the other...
	j->setTableWidget((void*) itm);

	QTableWidgetItem* typeItm = new QTableWidgetItem((*j->type()).string);
	QTableWidgetItem* statusItm = new QTableWidgetItem((*j->status()).string);
	QTableWidgetItem* idItm = new QTableWidgetItem(j->id());

	typeItm->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	itm->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	statusItm->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	idItm->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	QList<QTableWidgetItem*> l;
	l << itm;
	l << typeItm;
	l << new QTableWidgetItem(j->information());
	l << new QTableWidgetItem(j->comment());
	l << statusItm;
	l << idItm;

	__table->insertRow(__table->rowCount());
	for (int i = 0; i < __table->columnCount(); ++i) {
		l.at(i)->setToolTip(j->tooltip());
		__table->setItem(__table->rowCount() - 1, i, l.at(i));
	}
	__table->resizeColumnsToContents();
	__table->resizeRowsToContents();

	__queue << j;

	//! Add new job in selected queue if the queue has been started in
	//! RunnningAll mode...
	if ( __status == RunningAll ) {
		updateJobStatus(j, jqsScheduled);
		__selectedQueue << j;
	}

	connect(j, SIGNAL(started()), this, SLOT(jobStarted()));
	connect(j, SIGNAL(terminated()), this, SLOT(jobTerminated()));

	//! Visual trick... instantiate some blinker from the mainframe ;)
	SDPASSERT(MainFrame::instancePtr());
	MainFrame::instancePtr()->newJobToManage();

	//! Only store job in database if it is brand new, because a re-entrant
	//! job (from database reading) doesn't need to be updated...
	SDPASSERT(DatabaseManager::instancePtr());
	if ( DetectionJob* det = dynamic_cast<DetectionJob*>(j) ) {
		if ( !DatabaseManager::instancePtr()->detectionExists(j->id()) )
		    DatabaseManager::instancePtr()->commitDetection(det);
	}
	else if ( DispatchJob* dis = dynamic_cast<DispatchJob*>(j) ) {
		if ( !DatabaseManager::instancePtr()->dispatchExists(j->id()) )
		    DatabaseManager::instancePtr()->commitDispatch(dis);
	}
}


void ActivityPanel::removeJob(Job* j) {

	SDPASSERT(Cache::instancePtr());
	SDPASSERT(DatabaseManager::instancePtr());
	SDPASSERT(TriggerPanel::instancePtr());
	SDPASSERT(RecentPanel::instancePtr());

	if ( __selectedQueue.contains(j) )
	    __selectedQueue.removeOne(j);

	for (int i = 0; i < __queue.size(); ++i)
		if ( __queue.at(i) == j ) {
			//! INFO: This may be uncalled for -> the QProcess tends to signal
			//! properly that it has been destroyed by gently yelling from
			//! the top of its lungs... ;)
			//! TODO: Re-Check that theory later!
			if ( j->status() == Running )
			    j->stop();

			if ( DetectionJob* det = dynamic_cast<DetectionJob*>(j) ) {
				//! Remove children
				TriggerPanel::instancePtr()->removeTriggers(j->id());

				RecentPanel::instancePtr()->removeJobs(QList<QString>() << j->id());

				//! Remove job from database first
				DatabaseManager::instancePtr()->removeDetection(det);
			}
			else if ( DispatchJob* dis = dynamic_cast<DispatchJob*>(j) ) {
				DatabaseManager::instancePtr()->removeDispatch(dis);
			}

			__queue.removeOne(j);
			Cache::instancePtr()->removeObject(j->id(), false, false);
		}
}


bool ActivityPanel::saveParameters() {
	return true;
}


quint32 ActivityPanel::remainingJobs() {

	quint32 count = 0;
	for (int i = 0; i < __queue.size(); ++i)
		if ( __queue.at(i)->status() == Running )
		    ++count;

	return count;
}


bool ActivityPanel::autoArchiveJobs() {
	return __archiveButton->isChecked();
}


void ActivityPanel::removeJobs(QList<QString> jobs) {

	JobItems itms = allJobs();
	JobItems selection;
	for (int i = 0; i < itms.size(); ++i) {
		if ( !jobs.contains(itms.at(i).job->id()) ) continue;
		selection << itms[i];
	}

	QList<TableItem*> sel;
	for (int i = 0; i < selection.size(); ++i) {
		if ( selection.at(i).job->status() == Running ) {
			selection.at(i).job->stop();
			Utils::responsiveDelay(2000);
		}
		__selectedQueue.removeOne(selection.at(i).job);
		__queue.removeOne(selection.at(i).job);
		sel << selection.at(i).item;
	}

	Logger::instancePtr()->addMessage(Logger::DEBUG, __func__,
	    QString("Removing %1 job(s) from panel.").arg(sel.size()));

	removeTableRow(__table, sel);
}


void ActivityPanel::initInteractiveTable() {

	QFont font;
	font.setPointSize(9);
#ifdef Q_OS_MAC
	font.setPointSize(11);
#endif

	QVBoxLayout* l = new QVBoxLayout(__ui->widgetTable);
	l->setMargin(0);
	l->setSpacing(0);

	QStringList headers;
	for (size_t i = 0; i < ActivityHeadersString.size(); ++i) {
		headers << ActivityHeadersString[i];
		QAction* a = new QAction(tr(ActivityHeadersString[i]), this);
		a->setCheckable(true);
		a->setChecked(true);
		connect(a, SIGNAL(triggered()), this, SLOT(showHideHeaderItems()));
		__actions.append(PairedAction(static_cast<int>(i), a));
	}

	__table = new InteractiveTable(__ui->widgetTable);
	__table->setObjectName(QString::fromUtf8("tableWidgetJobs"));
	__table->setFont(font);
	__table->setFrameShape(QFrame::NoFrame);
	__table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	__table->setProperty("showDropIndicator", QVariant(false));
	__table->setDragDropOverwriteMode(false);
	__table->setAlternatingRowColors(false);
	__table->setSelectionBehavior(QAbstractItemView::SelectRows);
	__table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	__table->setShowGrid(true);
	__table->setGridStyle(Qt::SolidLine);
	__table->setSortingEnabled(true);
	__table->verticalHeader()->setVisible(false);
	__table->setColumnCount(headers.size());
	__table->setHorizontalHeaderLabels(headers);
	__table->setStyleSheet("QTableView {selection-background-color: rgb(0,0,0,100);}");
	__table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	__table->resizeColumnsToContents();

	__table->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(__table->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)),
	    this, SLOT(headerMenu(const QPoint&)));
	connect(__table, SIGNAL(cellClicked(int, int)), this, SLOT(showJobOutput(const int&, const int&)));
	connect(__table->selectionModel(),
	    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this,
	    SLOT(jobSelectionChanged(const QItemSelection&, const QItemSelection&)));
	connect(__table, SIGNAL(deleteSelected()), this, SLOT(removeJobs()));

	l->addWidget(__table);
}


JobItems ActivityPanel::selectedJobs() {

	QList<QTableWidgetItem*> l = __table->selectedItems();
	if ( l.isEmpty() ) return JobItems();

	QList<TableItem*> sel;
	for (int i = 0; i < l.size(); ++i) {
		TableItem* itm = dynamic_cast<TableItem*>(l.at(i));
		if ( !itm ) continue;
		sel << itm;
	}

	JobItems itms;
	for (int i = 0; i < sel.size(); ++i) {
		Job* job = reinterpret_cast<Job*>(sel.at(i)->object);
		if ( !job ) continue;
		JobItem ji;
		ji.item = sel.at(i);
		ji.job = job;
		itms << ji;
	}

	return itms;
}


JobItems ActivityPanel::allJobs() {

	JobItems items;
	for (int i = 0; i < __table->rowCount(); ++i) {

		TableItem* itm = dynamic_cast<TableItem*>(__table->item(i, 0));
		if ( !itm ) continue;
		Job* job = reinterpret_cast<Job*>(itm->object);
		if ( !job ) continue;
		JobItem ji;
		ji.item = itm;
		ji.job = job;
		items << ji;
	}

	return items;
}


void ActivityPanel::commitJobsIntoDB() {

	SDPASSERT(DatabaseManager::instancePtr());
	SDPASSERT(Logger::instancePtr());

	Logger::instancePtr()->addMessage(Logger::DEBUG, __func__,
	    QString("Jobs stored in database: %1.")
	        .arg(QString::number(__queue.count())), true);

	for (int i = 0; i < __queue.size(); ++i)
		if ( DetectionJob* det = dynamic_cast<DetectionJob*>(__queue.at(i)) )
			DatabaseManager::instancePtr()->commitDetection(det);
		else if ( DispatchJob* dis = dynamic_cast<DispatchJob*>(__queue.at(i)) )
		    DatabaseManager::instancePtr()->commitDispatch(dis);
}


void ActivityPanel::archiveJobs() {

	if ( !__archiveButton->isChecked() ) return;
	SDPASSERT(RecentPanel::instancePtr());
	SDPASSERT(DatabaseManager::instancePtr());

	RecentPanel* rp = RecentPanel::instancePtr();
	DatabaseManager* db = DatabaseManager::instancePtr();

	for (int i = 0; i < __queue.size(); ++i) {
		if ( __queue.at(i)->status() != Terminated ) continue;

		//! Save this instance of the object in db first because RecentPanel
		//! won't update job's status for us. Job will end up being loaded
		//! as if it hadn't been executed before...
		if ( DetectionJob* det = dynamic_cast<DetectionJob*>(__queue.at(i)) )
			db->commitDetection(det);
		else if ( DispatchJob* dis = dynamic_cast<DispatchJob*>(__queue.at(i)) )
		    db->commitDispatch(dis);

		rp->loadJob(__queue.at(i));
	}

	QList<TableItem*> sel;
	for (int i = 0; i < __table->rowCount(); ++i) {
		TableItem* itm = dynamic_cast<TableItem*>(__table->item(i, getHeaderPosition(aCTIME)));
		if ( !itm ) continue;
		sel << itm;
	}

	QList<TableItem*> parents;
	for (int i = 0; i < sel.size(); ++i) {
		Job* job = reinterpret_cast<Job*>(sel.at(i)->object);
		if ( !job ) continue;
		if ( job->status() != Terminated ) continue;
		__queue.removeOne(job);
		parents << sel.at(i);
	}

	removeTableRow(__table, parents);
}


void ActivityPanel::headerMenu(const QPoint&) {

	QMenu menu(this);
	for (HeaderActions::const_iterator it = __actions.constBegin();
	        it != __actions.constEnd(); ++it)
		menu.addAction((*it).second);
	menu.exec(QCursor::pos());
}


void ActivityPanel::showHideHeaderItems() {

	for (HeaderActions::const_iterator it = __actions.constBegin();
	        it != __actions.constEnd(); ++it)
		((*it).second->isChecked()) ?
		    __table->showColumn((*it).first)
		        : __table->hideColumn((*it).first);
}


void ActivityPanel::startQueue() {

	if ( __status != Idle ) {
		QMessageBox::information(this, tr("Information"),
		    tr("A session is already running, you may stop it "
			    "or let it finish before starting a new one."));
		return;
	}

	SDPASSERT(MainFrame::instancePtr());
	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(Logger::instancePtr());
	SDPASSERT(__timer);

	if ( __queue.isEmpty() ) return;

	__selectedQueue.clear();
	__remainingSlots = ParameterManager::instancePtr()->parameter("Config-MaxThreads").toInt();


	JobItems jobs = selectedJobs();
	__status = RunningSelected;

	if ( jobs.isEmpty() ) {
		__status = RunningAll;
		jobs = allJobs();
	}

	for (int i = 0; i < jobs.size(); ++i) {
		if ( !jobs.at(i).job ) continue;
		if ( jobs.at(i).job->status() != Pending
		    && jobs.at(i).job->status() != Stopped )
		    continue;
		updateJobStatus(jobs.at(i).job, jqsScheduled);
		__selectedQueue << jobs.at(i).job;

		Logger::instancePtr()->addMessage(Logger::INFO, __func__,
		    "Scheduled " + jobs.at(i).job->id() + " for execution run", false);
	}

	if ( __selectedQueue.isEmpty() ) {
		Logger::instancePtr()->addMessage(Logger::DEBUG, __func__,
		    QString("Starting queue failed:, %1 slot(s) are available but to no job has been queued.")
		        .arg(QString::number(__remainingSlots)));
		__status = Idle;
		return;
	}

	__timer->start(1000);

	Logger::instancePtr()->addMessage(Logger::DEBUG, __func__,
	    QString("Starting queue, %1 slot(s) are available, %2 job(s) to process.")
	        .arg(QString::number(__remainingSlots))
	        .arg(QString::number(__selectedQueue.count())));
}


void ActivityPanel::stopQueue() {

	SDPASSERT(__timer);
	SDPASSERT(Logger::instancePtr());
	SDPASSERT(MainFrame::instancePtr());

	if ( __status == Idle ) return;

	JobItems jobs = selectedJobs();
	if ( jobs.isEmpty() ) {

		//! stop the entire queue
		for (int i = 0; i < __table->rowCount(); ++i) {
			TableItem* itm = dynamic_cast<TableItem*>(__table->item(i, 0));
			if ( !itm ) continue;
			Job* job = reinterpret_cast<Job*>(itm->object);
			if ( !job ) continue;
			if ( job->status() != Running ) continue;
			job->stop();
			updateJobStatus(job);
			Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
			    "Stopped job " + job->id(), false);
		}
	}
	else {
		//! Stop only selected jobs
		for (int i = 0; i < jobs.size(); ++i) {
			if ( !jobs.at(i).job ) continue;
			if ( jobs.at(i).job->status() != Running ) continue;
			jobs.at(i).job->stop();
			updateJobStatus(jobs.at(i).job);
			Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
			    "Stopped job " + jobs.at(i).job->id(), false);
		}
	}


	int pending = 0;
	int running = 0;
	for (int i = 0; i < __queue.count(); ++i) {
		if ( __queue.at(i)->status() == Running ) ++running;
		if ( __queue.at(i)->status() == Pending ) ++pending;
	}

	if ( running == 0 ) {
		__timer->stop();
		__status = Idle;
		const QString msg = "Session terminated, " + QString::number(pending)
		    + " jobs still await processing";

		Logger::instancePtr()->addMessage(Logger::DEBUG, __func__, msg);
		MainFrame::instancePtr()->newStatusMessage(msg);
	}
}


void ActivityPanel::removeJobs() {

	QList<QTableWidgetItem*> l = __table->selectedItems();

	//! Remove only selected items
	if ( l.size() != 0 ) {

		QList<TableItem*> sel;
		for (int i = 0; i < l.size(); ++i) {
			TableItem* itm = dynamic_cast<TableItem*>(l.at(i));
			if ( !itm ) continue;
			sel << itm;
		}

		QList<TableItem*> parents;
		for (int i = 0; i < sel.size(); ++i) {
			Job* job = reinterpret_cast<Job*>(sel.at(i)->object);
			if ( !job ) continue;
			if ( job->status() == Running )
			    if ( QMessageBox::question(this, tr("Status check"),
			        QString("%1 is currently running. Remove it anyway ?").arg(job->id()),
			        QMessageBox::Yes | QMessageBox::Cancel)
			        == QMessageBox::Cancel )
			        continue;

			removeJob(job);
			parents << sel.at(i);
		}

		removeTableRow(__table, parents);
	}

	//! Remove all items
	else {

		QList<TableItem*> parents;
		bool yesToAll = false;
		bool noToAll = false;
		for (int i = 0; i < __table->rowCount(); ++i) {
			TableItem* itm = dynamic_cast<TableItem*>(__table->item(i, 0));
			if ( !itm ) continue;

			Job* job = reinterpret_cast<Job*>(itm->object);
			if ( !job ) continue;

			if ( job->status() == Running ) {

				if ( !yesToAll && !noToAll ) {
					QMessageBox::StandardButton b =
					    QMessageBox::question(this, tr("Status check"),
					        QString("Job with id %1 is still running, remove it anyway ?")
					            .arg(job->id()), QMessageBox::Yes | QMessageBox::YesToAll |
					            QMessageBox::NoToAll | QMessageBox::No);
					if ( b == QMessageBox::Yes ) {
						removeJob(job);
						parents << itm;
					}
					else if ( b == QMessageBox::YesToAll ) {
						removeJob(job);
						parents << itm;
						yesToAll = true;
					}
					else if ( b == QMessageBox::No )
						continue;
					else if ( b == QMessageBox::NoToAll ) {
						noToAll = true;
						continue;
					}
				}
				else {
					if ( yesToAll && !noToAll ) {
						removeJob(job);
						parents << itm;
					}
					if ( !yesToAll && noToAll )
					    continue;
				}
			}
			else {
				removeJob(job);
				parents << itm;
			}
		}
		removeTableRow(__table, parents);
	}

	SDPASSERT(Logger::instancePtr());
	Logger::instancePtr()->addMessage(Logger::DEBUG, __func__,
	    QString("Available thread slot(s): %1").arg(QString::number(__remainingSlots)));
}


void ActivityPanel::resetJobs() {

	bool yesToAll = false;
	bool noToAll = false;
	QStringList reseted;
	JobItems items = selectedJobs();
	for (int i = 0; i < items.size(); ++i) {

		if ( !yesToAll && !noToAll ) {

			QMessageBox::StandardButton b = QMessageBox::question(this, tr("Reseting job"),
			    QString("Job with id %1 will be reseted and all its dependencies removed.\nAre you sure ?")
			        .arg(items.at(i).job->id()), QMessageBox::Yes | QMessageBox::YesToAll |
			        QMessageBox::NoToAll | QMessageBox::No);

			if ( b == QMessageBox::Yes ) {
				reseted << items.at(i).job->id();
				items.at(i).job->reset(true);
				__table->item(items.at(i).item->row(), getHeaderPosition(aSTATUS))->setText((*items.at(i).job->status()).string);
				__table->item(items.at(i).item->row(), getHeaderPosition(aSTATUS))->setForeground(Qt::black);
			}
			else if ( b == QMessageBox::YesToAll ) {
				reseted << items.at(i).job->id();
				items.at(i).job->reset(true);
				__table->item(items.at(i).item->row(), getHeaderPosition(aSTATUS))->setText((*items.at(i).job->status()).string);
				yesToAll = true;
			}
			else if ( b == QMessageBox::No ) {
				continue;
			}
			else if ( b == QMessageBox::NoToAll ) {
				noToAll = true;
				continue;
			}
		}
		else {
			if ( yesToAll && !noToAll ) {
				reseted << items.at(i).job->id();
				items.at(i).job->reset(true);
				__table->item(items.at(i).item->row(), getHeaderPosition(aSTATUS))->setText((*items.at(i).job->status()).string);
			}
			if ( !yesToAll && noToAll )
			    continue;
		}
	}
}


void ActivityPanel::startJobs() {

	SDPASSERT(Logger::instancePtr());
	SDPASSERT(MainFrame::instancePtr());
	SDPASSERT(ParameterManager::instancePtr());

	ParameterManager* pm = ParameterManager::instancePtr();

	int diskError = 0;
	int runDirError = 0;
	int started = 0;
	for (int i = 0; i < __selectedQueue.size(); ++i) {

		Job* job = __selectedQueue.at(i);

		if ( __remainingSlots < 1 ) continue;

		if ( job->status() != Pending && job->status() != Stopped )
		    continue;

		//! Skip job until user handles space situation(s)
		if ( !pm->parameter("Config-RunDirSizeOK").toBool() ) {
			updateJobStatus(job, jqsSkipped_RunDirError);
			runDirError++;
			continue;
		}
		if ( !pm->parameter("Config-DiskspaceOK").toBool() ) {
			updateJobStatus(job, jqsSkipped_DiskError);
			diskError++;
			continue;
		}

		job->run();
		started++;

		MainFrame::instancePtr()->newStatusMessage("Started job " + job->id() + ".");

		//! INFO: Wait a lil bit for the job to emit started() signal, slot(s)
		//! available will be decremented upon it.
		Utils::responsiveDelay(2000);
	}

	if ( __selectedQueue.isEmpty() ) {
		Logger::instancePtr()->addMessage(Logger::DEBUG, __func__,
		    "No more jobs to execute, stopping session");
		stopQueue();
		return;
	}

	if ( diskError != 0 && started == 0 ) {
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__,
		    "Session couldn't start: insufficient disk space.");
		stopQueue();
		return;
	}
	else if ( diskError != 0 && started != 0 ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Session started but some jobs have been postponed due to insufficient disk space.");
	}

	if ( runDirError != 0 && started == 0 ) {
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__,
		    "Session couldn't start: root run directory reached max size.");
		stopQueue();
		return;
	}
	else if ( runDirError != 0 && started != 0 ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Session started but some jobs have been postponed because root run dir size has reached its maximum.");
	}
}


void ActivityPanel::jobStarted() {

	QObject* sender = QObject::sender();

	Job* job = qobject_cast<Job*>(sender);
	if ( !job ) return;

	__remainingSlots--;
	updateJobStatus(job);
}


void ActivityPanel::jobTerminated() {

	QObject* sender = QObject::sender();

	Job* job = qobject_cast<Job*>(sender);
	if ( !job ) return;

	__remainingSlots++;
	updateJobStatus(job);

	if ( job->type() == Detection ) {
		if ( DetectionJob* dj = dynamic_cast<DetectionJob*>(job) )
		    //! TODO: perform this with thread pool
		    checkDetectionOutput(dj);
	}

	else if ( job->type() == Dispatch ) {
		//! Check out the result of the operation : see if anything went well,
		//! how many files has been processed and so on...
	}

	if ( __jobSelected == job ) {
		if ( job->status() == Terminated ) {
			__ui->labelStartTime->setText(job->runStartTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
			__ui->labelEndTime->setText(job->runEndTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
			__ui->labelDuration->setText(Utils::elapsedTime(job->runStartTime(), job->runEndTime()));
			__ui->frameJobInformation->show();
		}
	}

	//! Dequeue the job
	__selectedQueue.removeOne(job);

	if ( __selectedQueue.isEmpty() )
	    stopQueue();

	//! Archive terminated jobs if asked to
	if ( __archiveButton->isChecked() )
	    archiveJobs();
}


void ActivityPanel::incomingStdMsg() {

	QObject* sender = QObject::sender();

	Job* job = selectedJob();

	if ( sender != job ) return;

	displayJobOutput(job);
}


void ActivityPanel::jobSelectionChanged(const QItemSelection& selected,
                                        const QItemSelection&) {

	if ( !selected.isEmpty() ) {
		__runButton->setText("Run selected");
		__stopButton->setText("Stop selected");
		__removeButton->setText("Remove selected");
		__resetButton->setEnabled(true);
		__editButton->setEnabled(true);
	}
	else {
		__runButton->setText("Run");
		__stopButton->setText("Stop");
		__removeButton->setText("Clear");
		__ui->textEditJobs->clear();
		__ui->labelJobName->setText("");
		__ui->frameJobInformation->hide();
		__resetButton->setEnabled(false);
		__editButton->setEnabled(false);
	}

	__runButton->redraw();
	__stopButton->redraw();
	__removeButton->redraw();
	__resetButton->redraw();
}


void ActivityPanel::showJobOutput(const int& row, const int& col) {

	Q_UNUSED(col);
	TableItem* itm = dynamic_cast<TableItem*>(__table->item(row, 0));
	if ( !itm ) return;

	Job* job = reinterpret_cast<Job*>(itm->object);
	if ( !job ) return;

	if ( job->status() == Running ) {
		if ( __jobSelected ) {
			if ( __jobSelected != job ) {
				disconnect(__jobSelected, SIGNAL(newStdMsg()), this, SLOT(incomingStdMsg()));
				__jobSelected = job;
				connect(job, SIGNAL(newStdMsg()), this, SLOT(incomingStdMsg()));
			}
		}
		else {
			__jobSelected = job;
			connect(job, SIGNAL(newStdMsg()), this, SLOT(incomingStdMsg()));
		}
	}

	displayJobOutput(job);
}


void ActivityPanel::editJob() {

	Job* job = selectedJob();
	if ( !job ) return;

	if ( QMessageBox::question(this, tr("Job edition"),
	    QString("You are about to edit job %1, are you sure ?").arg(job->id()),
	    QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Cancel )
	    return;

	MainFrame* main = MainFrame::instancePtr();
	if ( DetectionJob* det = dynamic_cast<DetectionJob*>(job) ) {
		main->setActivePanel(MainFrame::pDetection);
		emit editDetectionJob(det);
	}
	else if ( DispatchJob* dis = dynamic_cast<DispatchJob*>(job) ) {
		main->setActivePanel(MainFrame::pDispatch);
		emit editDispatchJob(dis);
	}

	//! Cause we've already ruled out the fact that there is only one Job
	//! selected we can use this...
	JobItems selected = selectedJobs();
	removeTableRow(__table, QList<TableItem*>() << selected[0].item);
	removeJob(job);
}


Job* ActivityPanel::selectedJob() {

	QList<QTableWidgetItem*> l = __table->selectedItems();
	QList<TableItem*> selected;

	for (int i = 0; i < l.size(); ++i) {
		TableItem* itm = dynamic_cast<TableItem*>(l.at(i));
		if ( !itm ) continue;
		selected << itm;
	}

	if ( selected.size() != 1 ) return NULL;

	Job* job = reinterpret_cast<Job*>(selected[0]->object);
	if ( !job ) return NULL;

	return job;
}


void ActivityPanel::displayJobOutput(Job* job) {

	if ( !job ) return;
	__ui->textEditJobs->clear();
	__ui->labelJobName->setText(QString("Job %1").arg(job->id()));
	if ( job->status() == Terminated ) {
		__ui->labelStartTime->setText(job->runStartTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
		__ui->labelEndTime->setText(job->runEndTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
		__ui->labelDuration->setText(Utils::elapsedTime(job->runStartTime(), job->runEndTime()));
		__ui->frameJobInformation->show();
	}
	else {
		__ui->labelStartTime->setText("");
		__ui->labelEndTime->setText("");
		__ui->labelDuration->setText("");
	}

	for (Job::StandardOutput::const_iterator it = job->stdOutput().constBegin();
	        it != job->stdOutput().constEnd(); ++it) {
		if ( (*it).first == Job::Info ) {
			__ui->textEditJobs->append(
			    QString("<font color=\"black\">%1</font>")
			        .arg((*it).second));
		}
		else
			__ui->textEditJobs->append(
			    QString("<font color=\"red\">%1</font>")
			        .arg((*it).second));
	}
}


void ActivityPanel::updateJobStatus(Job* job, JobQueueStatus jqs,
                                    const QString& stat) {

	if ( !job ) return;

	TableItem* itm = reinterpret_cast<TableItem*>(job->tableWidget());
	if ( !itm ) return;

	QTableWidgetItem* status = __table->item(itm->row(), getHeaderPosition(aSTATUS));
	if ( !status ) return;

	QString msg;
	switch ( jqs ) {
		case jqsScheduled:
			msg = "Scheduled";
			break;
		case jqsRunning:
			msg = "Running";
			break;
		case jqsSkipped_DiskError:
			msg = "Skipped: insufficient diskspace";
			break;
		case jqsSkipped_RunDirError:
			msg = "Skipped: insufficient rundir space";
			break;
		case jqsCustom:
			msg = stat;
			break;
		case jqsNone:
			break;
	}

	QFont f(status->font());
	f.setBold(true);

	if ( job->status() == Stopped )
		status->setForeground(Qt::darkYellow);
	else if ( job->status() == Terminated )
		(job->runExitCode() == 0) ? status->setForeground(Qt::darkGreen) :
		    status->setForeground(Qt::red);
	else if ( job->status() == Running )
	    status->setForeground(Qt::black);
	if ( !msg.isEmpty() )
	    status->setForeground(Qt::black);

	status->setFont(f);
	status->setText((msg.isEmpty()) ? (*job->status()).string : msg);


	if ( job->status() == Running ) {

		status->setText("");

		if ( __table->cellWidget(itm->row(), getHeaderPosition(aSTATUS)) )
		    return;

		QFont f(status->font());
#ifdef Q_OS_MAC
		f.setPointSize(11);
#endif

		__table->setCellWidget(itm->row(), getHeaderPosition(aSTATUS),
		    new AnimInfoWidget(__table, ":images/loader.gif",
		        (msg.isEmpty()) ? (*job->status()).string : msg, f));
	}
	else {
		if ( __table->cellWidget(itm->row(), getHeaderPosition(aSTATUS)) )
		    __table->removeCellWidget(itm->row(), getHeaderPosition(aSTATUS));
	}
}


void ActivityPanel::checkDetectionOutput(DetectionJob* job) {
	if ( !job ) return;
	emit detectionJobTerminated(job);
	SDPASSERT(MainFrame::instancePtr());
	MainFrame::instancePtr()->newStatusMessage(job->id() + " is terminated");
}


CommitDialog::CommitDialog(const QString& file, QWidget* parent) :
		QDialog(parent), __ui(new Ui::CommitDialog), __retCode(cdUNKNOWN) {

	__ui->setupUi(this);
	__ui->labelStatus->setAutoFillBackground(true);
	__ui->labelInfo->setText("Committing triggers in file " + file);

	connect(__ui->pushButton, SIGNAL(clicked()), this, SLOT(close()));

	SDPASSERT(Logger::instancePtr());
	SDPASSERT(ParameterManager::instancePtr());
	ParameterManager* pm = ParameterManager::instancePtr();

	QProcess* proc = new QProcess(this);
	connect(proc, SIGNAL(finished(int)), this, SLOT(processTerminated(int)));

	proc->start(QString("%1 %2").arg(pm->parameter("BASH_BIN").toString()).arg(file));
	if ( !proc->waitForStarted() ) {
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__, "Failed to start commit operation");
		QMessageBox::critical(this, tr("Error"), tr("Failed to start commit operation!"));
	}
}


CommitDialog::~CommitDialog() {}


CommitDialog::ReturnCode CommitDialog::commitReturnCode() {
	return __retCode;
}


void CommitDialog::processTerminated(int retcode) {

	QObject* sender = QObject::sender();
	SDPASSERT(sender);

	QProcess* proc = qobject_cast<QProcess*>(sender);
	if ( !proc ) return;

	QColor c;
	if ( retcode ) {
		__retCode = cdFAILED;
		__ui->labelStatus->setText("Program did not executed successfully.");
		c = QColor(228, 59, 54);
	}
	else {
		__retCode = cdSUCCESS;
		__ui->labelStatus->setText("Program executed successfully.");
		c = QColor(99, 229, 99);
	}

	QPalette p = __ui->labelStatus->palette();
	p.setColor(QPalette::Background, c);
	__ui->labelStatus->setPalette(p);

	__ui->textEdit->clear();
	QByteArray out = proc->readAll();
	QList<QByteArray> output = out.split('\n');
	for (QList<QByteArray>::iterator it = output.begin();
	        it != output.end(); ++it) {
		if ( it->isEmpty() ) continue;
		__ui->textEdit->append((*it).constData());
	}

	//! TODO: see if this doesn't provoque a crash
	proc->deleteLater();
}


TriggerPanel::TriggerPanel(QWidget* parent) :
		PanelWidget(parent), __ui(new Ui::TriggerPanel) {

	__ui->setupUi(this);

	setHeader(tr("Trigger validation"));
	setDescription(tr("Analyze and validate triggers."));

	initInteractiveTable();

	//! Re-organize splitter so that the view get initialized half by half
	__ui->splitterMain->setSizes(QList<int>() << 3 << 1);

	QVBoxLayout* l = new QVBoxLayout(__ui->widgetToolbar);
	l->setContentsMargins(0, 0, 0, 4);
	FancyPanel* toolbar = new FancyPanel(this, FancyPanel::Horizontal, 65, FancyPanel::Gray);
	toolbar->setSelectionMode(FancyPanel::SingleFreeSelection);

	__acceptButton = new FancyButton(QPixmap(":images/accept.png"), tr("Accept"), toolbar);
	__rejectButton = new FancyButton(QPixmap(":images/reject.png"), tr("Reject"), toolbar);
	__removeButton = new FancyButton(QPixmap(":images/clear.png"), tr("Clear"), toolbar);
	__commitButton = new FancyButton(QPixmap(":images/commit.png"), tr("Commit"), toolbar);
	__resetButton = new FancyButton(QPixmap(":images/reset.png"), tr("Reset"), toolbar);
	__autocleanButton = new FancyButton(QPixmap(":images/autoclean.png"), tr("Auto clean"), toolbar);

	__acceptButton->setCheckable(false);
	__rejectButton->setCheckable(false);
	__removeButton->setCheckable(false);
	__commitButton->setCheckable(false);
	__resetButton->setCheckable(false);
	__autocleanButton->setCheckable(true);

	__acceptButton->setToolTip(tr("Validates selected trigger(s)"));
	__rejectButton->setToolTip(tr("Rejects selected trigger(s)"));
	__removeButton->setToolTip(tr("Removes selected trigger(s)"));
	__commitButton->setToolTip(tr("Commit selected trigger(s)"));
	__resetButton->setToolTip(tr("Reset selected trigger(s)"));
	__autocleanButton->setToolTip(tr("Cleans up committed triggers from the list"));

	toolbar->addFancyButton(__acceptButton);
	toolbar->addFancyButton(__rejectButton);
	toolbar->addFancyButton(__removeButton);
	toolbar->addFancyButton(__commitButton);
	toolbar->addFancyButton(__resetButton);
	toolbar->addFancyButton(__autocleanButton);

	connect(__acceptButton, SIGNAL(clicked()), this, SLOT(acceptTriggers()));
	connect(__rejectButton, SIGNAL(clicked()), this, SLOT(rejectTriggers()));
	connect(__removeButton, SIGNAL(clicked()), this, SLOT(removeTriggers()));
	connect(__commitButton, SIGNAL(clicked()), this, SLOT(commitTriggers()));
	connect(__resetButton, SIGNAL(clicked()), this, SLOT(resetTriggers()));
	connect(__autocleanButton, SIGNAL(clicked()), this, SLOT(autocleanTriggers()));

	l->addWidget(toolbar);

	QFont tbf(toolbar->font());
	tbf.setPointSize(10);
#ifdef Q_OS_MAC
	tbf.setPointSize(13);
#endif
	toolbar->setFont(tbf);

	//! Start with buttons deactivated
	__acceptButton->setEnabled(false);
	__rejectButton->setEnabled(false);
	__removeButton->setEnabled(false);
	__commitButton->setEnabled(false);
	__resetButton->setEnabled(false);
}


TriggerPanel::~TriggerPanel() {
	__commitables.clear();
}


bool TriggerPanel::saveParameters() {
	return true;
}


void TriggerPanel::loadTrigger(Trigger* trig) {

	if ( !trig ) return;

	if ( !__triggers.contains(trig) )
	    __triggers << trig;

	QFont f(__table->font());
	f.setBold(true);

	//! Assign the new job pointer to the TableItem object
	TableItem* ctime = new TableItem(trig->creationTime().toString("yyyy-MM-dd HH:mm:ss.zzz"), (void*) trig);
	//! Vice-versa, [...] this small hack will be useful whenever we need to
	//! retrieve one from the other...
	trig->setTableWidget((void*) ctime);

	QTableWidgetItem* otime = new QTableWidgetItem(trig->originTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	QTableWidgetItem* status = new QTableWidgetItem((*(trig->status())).string);

	int idx = 0;
	QString stations = "[" + QString::number(trig->stations().size()) + "] : ";

	for (int i = 0; i < trig->stations().size(); ++i, ++idx)
		if ( idx != trig->stations().size() - 1 )
			stations += trig->stations().at(i).networkCode + "." + trig->stations().at(i).code + ", ";
		else
			stations += trig->stations().at(i).networkCode + "." + trig->stations().at(i).code;

	QTableWidgetItem* sites = new QTableWidgetItem(stations);
	QTableWidgetItem* tid = new QTableWidgetItem(trig->id());

	ctime->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	otime->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	status->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	tid->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	status->setFont(f);

	QString tooltip;
	tooltip += "<div><p>";
	tooltip += "<b>Job</b>: " + trig->jobID() + "<br />";
	tooltip += "<b>Creation time</b>: " + trig->creationTime().toString("yyyy-MM-dd HH:mm:ss.zzz") + "<br />";
	tooltip += "<b>Trigger time</b>: " + trig->originTime().toString("yyyy-MM-dd HH:mm:ss.zzz") + "<br />";
	tooltip += "<b>Stations</b>: " + stations + "<br />";
	tooltip += "<b>Resume</b>: " + trig->resumePage() + "<br />";
	tooltip += "</p></div>";

	QList<QTableWidgetItem*> l;
	l << ctime;
	l << otime;
	l << status;
	l << sites;
	l << tid;

	__table->insertRow(__table->rowCount());
	for (int i = 0; i < __table->columnCount(); ++i) {
		l.at(i)->setToolTip(tooltip);
		__table->setItem(__table->rowCount() - 1, i, l.at(i));
	}

	__table->resizeColumnsToContents();
	__table->resizeRowsToContents();
}


void TriggerPanel::removeTriggers(const QString& jobID) {

	SDPASSERT(Logger::instancePtr());
	SDPASSERT(Cache::instancePtr());
	Logger* log = Logger::instancePtr();

	QList<TableItem*> items;
	QList<Trigger*> triggers;
	for (int i = 0; i < __table->rowCount(); ++i) {
		TableItem* itm = dynamic_cast<TableItem*>(__table->item(i, 0));
		if ( !itm ) continue;
		Trigger* trig = reinterpret_cast<Trigger*>(itm->object);
		if ( !trig ) continue;
		if ( trig->jobID() == jobID ) {
			triggers << trig;
			items << itm;
		}
	}

	log->addMessage(Logger::DEBUG, __func__, QString("Removing %1 trigger(s) linked to Job %2")
	    .arg(triggers.size()).arg(jobID));

	for (int i = 0; i < triggers.size(); ++i)
		removeTrigger(triggers.at(i));

	removeTableRow(__table, items);
}


void TriggerPanel::initInteractiveTable() {

	QFont font;
	font.setPointSize(9);

#ifdef Q_OS_MAC
	font.setPointSize(11);
#endif

	QVBoxLayout* l = new QVBoxLayout(__ui->widgetTable);
	l->setMargin(0);
	l->setSpacing(0);

	QStringList headers;
	for (size_t i = 0; i < TriggerHeadersString.size(); ++i) {
		headers << TriggerHeadersString[i];
		QAction* a = new QAction(tr(TriggerHeadersString[i]), this);
		a->setCheckable(true);
		a->setChecked(true);
		connect(a, SIGNAL(triggered()), this, SLOT(showHideHeaderItems()));
		__actions.append(PairedAction(static_cast<int>(i), a));
	}

	__table = new InteractiveTable(__ui->widgetTable);
	__table->setObjectName(QString::fromUtf8("tableWidgetTriggers"));
	__table->setMinimumSize(QSize(0, 0));
	__table->setMaximumSize(QSize(16777215, 16777215));
	__table->setFont(font);
	__table->setFrameShape(QFrame::NoFrame);
	__table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	__table->setSelectionBehavior(QAbstractItemView::SelectRows);
	__table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	__table->setSortingEnabled(true);
	__table->verticalHeader()->setVisible(false);
	__table->setColumnCount(headers.size());
	__table->setHorizontalHeaderLabels(headers);
	__table->setStyleSheet("QTableView {selection-background-color: rgb(0,0,0,100);}");
	__table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	__table->resizeColumnsToContents();

	__table->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(__table->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)),
	    this, SLOT(headerMenu(const QPoint&)));
	connect(__table, SIGNAL(cellClicked(int, int)), this, SLOT(showTriggerOutput(const int&, const int&)));
	connect(__table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
	    this, SLOT(triggerSelectionChanged(const QItemSelection&, const QItemSelection&)));
	connect(__table, SIGNAL(deleteSelected()), this, SLOT(removeTriggers()));

	l->addWidget(__table);
}


void TriggerPanel::displayTriggerOutput(Trigger* trig) {

	if ( !trig ) return;

	__ui->webView->load(QUrl::fromLocalFile(trig->resumePage()));
	__ui->textEdit->clear();
	__ui->textEdit->setText(trig->information());
	__ui->labelTriggerID->setText(trig->originTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
}


const QString TriggerPanel::generateSC3ML(Trigger* trig) {

	SDPASSERT(ParameterManager::instancePtr());
	ParameterManager* pm = ParameterManager::instancePtr();

	QString stationCount = QString::number(trig->stations().size());

	QString xml;
	xml += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + ENDL;
	xml += "<seiscomp xmlns=\"http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/0.7\" version=\"0.7\">" + ENDL;
	xml += xmlTAB + "<EventParameters>" + ENDL;

	for (int i = 0; i < trig->stations().size(); ++i) {

		QString id = trig->id() + QString::number(i);
		id.replace("Trigger", "Pick");

		//! Simulate delay
		QDateTime dt = trig->originTime();
		dt.addSecs(i);

		xml += xmlTAB + xmlTAB + "<pick publicID=\"" + id + "\" >" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + "<time>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<value>" + dt.toString("yyyy-MM-ddThh:mm:ss.zzzzzzZ") + "</value>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + "</time>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + "<waveformID networkCode=\"" + trig->stations().at(i).networkCode
		    + "\" stationCode=\"" + trig->stations().at(i).code
		    + "\" locationCode=\"" + trig->stations().at(i).locationCode
		    + "\" channelCode=\"" + trig->stations().at(i).channelCode + "\"/>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + "<phaseHint>P</phaseHint>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + "<evaluationMode>manual</evaluationMode>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + "<creationInfo>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<agencyID>" + pm->parameter("AGENCY").toString() + "</agencyID>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<author>" + pm->parameter("AUTHOR").toString() + "</author>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<creationTime>" + trig->creationTime().toString("yyyy-MM-ddThh:mm:ss.zzzzzzZ") + "</creationTime>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + "</creationInfo>" + ENDL;
		xml += xmlTAB + xmlTAB + "</pick>" + ENDL;
	}

	QString oid = trig->id();
	oid.replace("Trigger", "Origin");

	xml += xmlTAB + xmlTAB + "<origin publicID=\"" + oid + "\">" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "<time>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<value>" + trig->originTime().toString("yyyy-MM-ddThh:mm:ss.zzzzzzZ") + "</value>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "</time>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "<latitude>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<value>" + pm->parameter("Config-DefaultLatitude").toString() + "</value>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "</latitude>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "<longitude>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<value>" + pm->parameter("Config-DefaultLongitude").toString() + "</value>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "</longitude>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "<depth>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<value>10.0</value>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "</depth>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "<methodID>" + pm->parameter("LOC_METHOD_ID").toString() + "</methodID>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "<earthModelID>" + pm->parameter("LOC_EARTH_MODEL_ID").toString() + "</earthModelID>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "<quality>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<associatedPhaseCount>" + stationCount + "</associatedPhaseCount>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<usedPhaseCount>" + stationCount + "</usedPhaseCount>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<associatedStationCount>" + stationCount + "</associatedStationCount>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<usedStationCount>" + stationCount + "</usedStationCount>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<depthPhaseCount>" + stationCount + "</depthPhaseCount>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "</quality>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "<evaluationMode>automatic</evaluationMode>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "<evaluationStatus>confirmed</evaluationStatus>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "<creationInfo>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<agencyID>" + pm->parameter("AGENCY").toString() + "</agencyID>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<author>" + pm->parameter("AUTHOR").toString() + "</author>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<creationTime>" + trig->creationTime().toString("yyyy-MM-ddThh:mm:ss.zzzzzzZ") + "</creationTime>" + ENDL;
	xml += xmlTAB + xmlTAB + xmlTAB + "</creationInfo>" + ENDL;

	for (int i = 0; i < trig->stations().size(); ++i) {

		QString id = trig->id() + QString::number(i);
		id.replace("Trigger", "Pick");

		xml += xmlTAB + xmlTAB + xmlTAB + "<arrival>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<pickID>" + id + "</pickID>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<phase>P</phase>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<azimuth>" + QString::number(i) + "</azimuth>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<distance>" + QString::number(i) + "</distance>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<weight>1</weight>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<creationInfo>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<agencyID>" + pm->parameter("AGENCY").toString() + "</agencyID>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<author>" + pm->parameter("AUTHOR").toString() + "</author>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + xmlTAB + "<creationTime>" + trig->creationTime().toString("yyyy-MM-ddThh:mm:ss.zzzzzzZ") + "</creationTime>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + xmlTAB + "</creationInfo>" + ENDL;
		xml += xmlTAB + xmlTAB + xmlTAB + "</arrival>" + ENDL;
	}

	xml += xmlTAB + xmlTAB + "</origin>" + ENDL;
	xml += xmlTAB + "</EventParameters>" + ENDL;
	xml += "</seiscomp>" + ENDL;

	return xml;
}


void TriggerPanel::updateButtons() {
	__commitButton->setEnabled((__commitables.size()) ? true : false);
}


TriggerItems TriggerPanel::tableSelection() {

	QList<QTableWidgetItem*> l = __table->selectedItems();

	if ( l.size() == 0 ) return TriggerItems();

	QList<TableItem*> sel;
	for (int i = 0; i < l.size(); ++i) {
		TableItem* itm = dynamic_cast<TableItem*>(l.at(i));
		if ( !itm ) continue;
		sel << itm;
	}

	TriggerItems itms;
	for (int i = 0; i < sel.size(); ++i) {
		Trigger* trig = reinterpret_cast<Trigger*>(sel.at(i)->object);
		if ( !trig ) continue;
		TriggerItem t;
		t.item = sel[i];
		t.trigger = trig;
		itms << t;
	}

	return itms;
}


void TriggerPanel::commitTriggersIntoDB() {

	SDPASSERT(DatabaseManager::instancePtr());
	SDPASSERT(Logger::instancePtr());

	DatabaseManager* db = DatabaseManager::instancePtr();

	//! Store triggers only if their parents (jobs) are also in database
	int count = 0;
	for (int i = 0; i < __triggers.size(); ++i)
		if ( db->detectionExists(__triggers.at(i)->jobID()) )
		    db->commitTrigger(__triggers.at(i)), ++count;

	Logger::instancePtr()->addMessage(Logger::DEBUG, __func__,
	    QString("Triggers stored in database: %1.").arg(count), true);
}


void TriggerPanel::headerMenu(const QPoint&) {

	QMenu menu(this);
	for (HeaderActions::const_iterator it = __actions.constBegin();
	        it != __actions.constEnd(); ++it)
		menu.addAction((*it).second);
	menu.exec(QCursor::pos());
}


void TriggerPanel::showHideHeaderItems() {

	for (HeaderActions::const_iterator it = __actions.constBegin();
	        it != __actions.constEnd(); ++it)
		((*it).second->isChecked()) ?
		    __table->showColumn((*it).first) : __table->hideColumn((*it).first);
}


void TriggerPanel::createTrigger(DetectionJob* job) {

	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(Logger::instancePtr());
	SDPASSERT(ActivityPanel::instancePtr());

	ParameterManager* pm = ParameterManager::instancePtr();
	Logger* log = Logger::instancePtr();

	if ( !job ) {
		log->addMessage(Logger::CRITICAL, __func__, "Job's pointer is not valid");
		return;
	}

	//! Security check
	if ( !Utils::dirExists(job->runDir()) ) {
		log->addMessage(Logger::CRITICAL, __func__, "Job with id " + job->id() +
		    " has terminated but no run folder has been found.");
		return;
	}

	QString trigFile = pm->parameter("TRIGGER_FILE").toString();
	trigFile.replace("@JOB_RUN_DIR@", job->runDir());

	if ( !Utils::fileExists(trigFile) ) {
		log->addMessage(Logger::DEBUG, __func__, "Job with id " + job->id() +
		    " has terminated without trigger file.");
		return;
	}

	//! We read triggers.txt file. This file should have only trigger lines.
	//! Each line indicates a trigger...
	//! Ignore duplicate triggers. Duplicate triggers occur due to the method
	//! used : since we employ an overlapping algorithm to perform a proper
	//! analysis of the stream in its full time window...
	QStringList trigFileContent = Utils::getFileContentList(trigFile);

	if ( trigFileContent.size() == 0 ) {
		log->addMessage(Logger::DEBUG, __func__, "Job with id " + job->id() +
		    " has terminated and reported no triggers.");
		return;
	}

	log->addMessage(Logger::DEBUG, __func__, "Job with id " + job->id()
	    + " has terminated and reported " + QString::number(trigFileContent.count())
	    + " trigger(s).");

	//! Generate triggers
	QList<Trigger*> triggers;
	QStringList list;
	for (int i = 0; i < trigFileContent.count(); ++i) {

		QDateTime dt = QDateTime::fromString(trigFileContent.at(i), Qt::ISODate);
		if ( !dt.isValid() ) {
			log->addMessage(Logger::WARNING, __func__, "Wrong datetime format " + trigFileContent.at(i));
			continue;
		}

		Trigger::StationList stList;
		QStringList pics = Utils::getFileList(job->runDir(), ".png", trigFileContent.at(i));

		log->addMessage(Logger::DEBUG, __func__, QString::number(pics.size()) +
		    " snapshot(s) found for trigger " + trigFileContent.at(i) +
		    " originated from job " + job->id());

		for (int j = 0; j < pics.size(); ++j) {

			const QString pic = QString(pics.at(j)).remove(job->runDir());
			QStringList tokens = pic.split('-');
			if ( tokens.size() < 3 ) continue;

			// 2015-05-14T14:17:49.030000Z-WI-MAGL.png (old)
			// 2015-07-21T21:21:09.750000Z-MQ-FDF-00-HHZ.png (new)
			Trigger::Station st;
			st.networkCode = tokens[tokens.size() - 4];
			st.code = tokens[tokens.size() - 3];
			st.locationCode = tokens[tokens.size() - 2];
			st.channelCode = tokens[tokens.size() - 1].split('.')[0];
//			st.networkCode = tokens[tokens.size() - 2]; // (old)
//			st.code = tokens[tokens.size() - 1].split('.')[0]; // (old)
			st.snapshotFile = pics.at(j);

			stList << st;
		}

		QString webContent;
		webContent += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" "
			"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" + ENDL;
		webContent += "<html xmlns=\"http://www.w3.org/1999/xhtml\">" + ENDL;
		webContent += "<head>" + ENDL;
		webContent += TAB + "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" />" + ENDL;
		webContent += TAB + "<title>Job&nbsp;" + job->id() + "</title>" + ENDL;
		webContent += "</head>" + ENDL;
		webContent += "<body>" + ENDL;
		for (int k = 0; k < stList.count(); ++k) {
			webContent += TAB + "<div id=\"pic" + QString::number(k) + "\">" + ENDL;
			webContent += TAB + TAB + "<p>" + ENDL;
			webContent += TAB + TAB + TAB + "<a><img src=\"" + stList.at(k).snapshotFile +
			    "\" title=\"trigger\" alt=\"trigger\" width=\"500\" border=\"0\" /></a>" + ENDL;
			webContent += TAB + TAB + "</p><br />" + ENDL;
			webContent += TAB + "</div>" + ENDL;
		}
		webContent += "</body>";
		webContent += "</html>";

		const QString webFile = job->runDir() + QDir::separator() + pm->parameter("TRIGGER_PREFIX").toString()
		    + trigFileContent.at(i) + ".htm";

		if ( !Utils::writeScript(webFile, webContent) ) {
			log->addMessage(Logger::WARNING, __func__, "Failed to create HTML file " + webFile);
			continue;
		}
		else
			log->addMessage(Logger::DEBUG, __func__, "Generated HTML file " + webFile);

		const QString infoFile = job->runDir() + QDir::separator() + pm->parameter("TRIGGER_PREFIX").toString()
		    + trigFileContent.at(i) + ".txt";

		QString trigInfo;
		if ( Utils::fileExists(infoFile) )
			trigInfo = Utils::getFileContentString(infoFile, "");
		else
			log->addMessage(Logger::WARNING, __func__, "Failed to read trigger information file " + infoFile);

		Trigger* trig = Trigger::create(job->id());
		if ( trig ) {
			trig->setOriginTime(dt);
			trig->setStations(stList);
			trig->setResumePage(webFile);
			trig->setInformation(trigInfo);
			triggers << trig;
			list << trig->id();
		}
		else
			log->addMessage(Logger::WARNING, __func__, "Failed to create trigger object "
			    + dt.toString() + " for detection object " + job->id());
	}

	if ( triggers.isEmpty() ) return;

	for (QList<Trigger*>::iterator it = triggers.begin();
	        it != triggers.end(); ++it) {
		loadTrigger((*it));
	}

	emit newDetectionTriggers(job->id(), list);

	SDPASSERT(MainFrame::instancePtr());
	MainFrame::instancePtr()->newTriggerToInspect();
}


void TriggerPanel::removeTrigger(Trigger* t) {

	if ( !t ) return;

	SDPASSERT(Cache::instancePtr());
	SDPASSERT(DatabaseManager::instancePtr());

	for (int i = 0; i < __commitables.size(); ++i)
		if ( __commitables.at(i).object == t )
		    __commitables.removeAt(i);

	if ( !__triggers.contains(t) ) return;

	emit triggerStatusModified(static_cast<int>(tmDeleted), QStringList() << t->id());

	__triggers.removeOne(t);
	DatabaseManager::instancePtr()->removeTrigger(t);
	Cache::instancePtr()->removeObject(t->id(), false, false);
}


void TriggerPanel::rejectTriggers() {

	QList<QTableWidgetItem*> l = __table->selectedItems();

	//! Reject only selected triggers
	if ( l.size() == 0 ) return;

	QList<TableItem*> sel;
	for (int i = 0; i < l.size(); ++i) {
		TableItem* itm = dynamic_cast<TableItem*>(l.at(i));
		if ( !itm ) continue;
		sel << itm;
	}

	QList<TableItem*> parents;
	bool yesToAll = false;
	bool noToAll = false;
	QStringList files;
	QStringList rejected;

	for (int i = 0; i < sel.size(); ++i) {

		Trigger* trig = reinterpret_cast<Trigger*>(sel.at(i)->object);
		if ( !trig ) continue;

		if ( !yesToAll && !noToAll ) {
			QMessageBox::StandardButton b =
			    QMessageBox::question(this, tr("Trigger rejection"),
			        QString("Are you sure about rejecting trigger %1 ?").arg(trig->id()),
			        QMessageBox::Yes | QMessageBox::YesToAll |
			            QMessageBox::NoToAll | QMessageBox::No);
			if ( b == QMessageBox::Yes ) {
				trig->setStatus(Rejected);
				rejected << trig->id();
				for (int j = 0; j < __commitables.size(); ++j)
					if ( __commitables.at(j).objectFile.contains(trig->id()) )
					    files << __commitables.at(j).objectFile;
			}
			else if ( b == QMessageBox::YesToAll ) {
				trig->setStatus(Rejected);
				rejected << trig->id();
				for (int j = 0; j < __commitables.size(); ++j)
					if ( __commitables.at(j).objectFile.contains(trig->id()) )
					    files << __commitables.at(j).objectFile;
				yesToAll = true;
			}
			else if ( b == QMessageBox::No )
				continue;
			else if ( b == QMessageBox::NoToAll ) {
				noToAll = true;
				continue;
			}
		}
		else {
			if ( yesToAll && !noToAll ) {
				trig->setStatus(Rejected);
				rejected << trig->id();
				for (int j = 0; j < __commitables.size(); ++j)
					if ( __commitables.at(j).objectFile.contains(trig->id()) )
					    files << __commitables.at(j).objectFile;
			}
			if ( !yesToAll && noToAll )
			    continue;
		}
	}

	for (int i = 0; i < __table->rowCount(); ++i) {
		TableItem* itm = dynamic_cast<TableItem*>(__table->item(i, 0));
		QTableWidgetItem* status = __table->item(i, 2);
		if ( !itm || !status ) continue;
		Trigger* trig = reinterpret_cast<Trigger*>(itm->object);
		if ( !trig ) continue;
		status->setText((*trig->status()).string);
	}

	for (int i = 0; i < files.size(); ++i) {
		QFile::remove(files.at(i));
		int idx = 0;
		for (int j = 0; j < __commitables.size(); ++j)
			if ( __commitables.at(j).objectFile == files.at(i) )
			    idx = j;
		__commitables.removeAt(idx);
	}

	updateButtons();

	if ( !rejected.isEmpty() )
	    emit triggerStatusModified(static_cast<int>(tmRejected), rejected);
}


void TriggerPanel::acceptTriggers() {

	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(Logger::instancePtr());

	ParameterManager* pm = ParameterManager::instancePtr();
	Logger* log = Logger::instancePtr();

	QList<QTableWidgetItem*> l = __table->selectedItems();

	//! Accept only selected triggers
	if ( l.size() == 0 ) return;

	QList<TableItem*> sel;
	for (int i = 0; i < l.size(); ++i) {
		TableItem* itm = dynamic_cast<TableItem*>(l.at(i));
		if ( !itm ) continue;
		sel << itm;
	}

	QStringList acceptedList;
	QList<Trigger*> accepted;
	QList<TableItem*> parents;
	for (int i = 0; i < sel.size(); ++i) {
		Trigger* trig = reinterpret_cast<Trigger*>(sel.at(i)->object);
		if ( !trig ) continue;
		if ( QMessageBox::question(this, tr("Confirmation"),
		    QString("Are you sure about accepting trigger %1 ?").arg(trig->id()),
		    QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Cancel )
		    continue;

		trig->setStatus(Accepted);
		accepted << trig;
		acceptedList << trig->id();
	}

	for (int i = 0; i < __table->rowCount(); ++i) {
		TableItem* itm = dynamic_cast<TableItem*>(__table->item(i, 0));
		QTableWidgetItem* status = __table->item(i, 2);
		if ( !itm || !status ) continue;
		Trigger* trig = reinterpret_cast<Trigger*>(itm->object);
		if ( !trig ) continue;

		status->setText((*trig->status()).string);
	}

	if ( !Utils::dirExists(pm->parameter("XML_ARCHIVE_DIR").toString()) )
	    if ( !Utils::mkdir(pm->parameter("XML_ARCHIVE_DIR").toString()) )
	        log->addMessage(Logger::WARNING, __func__, "Failed to create directory "
	            + pm->parameter("XML_ARCHIVE_DIR").toString());

	QStringList files;
	for (int i = 0; i < accepted.count(); ++i) {
		const QString file = pm->parameter("XML_ARCHIVE_DIR").toString()
		    + QDir::separator() + accepted.at(i)->id() + ".xml";

		Commitable c;
		c.object = accepted.at(i);
		c.objectFile = file;

		if ( Utils::writeScript(file, generateSC3ML(accepted.at(i))) )
			__commitables << c;
		else
			log->addMessage(Logger::WARNING, __func__, "Failed to create XML archive " + file);
	}

	updateButtons();

	if ( !acceptedList.isEmpty() )
	    emit triggerStatusModified(static_cast<int>(tmAccepted), acceptedList);
}


void TriggerPanel::removeTriggers() {

	QList<QTableWidgetItem*> l = __table->selectedItems();

	//! Remove only selected triggers
	if ( l.size() == 0 ) return;

	QList<TableItem*> sel;
	for (int i = 0; i < l.size(); ++i) {
		TableItem* itm = dynamic_cast<TableItem*>(l.at(i));
		if ( !itm ) continue;
		sel << itm;
	}

	QList<TableItem*> parents;
	for (int i = 0; i < sel.size(); ++i) {

		Trigger* trig = reinterpret_cast<Trigger*>(sel.at(i)->object);
		if ( !trig ) continue;

		removeTrigger(trig);
		parents << sel.at(i);
	}

	removeTableRow(__table, parents);
}


void TriggerPanel::commitTriggers() {

	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(Logger::instancePtr());

	ParameterManager* pm = ParameterManager::instancePtr();
	Logger* log = Logger::instancePtr();

	//! Commit only reviewed triggers a.k.a. accepted triggers
	if ( __commitables.size() == 0 ) {
		QMessageBox::critical(this, tr("Error"), tr("There is no triggers in the commit list.\n"
			"Remember to properly validate each trigger first... ;)"));
		return;
	}

	if ( QMessageBox::question(this, tr("Confirmation"),
	    QString("Are you sure about committing %1 trigger(s) into database ?")
	        .arg(QString::number(__commitables.size())),
	    QMessageBox::Yes | QMessageBox::Cancel)
	    == QMessageBox::Cancel )
	    return;

	QString script;
	script += "#!" + pm->parameter("BASH_BIN").toString() + ENDL;
	for (int i = 0; i < __commitables.size(); ++i)
		script += pm->parameter("SEISCOMP_BIN").toString() + " exec scdispatch -O add -i "
		    + __commitables.at(i).objectFile + " " + pm->parameter("SC_DISPATCH_EXTRA_ARG").toString() + ENDL;
	script += ENDL;

	const QString commitFile = pm->parameter("XML_ARCHIVE_DIR").toString()
	    + QDir::separator() + "commit.sh";
	if ( !Utils::writeScript(commitFile, script) ) {
		log->addMessage(Logger::WARNING, __func__, "Failed to generate commit script " + commitFile);
		return;
	}

	CommitDialog d(commitFile, this);
	d.exec();

	if ( d.commitReturnCode() == CommitDialog::cdSUCCESS ) {

		QStringList committed;
		for (int i = 0; i < __commitables.size(); ++i) {

			TableItem* itm = reinterpret_cast<TableItem*>(__commitables.at(i).object->tableWidget());
			if ( !itm ) continue;

			QTableWidgetItem* ti = __table->item(itm->row(), getHeaderPosition(tSTATUS));
			if ( !ti ) return;

			__commitables.at(i).object->setStatus(Committed);
			ti->setText((*__commitables.at(i).object->status()).string);
			committed << __commitables.at(i).object->id();
		}

		//! Clear the list of triggers to commit after the run
		__commitables.clear();
		if ( !committed.isEmpty() ) {
			emit triggerStatusModified(static_cast<int>(tmCommitted), committed);
			if ( __autocleanButton->isChecked() )
			    autocleanTriggers();
		}
	}

	else {

		for (int i = 0; i < __commitables.size(); ++i) {

			TableItem* itm = reinterpret_cast<TableItem*>(__commitables.at(i).object->tableWidget());
			if ( !itm ) continue;

			QTableWidgetItem* ti = __table->item(itm->row(), getHeaderPosition(tSTATUS));
			if ( !ti ) continue;

			ti->setText(QString("%1 (commit failed)")
			    .arg((*__commitables.at(i).object->status()).string));
		}
	}
}


void TriggerPanel::resetTriggers() {

	SDPASSERT(Logger::instancePtr());

	QStringList reseted;
	TriggerItems itms = tableSelection();
	for (TriggerItems::iterator it = itms.begin(); it != itms.end(); ++it) {
		(*it).trigger->setStatus(WaitingForRevision);
		(*it).item->setText((*(*it).trigger->status()).string);
		reseted << (*it).trigger->id();
		Logger::instancePtr()->addMessage(Logger::INFO, __func__, "Reseted trigger with id " + (*it).trigger->id());
	}

	if ( !reseted.isEmpty() )
	    emit triggerStatusModified(static_cast<int>(tmAwaiting), reseted);
}


void TriggerPanel::autocleanTriggers() {

	QList<TableItem*> items;
	for (int i = 0; i < __table->rowCount(); ++i) {
		TableItem* itm = dynamic_cast<TableItem*>(__table->item(i, getHeaderPosition(tCTIME)));
		QTableWidgetItem* status = __table->item(i, getHeaderPosition(tSTATUS));
		if ( !itm || !status ) continue;
		Trigger* trig = reinterpret_cast<Trigger*>(itm->object);
		if ( !trig ) continue;
		if ( trig->status() == Committed )
		    items << itm;
	}

	removeTableRow(__table, items);
}


void TriggerPanel::triggerSelectionChanged(const QItemSelection& selected,
                                           const QItemSelection&) {

	if ( selected.size() != 0 ) {
		__ui->webView->show();
		__acceptButton->setEnabled(true);
		__rejectButton->setEnabled(true);
		__removeButton->setEnabled(true);
	}
	else {
		__ui->textEdit->clear();
		__ui->labelTriggerID->setText("-");
		__ui->webView->hide();
		__acceptButton->setEnabled(false);
		__rejectButton->setEnabled(false);
		__removeButton->setEnabled(false);
	}

	updateButtons();
}


void TriggerPanel::showTriggerOutput(const int& row, const int& col) {

	Q_UNUSED(col);
	TableItem* itm = dynamic_cast<TableItem*>(__table->item(row, 0));
	if ( !itm ) return;

	Trigger* trig = reinterpret_cast<Trigger*>(itm->object);
	if ( !trig ) return;

	displayTriggerOutput(trig);
}


EnqueueDialog::EnqueueDialog(const QString& type, QWidget* parent) :
		QDialog(parent), __ui(new Ui::Enqueue), __accepted(false) {

	__ui->setupUi(this);
	__ui->labelOperationType->setText(type);

	connect(__ui->buttonBoxValidation, SIGNAL(accepted()), this, SLOT(validated()));
	connect(__ui->buttonBoxValidation, SIGNAL(rejected()), this, SLOT(cancelled()));
}


EnqueueDialog::~EnqueueDialog() {}


void EnqueueDialog::validated() {
	__information = __ui->lineEditInformation->text();
	__comment = __ui->lineEditComment->text();
	__accepted = true;
}


void EnqueueDialog::cancelled() {
	__accepted = false;
}


DetectionPanel::DetectionPanel(QWidget* parent) :
		PanelWidget(parent), __ui(new Ui::DetectionPanel) {

	__ui->setupUi(this);

	setHeader(tr("Event detection"));
	setDescription(tr("Fetch and analyze data."));

	__dataSrc = new DataSourceSubPanel(this);
	__inventory = new InventorySubPanel(this);
	__tw = new TimeWindowSubPanel(this);
	__trig = new TriggerSubPanel(this);
	__stream = new StreamSubPanel(this);
	__scriptEditor = new ScriptEditor(10, this);

	__widgets << __dataSrc << __inventory << __tw << __trig << __stream;

	QFrame** line = new QFrame*[__widgets.count() - 1];
	for (int i = 0; i < __widgets.count() - 1; ++i) {
		line[i] = new QFrame(this);
		line[i]->setFrameShape(QFrame::HLine);
		line[i]->setFrameShadow(QFrame::Sunken);
	}

	int i = 0;
	QVBoxLayout* l0 = new QVBoxLayout(__ui->scrollAreaWidgetContents);
	l0->setContentsMargins(0, 0, -1, -1);
	l0->addWidget(__dataSrc);
	l0->addWidget(line[i++]);
	l0->addWidget(__stream);
	l0->addWidget(line[i++]);
	l0->addWidget(__tw);
	l0->addWidget(line[i++]);
	l0->addWidget(__trig);
	l0->addWidget(line[i++]);
	l0->addWidget(__inventory);

	QVBoxLayout* l1 = new QVBoxLayout(__ui->widgetScript);
	l1->setMargin(0);
	l1->addWidget(__scriptEditor);

	PythonSyntaxHighlighter* pha = new PythonSyntaxHighlighter;
	pha->setDocument(__scriptEditor->document());
	pha->setParent(__scriptEditor);

	__ui->pushButtonEnqueue->setIcon(QIcon(":images/schedule.png"));

	connect(__ui->pushButtonApply, SIGNAL(clicked()), this, SLOT(generateScript()));
	connect(__ui->pushButtonEnqueue, SIGNAL(clicked()), this, SLOT(enqueueJob()));
}


DetectionPanel::~DetectionPanel() {}


bool DetectionPanel::saveParameters() {

	for (SubPanelList::iterator it = __widgets.begin();
	        it != __widgets.end(); ++it)
		(*it)->saveParameters();

	return true;
}


void DetectionPanel::load(DetectionJob* job) {

	if ( !job ) return;

	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(Logger::instancePtr());

	ParameterManager* pm = ParameterManager::instancePtr();

	__scriptEditor->clear();
	__scriptEditor->insertPlainText(job->scriptData().toString());

	//! Setup the station's inventory
	__inventory->loadParameters();
	for (int i = 0; i < job->stations().size(); ++i) {
		__inventory->addNetwork(job->stations().at(i).networkCode);
		__inventory->addStation(job->stations().at(i).networkCode,
		    job->stations().at(i).code, job->stations().at(i).locationCode,
		    job->stations().at(i).channelCode);
	}

	//! Load each section parameters
	pm->setObjectParameters(job->parameters(DetectionJob::peDATASOURCE), "DataSourceSubPanel");
	pm->setObjectParameters(job->parameters(DetectionJob::peSTREAM), "StreamSubPanel");
	pm->setObjectParameters(job->parameters(DetectionJob::peTIMEWINDOW), "TimeWindowSubPanel");
	pm->setObjectParameters(job->parameters(DetectionJob::peTRIGGER), "TriggerSubPanel");

	__dataSrc->loadParameters();
	__stream->loadParameters();
	__trig->loadParameters();
	__tw->loadParameters();

	Logger::instancePtr()->addMessage(Logger::INFO, __func__, "Loaded job " + job->id() + " parameters");
}


void DetectionPanel::generateScript() {

	//! Check parameters from sub-panels...
	//! Anything raising error should be taken care of by user so that the
	//! script could be free of errors.

	//! [0] The data source
	if ( !__dataSrc->sourceIsOkay() ) return;

	//! [1] The stream
	if ( !__stream->streamIsOkay() ) return;

	//! [2] The time window
	if ( !__tw->timeWindowIsOkay() ) return;
	if ( !__tw->sampleLengthIsOkay() ) return;

	//! [3] The inventory
	if ( !__inventory->inventoryIsOkay() ) return;

	//! Save parameters
	for (SubPanelList::iterator it = __widgets.begin();
	        it != __widgets.end(); ++it)
		(*it)->saveParameters();

	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(InventorySubPanel::instancePtr());

	ParameterManager* w = ParameterManager::instancePtr();

	QString trigMethod;
	QString carlSTATrigConfig;
	if ( w->parameter("CarlSTATrig").toBool() ) {
		trigMethod = "carlstatrig";
		carlSTATrigConfig += ", ratio= " + w->parameter("CarlSTATrigRatio").toString();
		carlSTATrigConfig += ", quiet=" + w->parameter("CarlSTATrigQuiet").toString();
	}
	if ( w->parameter("ClassicSTALTA").toBool() )
	    trigMethod = "classicstalta";
	if ( w->parameter("DelayedSTALTA").toBool() )
	    trigMethod = "delayedstalta";
	if ( w->parameter("RecursiveSTALTA").toBool() )
	    trigMethod = "recstalta";

	QString chans;
	if ( w->parameter("streamChannelAll").toBool() )
	    chans = "*";
	if ( w->parameter("streamChannelZOnly").toBool() )
	    chans = "*Z";
	if ( w->parameter("streamChannelNSOnly").toBool() )
	    chans = "*N";
	if ( w->parameter("streamChannelEWOnly").toBool() )
	    chans = "*E";
	if ( w->parameter("streamChannelCustom").toBool() )
	    chans = w->parameter("streamChannelCustomValue").toString();

	//! Add the trigger info dynamically
	QString trigInfoFile = w->parameter("TRIGGER_INFO_FILE").toString();
	trigInfoFile.replace("@TRIGGER_DATETIME@", "\" + str(trig[it]['time']) + \"");

	//! At the time of this version of ObsPy, it is said that the coincidence
	//! trigger is to be used for stations from a sole network (the API is
	//! actually displaying information station by station and doesn't
	//! seem to care about the stations networks). This may be why the
	//! authors discarded the use of multiples networks in this process...
	//! Although it is discarded, it actually works pretty well so we make
	//! sure we handle it for future use.
	if ( __inventory->networkCount() != 0 ) {
		QMessageBox::information(this, tr("Information"),
		    tr("You are trying to operate a network coincidence trigger "
			    "on more than one network. ObsPy implementation is not built "
			    "for such request. The results are therefore uncertain."));
	}

	QString script;
	script += "#!" + w->parameter("ENV_BIN").toString() + " python" + ENDL;
	script += ENDL;
	script += "\"\"\"" + ENDL;
	script += "############################################################" + ENDL;
	script += TAB + PROJECT_NAME + "-" + PROJECT_VERSION + ENDL;
	script += TAB + "File type: detection script." + ENDL;
	script += TAB + "Datetime: " + QDateTime::currentDateTime().toString() + ENDL;
	script += "############################################################" + ENDL;
	script += "\"\"\"" + ENDL;
	script += "import errno, sys" + ENDL;
	script += "import locale" + ENDL;
	script += "from obspy.core import UTCDateTime, Stream, read" + ENDL;
	script += "from obspy.arclink import Client" + ENDL;
	script += "from obspy.signal import coincidenceTrigger" + ENDL;
	script += "import matplotlib.pyplot as plt" + ENDL;
	script += "import datetime" + ENDL;
	script += ENDL;
	script += "\"\"\" Setup script locale \"\"\"" + ENDL;
	script += "locale.setlocale(locale.LC_ALL, 'en_US.UTF-8')" + ENDL;
	script += ENDL;
	script += "\"\"\" Some useful things \"\"\"" + ENDL;
	script += "def debug(msg):" + ENDL;
	script += TAB + "now = datetime.datetime.now()" + ENDL;
	script += TAB + "sys.stdout.write(now.strftime(\"[%H:%M:%S] \") + msg + \"\\n\")" + ENDL;
	script += ENDL;
	script += "def error(msg):" + ENDL;
	script += TAB + "now = datetime.datetime.now()" + ENDL;
	script += TAB + "sys.stderr.write(now.strftime(\"[%H:%M:%S] \") + msg + \"\\n\")" + ENDL;
	script += ENDL;
	script += ENDL;
	script += "\"\"\" Environment variables \"\"\"" + ENDL;
	script += "tmpDataDir = \"@JOB_RUN_DIR@\"" + ENDL;
	script += "logFile = tmpDataDir + \"detect.log\"" + ENDL;
	script += "orgExportFile = tmpDataDir + \"triggers.txt\"" + ENDL;

	if ( w->parameter("dataSourceFile").toBool() ) {

		script += ENDL;
		script += "\"\"\" The mSEED file to be analyzed \"\"\"" + ENDL;
		script += "filename = \"" + w->parameter("dataSourceFilepath").toString() + "\"" + ENDL;
		script += ENDL;
		script += "st = read(filename)" + ENDL;
		script += ENDL;
		script += "# List of stations and corresponding networks" + ENDL;
		script += __inventory->pythonStations("networkCodes", "stationCodes");
		script += ENDL;
		script += "# The length per sample to be analyzed" + ENDL;
		script += "period = " + w->parameter("streamSampleDuration").toString() + ENDL;
		script += ENDL;
		script += "# Determine the usable time-window of the file " + ENDL;
		script += "streamStart = 0" + ENDL;
		script += "streamEnd = 0" + ENDL;
		script += "stCount = 0" + ENDL;
		script += "for t in st:" + ENDL;
		script += TAB + "if (streamStart < t.stats.starttime) is True:" + ENDL;
		script += TAB + TAB + "streamStart = t.stats.starttime" + ENDL;
		script += TAB + "if (streamEnd == 0) is True:" + ENDL;
		script += TAB + TAB + "streamEnd = t.stats.endtime" + ENDL;
		script += TAB + "if (streamEnd > t.stats.endtime) is True:" + ENDL;
		script += TAB + TAB + "streamEnd = t.stats.endtime" + ENDL;
		script += TAB + "stCount += 1" + ENDL;
		script += "debug(\"===================================================================\")" + ENDL;
		script += "debug(\"The file contains \" + str(stCount) + \" stream(s)\")" + ENDL;
		script += "debug(\"===================================================================\")" + ENDL;
	}


	else {

		script += "filename = tmpDataDir + \"data.mseed\"" + ENDL;
		script += "serverAddress = \"" + w->parameter("dataSourceArcserverAddress").toString() + "\"" + ENDL;
		script += "serverPort = " + w->parameter("dataSourceArcserverPort").toString() + ENDL;
		script += ENDL;
		script += "# The stream's start and end times" + ENDL;
		script += "streamStart = UTCDateTime(\"" + w->parameter("streamStartTime").toString() + "\")" + ENDL;
		script += "streamEnd = UTCDateTime(\"" + w->parameter("streamEndTime").toString() + "\")" + ENDL;
		script += ENDL;
		script += "# The length per sample to be analyzed" + ENDL;
		script += "period = " + w->parameter("streamSampleDuration").toString() + ENDL;
		script += ENDL;
		script += "# Stream validity check" + ENDL;
		script += "hasStream = False" + ENDL;
		script += ENDL;
		script += ENDL;


		script += "\"\"\"" + ENDL;
		script += TAB + "FIRST STEP" + ENDL;
		script += TAB + "- Fetch and store data from requested period in temporary file so it" + ENDL;
		script += TAB + "  can be reviewed later on by the user." + ENDL;
		script += "\"\"\"" + ENDL;
		script += ENDL;
		script += ENDL;
		script += "# Feed the stream object with waveforms..." + ENDL;
		script += "# We're gonna request waveforms for each station of each network, and," + ENDL;
		script += "# amalgamate those data all together into the following stream object." + ENDL;
		script += "st = Stream()" + ENDL;
		script += ENDL;
		script += "# List of stations and corresponding networks " + ENDL;
		script += __inventory->pythonStations("networkCodes", "stationCodes");
		script += ENDL;
		script += ENDL;
		script += "# Establish connection to Arclink server" + ENDL;
		script += "client = Client(host=serverAddress, port=serverPort, user=\"script@sdp\")" + ENDL;
		script += ENDL;
		script += "debug(\"===================================================================\")" + ENDL;
		script += "debug(\"Preparing to fetch data from \" + str(streamStart) + \" to \" + str(streamEnd))" + ENDL;
		script += ENDL;
		script += "if not client:" + ENDL;
		script += TAB + "error(\"Connection to Arclink server'\" + serverAddress +" + ENDL;
		script += "                     \":\" + str(serverPort) + \"' could not be established\")" + ENDL;
		script += TAB + "sys.exit(errno.EACCES)" + ENDL;
		script += ENDL;
		script += "debug(\"Sent request to Arclink server, now awaiting data...\")" + ENDL;
		script += "debug(\"-------------------------------------------------------------------\")" + ENDL;
		script += ENDL;
		script += "for netCode, staCode in zip(networkCodes, stationCodes):" + ENDL;

		QStringList chanList = chans.split(',');
		for (int j = 0; j < chanList.size(); ++j) {
			script += TAB + "try:" + ENDL;
			script += TAB + TAB + "debug(\"Requesting data for \" + netCode + \"/\" + staCode)" + ENDL;
			script += TAB + TAB + "st += client.getWaveform(netCode, staCode, \"*\", \"" + chanList.at(j) + "\", streamStart, streamEnd, route=False)" + ENDL;
			script += TAB + "except Exception as e:" + ENDL;
			script += TAB + TAB + "error(\"Failed: \" + netCode + \"/\" + staCode + \" - \" + str(e))" + ENDL;
			script += ENDL;
		}
		script += ENDL;
		script += ENDL;


		script += "\"\"\"" + ENDL;
		script += TAB + "SECOND STEP" + ENDL;
		script += TAB + "- We now check briefly and before writing down the content of the acquired" + ENDL;
		script += TAB + "  data. See if we've got something to sink our teeth in!" + ENDL;
		script += "\"\"\"" + ENDL;
		script += ENDL;
		script += "if st.count() > 0:" + ENDL;
		script += TAB + "debug(\"-------------------------------------------------------------------\")" + ENDL;
		script += TAB + "debug(\"Writing down temp data to \" + filename)" + ENDL;
		script += TAB + "st.write(filename, format='MSEED')" + ENDL;
		script += TAB + "hasStream = True" + ENDL;
		script += ENDL;
		script += ENDL;
		script += "if not hasStream:" + ENDL;
		script += TAB + "error(\"===================================================================\")" + ENDL;
		script += TAB + "error(\"No mseed data acquired, exiting script...\")" + ENDL;
		script += TAB + "error(\"===================================================================\")" + ENDL;
		script += TAB + "sys.exit(1)" + ENDL;
		script += ENDL;
		script += ENDL;


		script += "\"\"\"" + ENDL;
		script += TAB + "THIRD STEP" + ENDL;
		script += TAB + "- Analyze stored data with the coincidence trigger and output anything" + ENDL;
		script += TAB + "  that come close to an origin into a origin file. This file will be" + ENDL;
		script += TAB + "  validated by the user afterwards." + ENDL;
		script += "\"\"\"" + ENDL;

	}

	script += ENDL;
	script += "tStart = streamStart" + ENDL;
	script += "trigTotal = 0" + ENDL;
	script += ENDL;
	script += ENDL;
	script += "debug(\"-------------------------------------------------------------------\")" + ENDL;
	script += "debug(\"Iterations will run from \" + str(streamStart) + \" to \" + str(streamEnd))" + ENDL;
	script += "debug(\"-------------------------------------------------------------------\")" + ENDL;
	script += ENDL;
	script += "loopCount = 1" + ENDL;
	script += "while tStart < streamEnd:" + ENDL;
	script += ENDL;
	script += TAB + "# Overlap current sample with previous sample if there is one" + ENDL;
	script += TAB + "tEnd = tStart + period" + ENDL;
	script += TAB + "nstart = UTCDateTime()" + ENDL;
	script += TAB + "nend = UTCDateTime()" + ENDL;
	script += ENDL;
	script += TAB + "if tStart == streamStart:" + ENDL;
	script += TAB + TAB + "nstart = tStart" + ENDL;
	script += TAB + TAB + "nend = tEnd" + ENDL;
	script += TAB + "else:" + ENDL;
	script += TAB + TAB + "nstart = tStart - period / 2" + ENDL;
	script += TAB + TAB + "nend = tEnd" + ENDL;
	script += ENDL;
	script += TAB + "debug(\"-------------------------------------------------------------------\")" + ENDL;
	script += TAB + "debug(\"[\" + str(loopCount) + \"] Analysis from \" + str(nstart) + \" to \" + str(nend))" + ENDL;
	script += ENDL;
	script += TAB + "trace = Stream()" + ENDL;
	script += TAB + "try:" + ENDL;
	script += TAB + TAB + "trace = read(filename, starttime=nstart, endtime=nend)" + ENDL;
	script += TAB + "except Exception as e:" + ENDL;
	script += TAB + TAB + "error(\"    Failed: \" + str(e))" + ENDL;
	script += ENDL;
	script += TAB + "if not trace:" + ENDL;
	script += TAB + TAB + "error(\"Datafile \" + filename + \" is not readable\")" + ENDL;
	script += TAB + TAB + "sys.exit(errno.EACCES)" + ENDL;
	script += ENDL;
	script += TAB + "fsel = trace.select(channel=\"" + chans + "\")" + ENDL;
	script += TAB + "debug(\" \" + str(fsel.count()) + \" stream(s) to be checked out\")" + ENDL;
	script += ENDL;
	script += TAB + "# Check for gaps in stations streams" + ENDL;
	script += TAB + "selection = Stream()" + ENDL;
	script += TAB + "for netCode, staCode in zip(networkCodes, stationCodes):" + ENDL;
	script += TAB + TAB + "psel = trace.select(network=netCode, station=staCode, channel=\"" + chans + "\")" + ENDL;
	script += TAB + TAB + "if len(psel.getGaps()) == 0:" + ENDL;
	script += TAB + TAB + TAB + "selection += psel" + ENDL;
	script += TAB + TAB + "else:" + ENDL;
	script += TAB + TAB + TAB + "debug(\" Ignored gapped stream for station \" + netCode + \"/\" + staCode)" + ENDL;
	script += ENDL;
	script += TAB + "debug(\" \" + str(selection.count()) + \" stream(s) will be analyzed\")" + ENDL;
	script += ENDL;
	script += TAB + "# Perform coincidence trigger on remaining streams" + ENDL;
	script += TAB + "trig = list()" + ENDL;
	script += TAB + "hasComputed = True" + ENDL;
	if ( w->parameter("Config-Filter-Enabled").toBool() ) {
		QString properties;
		if ( w->parameter("Config-Filter-Name").toString().contains("bandpass") )
			properties = QString("freqmin=%1, freqmax=%2")
			    .arg(QString::number(w->parameter("Config-Filter-FreqMin").toDouble()))
			    .arg(QString::number(w->parameter("Config-Filter-FreqMax").toDouble()));
		else
			properties = QString("freq=%1").arg(QString::number(w->parameter("Config-Filter-FreqMin").toDouble()));
		script += TAB + "# Applying filter" + ENDL;
		script += TAB + QString("selection.filter(\"%1\", %2)")
		    .arg(w->parameter("Config-Filter-Name").toString()).arg(properties) + ENDL;
	}

	script += TAB + "try:" + ENDL;
	script += TAB + TAB + "trig = coincidenceTrigger(\"" + trigMethod + "\", "
	    + w->parameter("TreshON").toString() + ", " + w->parameter("TreshOFF").toString()
	    + ", selection, " + w->parameter("SUM").toString() + ", sta=" + w->parameter("STA").toString()
	    + ", lta=" + w->parameter("LTA").toString() + ", details=True" + carlSTATrigConfig + ")" + ENDL;
	script += TAB + "except Exception as e:" + ENDL;
	script += TAB + TAB + "hasComputed = False" + ENDL;
	script += TAB + TAB + "error(\" Failed: \" + str(e))" + ENDL;
	script += ENDL;
	script += TAB + "trigCount = len(trig)" + ENDL;
	script += ENDL;
	script += TAB + "if trigCount > 0:" + ENDL;
	script += ENDL;
	script += TAB + TAB + "debug(\" Coincidence trigger reported \" + str(trigCount) + \" detection(s)\")" + ENDL;
	script += ENDL;
	script += TAB + TAB + "for it in range(0, trigCount):" + ENDL;
	script += ENDL;
	script += TAB + TAB + TAB + "trigTotal += 1" + ENDL;
	script += TAB + TAB + TAB + "debug(\" Possible event at \" + str(trig[it]['time']))" + ENDL;
	script += TAB + TAB + TAB + "debug(\" stations = \" + str(trig[it]['stations']))" + ENDL;
	script += ENDL;
	script += TAB + TAB + TAB + "# Add new origin line into run trigger file" + ENDL;
	script += TAB + TAB + TAB + "with open(orgExportFile, \"a\") as ofile:" + ENDL;
	script += TAB + TAB + TAB + TAB + "ofile.write(\"%s\\n\" % str(trig[it]['time']))" + ENDL;
	script += TAB + TAB + TAB + TAB + "debug(\" Added record of trigger at \" + str(trig[it]['time']) + \" into trigger file.\")" + ENDL;
	script += ENDL;
	script += TAB + TAB + TAB + "# Add trigger information in custom trigger file" + ENDL;
	script += TAB + TAB + TAB + "with open(\"" + trigInfoFile + "\", \"w\") as tfile:" + ENDL;
	script += TAB + TAB + TAB + TAB + "tfile.write(\"%s\\n\" % str(trig[it]))" + ENDL;
	script += TAB + TAB + TAB + TAB + "debug(\" Added record of trigger information to file\")" + ENDL;
	script += ENDL;
	script += TAB + TAB + TAB + "# TODO: Find who to compare with the network as argument too" + ENDL;
	script += TAB + TAB + TAB + "# If two networks have a same station's name, this can be a problem" + ENDL;
	script += TAB + TAB + TAB + "triggeredStations = Stream()" + ENDL;
	script += TAB + TAB + TAB + "for i in range(0, len(selection)):" + ENDL;
	script += TAB + TAB + TAB + TAB + "for j in range(0, len(trig[it]['stations'])):" + ENDL;
	script += TAB + TAB + TAB + TAB + TAB + "if selection[i].stats.station == trig[it]['stations'][j]:" + ENDL;
	script += TAB + TAB + TAB + TAB + TAB + TAB + "triggeredStations += selection[i]" + ENDL;
	script += TAB + TAB + TAB + TAB + TAB + "j += 1" + ENDL;
	script += TAB + TAB + TAB + TAB + "i += 1" + ENDL;
	script += ENDL;
	script += TAB + TAB + TAB + "for i in range(0, len(triggeredStations)):" + ENDL;
	script += ENDL;
	script += TAB + TAB + TAB + TAB + "stationStream = triggeredStations[i]" + ENDL;
	script += ENDL;
	script += TAB + TAB + TAB + TAB + "if not stationStream:" + ENDL;
	script += TAB + TAB + TAB + TAB + TAB + "continue" + ENDL;
	script += ENDL;

	if ( w->parameter("Config-Filter-Enabled").toBool() ) {
		QString properties;
		if ( w->parameter("Config-Filter-Name").toString().contains("bandpass") )
			properties = QString("freqmin=%1, freqmax=%2")
			    .arg(QString::number(w->parameter("Config-Filter-FreqMin").toDouble()))
			    .arg(QString::number(w->parameter("Config-Filter-FreqMax").toDouble()));
		else
			properties = QString("freq=%1").arg(QString::number(w->parameter("Config-Filter-FreqMin").toDouble()));
		script += TAB + TAB + TAB + TAB + "# Applying filter" + ENDL;
		script += TAB + TAB + TAB + TAB + QString("stationStream.filter(\"%1\", %2)")
		    .arg(w->parameter("Config-Filter-Name").toString()).arg(properties) + ENDL;
		script += ENDL;
	}

	script += TAB + TAB + TAB + TAB + "# Stream sampling rate" + ENDL;
	script += TAB + TAB + TAB + TAB + "df = stationStream.stats.sampling_rate" + ENDL;
	script += ENDL;
	script += TAB + TAB + TAB + TAB + "# Add raw spectrograph" + ENDL;
	script += TAB + TAB + TAB + TAB + "first = plt.subplot(211)" + ENDL;
	script += TAB + TAB + TAB + TAB + "# filteredTr = stationStream.filter(\"highpass\", freq=1.0)" + ENDL;
	script += TAB + TAB + TAB + TAB + "first.set_title(stationStream.stats.network + \"-\" + stationStream.stats.station + \"-\" + stationStream.stats.location + \"-\" + stationStream.stats.channel)" + ENDL;
	script += TAB + TAB + TAB + TAB + "plt.plot(stationStream.data, 'k')" + ENDL;
	script += TAB + TAB + TAB + TAB + "ymin, ymax = first.get_ylim()" + ENDL;
	script += TAB + TAB + TAB + TAB + "plt.vlines((trig[it]['time'] - stationStream.stats.starttime) * df, ymin, ymax, color='r', linewidth=2)" + ENDL;
	script += TAB + TAB + TAB + TAB + "plt.vlines((trig[it]['time'] + trig[it]['duration'] - stationStream.stats.starttime) * df, ymin, ymax, color='b', linewidth=2)" + ENDL;
	script += TAB + TAB + TAB + TAB + "plt.axis('tight')" + ENDL;
	script += ENDL;
	script += TAB + TAB + TAB + TAB + "# Add raw spectrogram" + ENDL;
	script += TAB + TAB + TAB + TAB + "second = plt.subplot(212)" + ENDL;
	script += TAB + TAB + TAB + TAB + "plt.specgram(stationStream.data)" + ENDL;
	script += TAB + TAB + TAB + TAB + "plt.axis('tight')" + ENDL;
	script += ENDL;
	script += TAB + TAB + TAB + TAB + "# plt.show()" + ENDL;
	script += TAB + TAB + TAB + TAB + "# Save the new trigger figure" + ENDL;
	script += TAB + TAB + TAB + TAB + "pltName = tmpDataDir + str(trig[it]['time']) + \"-\" + stationStream.stats.network + \"-\" + stationStream.stats.station + \"-\" + stationStream.stats.location + \"-\" + stationStream.stats.channel + \".png\"" + ENDL;
	script += TAB + TAB + TAB + TAB + "plt.savefig(pltName, bbox_inches='tight', pad_inches=0)" + ENDL;
	script += TAB + TAB + TAB + TAB + "debug(\" Created trigger snapshot \" + pltName)" + ENDL;
	script += ENDL;
	script += TAB + TAB + TAB + TAB + "# Clear the current figure so that it will not be pasted" + ENDL;
	script += TAB + TAB + TAB + TAB + "# to the next one" + ENDL;
	script += TAB + TAB + TAB + TAB + "plt.clf()" + ENDL;
	script += TAB + TAB + TAB + TAB + "i += 1" + ENDL;
	script += TAB + TAB + TAB + "it += 1" + ENDL;
	script += TAB + "else:" + ENDL;
	script += TAB + TAB + "if hasComputed:" + ENDL;
	script += TAB + TAB + TAB + "debug(\"0 similarities reported for selected time window.\")" + ENDL;
	script += ENDL;
	script += TAB + "tStart += period" + ENDL;
	script += TAB + "loopCount += 1" + ENDL;
	script += ENDL;
	script += "debug(\"===================================================================\")" + ENDL;
	script += "debug(\"This run has generated a total of \" + str(trigTotal) + \" trigger(s)\")" + ENDL;
	script += ENDL;
	script += "if trigTotal > 0:" + ENDL;
	script += TAB + "debug(\"Check out \" + orgExportFile + \" and identify possible origin(s)\")" + ENDL;
	script += ENDL;
	script += "debug(\"===================================================================\")" + ENDL;
	script += ENDL;
	script += "sys.exit(0)" + ENDL;

	__scriptEditor->clear();
	__scriptEditor->insertPlainText(script);
}

void DetectionPanel::enqueueJob() {

	if ( __scriptEditor->toPlainText().isEmpty() ) return;

	EnqueueDialog d("Detection information:", this);
	d.exec();

	if ( !d.accepted() ) return;

	SDPASSERT(ActivityPanel::instancePtr());
	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(InventorySubPanel::instancePtr());
	SDPASSERT(Logger::instancePtr());

	ActivityPanel* monitor = ActivityPanel::instancePtr();
	ParameterManager* w = ParameterManager::instancePtr();
	InventorySubPanel* s = InventorySubPanel::instancePtr();

	QStringList networks;
	for (int i = 0; i < s->entities().size(); ++i) {
		if ( s->entities().at(i)->parent ) continue;
		networks << s->entities().at(i)->name;
	}

	DetectionJob::DetectionStationList stationList;
	QStringList stations;
	for (int i = 0; i < s->entities().size(); ++i) {
		if ( !s->entities().at(i)->parent ) continue;

		DetectionJob::DetectionStation sta;
		sta.networkCode = s->entities().at(i)->parent->name;
		sta.code = s->entities().at(i)->name;
		sta.locationCode = s->entities().at(i)->locationCode;
		sta.channelCode = s->entities().at(i)->channelCode;
		stationList << sta;

		stations << s->entities().at(i)->parent->name + "." + s->entities().at(i)->name;
	}

	QString trigMethod;
	if ( w->parameter("CarlSTATrig").toBool() )
	    trigMethod = "carlstatrig";
	if ( w->parameter("ClassicSTALTA").toBool() )
	    trigMethod = "classicstalta";
	if ( w->parameter("DelayedSTALTA").toBool() )
	    trigMethod = "delayedstalta";
	if ( w->parameter("RecursiveSTALTA").toBool() )
	    trigMethod = "recstalta";

	QString tooltip;
	if ( w->parameter("dataSourceFile").toBool() ) {
		tooltip += "<b>Source</b>: " + w->parameter("dataSourceFilepath").toString() + "<br />";
	}
	else {
		tooltip += "<b>Source</b>: " + w->parameter("dataSourceArcserverAddress").toString()
		    + ":" + w->parameter("dataSourceArcserverPort").toString() + "<br />";
		tooltip += "<p><b>Stream start time</b>: " + w->parameter("streamStartTime").toString() + "<br />";
		tooltip += "<b>Stream end time</b>: " + w->parameter("streamEndTime").toString() + "<br />";
	}
	tooltip += "<b>Network(s)</b>: " + QString::number(networks.size()) + " [ ";
	for (int i = 0; i < networks.size(); ++i)
		(i == networks.size()) ? tooltip += networks.at(i) : tooltip += networks.at(i) + ", ";
	tooltip += " ] <br />";
	tooltip += "<b>Station(s)</b>: " + QString::number(stations.size()) + " [ ";
	for (int i = 0; i < stations.size(); ++i)
		(i == stations.size()) ? tooltip += stations.at(i) : tooltip += stations.at(i) + ", ";
	tooltip += " ] <br />";
	tooltip += "<b>Trigger method</b>: " + trigMethod + "<br />";
	tooltip += "</p>";

	//! Setup the job so it uses a custom dir. Default dir will be
	//! 'detect-currentDateTime' (the one generated from the ID of the job)
	//! but it gets messy when the user wants to review a detection from a
	//! certain time window and the sole solution would be to check out each
	//! folder and seek for the information in the script itself...
	QString customRunDir = QString("detection-%1-%2")
	    .arg(w->parameter("streamStartTime").toDateTime().toString("yyyyMMddhhmmss"))
	    .arg(w->parameter("streamEndTime").toDateTime().toString("yyyyMMddhhmmss"));

	if ( w->parameter("dataSourceFile").toBool() )
	    customRunDir = QString();

	DetectionJob* job = DetectionJob::create();
	if ( job ) {
		job->setInformation(d.information());
		job->setComment(d.comment());
		job->setTooltip(tooltip);
		job->setCustomRunDir(customRunDir);
		job->setScriptData(QVariant::fromValue(__scriptEditor->toPlainText()));
		job->setStations(stationList);
		job->setParameterList(DetectionJob::peDATASOURCE, w->getObjectParameters("DataSourceSubPanel"));
		job->setParameterList(DetectionJob::peSTREAM, w->getObjectParameters("StreamSubPanel"));
		job->setParameterList(DetectionJob::peTIMEWINDOW, w->getObjectParameters("TimeWindowSubPanel"));
		job->setParameterList(DetectionJob::peTRIGGER, w->getObjectParameters("TriggerSubPanel"));

		monitor->addJob(job);
	}
	else
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Failed to create detection object. Something went wrong...");
}


DispatchPanel::DispatchPanel(QWidget* parent) :
		PanelWidget(parent), __ui(new Ui::DispatchPanel) {

	__ui->setupUi(this);

	setHeader(tr("Data dispatch"));
	setDescription(tr("Create a SeisComP Data Structure archive."));

	__scriptEditor = new ScriptEditor(10, this);
	QVBoxLayout* l = new QVBoxLayout(__ui->widgetScript);
	l->setMargin(0);
	l->setSpacing(0);
	l->addWidget(__scriptEditor);

	BashHighlighter* h = new BashHighlighter;
	h->setDocument(__scriptEditor->document());
	h->setParent(__scriptEditor);

	__ui->pushButtonEnqueue->setIcon(QIcon(":images/schedule.png"));
	__ui->radioButtonMsrouter->setChecked(true);
	__ui->lineEditSDSPattern->setText("%Y/%n/%s/%c.D/%n.%s.%l.%c.D.%Y.%j");
	__ui->lineEditDataPattern->setText("MSEED/%Y.%d");
	__ui->radioButtonCustomParameters->setChecked(true);
	__ui->lineEditArguments->hide();
	__ui->widgetOptions->hide();

	connect(__ui->pushButtonApply, SIGNAL(clicked()), this, SLOT(generateScript()));
	connect(__ui->toolButtonDataDir, SIGNAL(clicked()), this, SLOT(selectDataDir()));
	connect(__ui->toolButtonSDSDir, SIGNAL(clicked()), this, SLOT(selectSDSDir()));
	connect(__ui->pushButtonEnqueue, SIGNAL(clicked()), this, SLOT(enqueueJob()));
	connect(__ui->radioButtonMsmod, SIGNAL(clicked()), this, SLOT(showHideMsModOptions()));
	connect(__ui->radioButtonMsrouter, SIGNAL(clicked()), this, SLOT(showHideMsModOptions()));
	connect(__ui->radioButtonCustomArguments, SIGNAL(clicked()), this, SLOT(showHideCustomOptions()));
	connect(__ui->radioButtonCustomParameters, SIGNAL(clicked()), this, SLOT(showHideCustomOptions()));


	SDPASSERT(ParameterManager::instancePtr());

	ParameterManager::instancePtr()->registerParameter("Dispatch-Msrouter");
	ParameterManager::instancePtr()->registerParameter("Dispatch-Msmod");
	ParameterManager::instancePtr()->registerParameter("Dispatch-DataDir");
	ParameterManager::instancePtr()->registerParameter("Dispatch-DataPattern");
	ParameterManager::instancePtr()->registerParameter("Dispatch-SDSDir");
	ParameterManager::instancePtr()->registerParameter("Dispatch-SDSPattern");

	loadConfiguration();
}


DispatchPanel::~DispatchPanel() {}


bool DispatchPanel::saveParameters() {

	SDPASSERT(ParameterManager::instancePtr());

	ParameterManager* p = ParameterManager::instancePtr();

	p->setParameter("Dispatch-Msrouter", QVariant::fromValue(__ui->radioButtonMsrouter->isChecked()));
	p->setParameter("Dispatch-Msmod", QVariant::fromValue(__ui->radioButtonMsmod->isChecked()));
	p->setParameter("Dispatch-DataDir", QVariant::fromValue(__ui->lineEditDataDir->text()));
	p->setParameter("Dispatch-DataPattern", QVariant::fromValue(__ui->lineEditDataPattern->text()));
	p->setParameter("Dispatch-SDSDir", QVariant::fromValue(__ui->lineEditSDSDir->text()));
	p->setParameter("Dispatch-SDSPattern", QVariant::fromValue(__ui->lineEditSDSPattern->text()));

	return true;
}


bool DispatchPanel::loadConfiguration() {

	SDPASSERT(MainFrame::instancePtr());

	Config* cfg = MainFrame::instancePtr()->config();

	try {
		__ui->lineEditSDSDir->setText(cfg->getString("dispatch.sds.folder"));
	} catch ( ... ) {}
	try {
		__ui->lineEditSDSPattern->setText(cfg->getString("dispatch.sds.pattern"));
	} catch ( ... ) {}

	try {
		__ui->lineEditDataDir->setText(cfg->getString("dispatch.mseed.folder"));
	} catch ( ... ) {}
	try {
		__ui->lineEditDataPattern->setText(cfg->getString("dispatch.mseed.pattern"));
	} catch ( ... ) {}
	try {
		const QString m = cfg->getString("dispatch.method");
		(m.contains("msrouter")) ?
		    __ui->radioButtonMsrouter->setChecked(true)
		        : __ui->radioButtonMsmod->setChecked(true);
	} catch ( ... ) {}
	try {
		__ui->lineEditArguments->setText(cfg->getString("dispatch.msmod.customArguments"));
	} catch ( ... ) {}

	return true;
}


void DispatchPanel::load(DispatchJob*) {

}


void DispatchPanel::generateScript() {

	saveParameters();

	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(Environment::instancePtr());

	ParameterManager* p = ParameterManager::instancePtr();
	Environment* env = Environment::instancePtr();

	QString args;
	if ( __ui->radioButtonMsmod->isChecked() ) {
		if ( __ui->radioButtonCustomArguments->isChecked() )
			args = __ui->lineEditArguments->text();
		else {
			if ( __ui->checkBoxApplyTimecorr->isChecked() )
			    args += "--applytimecorr " + __ui->lineEditApplyTimecorr->text() + " ";
			if ( __ui->checkBoxChanCode->isChecked() )
			    args += "--chan " + __ui->lineEditChanCode->text() + " ";
			if ( __ui->checkBoxLocCode->isChecked() )
			    args += "--loc " + __ui->lineEditLocCode->text() + " ";
			if ( __ui->checkBoxNetCode->isChecked() )
			    args += "--net " + __ui->lineEditNetCode->text() + " ";
			if ( __ui->checkBoxQuality->isChecked() )
			    args += "--quality " + __ui->lineEditQuality->text() + " ";
			if ( __ui->checkBoxSamplerate->isChecked() )
			    args += "--samprate " + __ui->lineEditSamplerate->text() + " ";
			if ( __ui->checkBoxStaCode->isChecked() )
			    args += "--sta " + __ui->lineEditStaCode->text() + " ";
			if ( __ui->checkBoxTimecorr->isChecked() )
			    args += "--timecorr " + __ui->lineEditTimecorr->text() + " ";
			if ( __ui->checkBoxTimecorrval->isChecked() )
			    args += "--timecorrval " + __ui->lineEditTimecorrval->text() + " ";
			if ( __ui->checkBoxTimeshift->isChecked() )
			    args += "--timeshift " + __ui->lineEditTimeshift->text() + " ";
		}
	}


	QString script;
	script += "#!" + p->parameter("BASH_BIN").toString() + ENDL;
	script += ENDL;
	script += "############################################################" + ENDL;
	script += "#" + TAB + PROJECT_NAME + "-" + PROJECT_VERSION + ENDL;
	script += "#" + TAB + "File type: dispatch script." + ENDL;
	script += "#" + TAB + "Datetime: " + QDateTime::currentDateTime().toString() + ENDL;
	script += "############################################################" + ENDL;
	script += ENDL;
	script += ENDL;
	script += "MSEEDDIR=" + p->parameter("Dispatch-SDSDir").toString() + ENDL;
	script += "MSEEDTMPDIR=${MSEEDDIR}/tmp/" + ENDL;
	if ( p->parameter("Dispatch-Msrouter").toBool() )
		script += "DISPATCHTOOL=" + p->parameter("MSROUTER_BIN").toString() + ENDL;
	else
		script += "DISPATCHTOOL=" + p->parameter("MSMOD_BIN").toString() + ENDL;
	script += ENDL;
	script += "LOGDIR=" + env->shareDir() + "/dispatch" + ENDL;
	script += "STDOUTLOG=${LOGDIR}/out.log" + ENDL;
	script += ENDL;
	script += "STARTDATE=$(date +\"%s\")" + ENDL;
	script += ENDL;
	script += "echo \"`date +%Y.%m.%d-%I.%M.%S` -- Dispatcher ready to operate and fill '${MSEEDDIR}' SDS archive\"" + ENDL;
	script += "mkdir -p ${LOGDIR}" + ENDL;
	script += "touch ${STDOUTLOG}" + ENDL;
	script += ENDL;
	script += "# Count the number of subfolders in current directory, ignore symlinks and" + ENDL;
	script += "# folders starting with '.' (hence the -2)" + ENDL;
	script += "DNB=$((`ls -l | grep -v ^l | wc -l`-2))" + ENDL;
	script += "didx=0" + ENDL;
	script += ENDL;
	script += "CFOLDER=`pwd`" + ENDL;
	script += "fcount=0" + ENDL;
	script += "for j in `ls 2>/dev/null`" + ENDL;
	script += "do" + ENDL;
	script += "    cd ${CFOLDER}/${j} 2>/dev/null # ignore subfolder parsing msg" + ENDL;
	script += "    FNB=$((`ls -l | grep -v ^l | wc -l`-2))" + ENDL;
	script += "    fidx=0" + ENDL;
	script += "    for i in `ls *.m* 2>/dev/null`" + ENDL;
	script += "    do" + ENDL;
	script += "	if test -f \"$i\"" + ENDL;
	script += "	then" + ENDL;
	if ( p->parameter("Dispatch-Msrouter").toBool() )
		script += "	    ${DISPATCHTOOL} -A $MSEEDDIR/" + p->parameter("Dispatch-SDSPattern").toString() + " $i &>${STDOUTLOG}" + ENDL;
	else
		script += "	    ${DISPATCHTOOL} " + args + " -A $MSEEDDIR/" + p->parameter("Dispatch-SDSPattern").toString() + " $i &>${STDOUTLOG}" + ENDL;
	script += "	    echo -n \"`date +%Y.%m.%d-%I.%M.%S` -- [$((${didx}*100/${DNB}))%] ";
	script += "[$((${fidx}*100/${FNB}))%] Dispatching file ${i} [$((DNB-didx))/$((FNB-fidx)) ";
	script += "files remaining]\"" + ENDL;
	script += "	    echo -n R | tr 'R' '\r'" + ENDL;
	script += "	    ((fcount+=1))" + ENDL;
	script += "	fi" + ENDL;
	script += "	((fidx+=1))" + ENDL;
	script += "    done" + ENDL;
	script += "    ((didx+=1))" + ENDL;
	script += "done" + ENDL;
	script += ENDL;
	script += ENDL;
	script += "ENDDATE=$(date +\"%s\")" + ENDL;
	script += "DIFF=$(($ENDDATE-$STARTDATE))" + ENDL;
	script += "echo \"Displatched ${fcount} file(s) from ${didx} folder(s) in ";
	script += "$((${DIFF} / 3600)) hour(s) $((${DIFF} / 60)) minute(s) and $((${DIFF} % 60)) second(s).\"" + ENDL;

	__scriptEditor->clear();
	__scriptEditor->insertPlainText(script);
}


void DispatchPanel::enqueueJob() {

	if ( __scriptEditor->toPlainText().isEmpty() ) return;

	EnqueueDialog d("Dispatch information:", this);
	d.exec();

	if ( !d.accepted() ) return;

	SDPASSERT(ActivityPanel::instancePtr());
	SDPASSERT(Logger::instancePtr());

	ActivityPanel* monitor = ActivityPanel::instancePtr();

	DispatchJob* job = DispatchJob::create();
	if ( job ) {
		job->setInformation(d.information());
		job->setComment(d.comment());
		job->setScriptData(QVariant::fromValue(__scriptEditor->toPlainText()));
		job->setTool((__ui->radioButtonMsrouter->isChecked()) ?
		    "msrouter" : "msmod");
		job->setMsdDir(__ui->lineEditDataDir->text());
		job->setMsdPattern(__ui->lineEditDataPattern->text());
		job->setSdsDir(__ui->lineEditSDSDir->text());
		job->setSdsPattern(__ui->lineEditSDSPattern->text());

		monitor->addJob(job);
	}
	else
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Failed to create dispatch job. Something went wrong...");
}


void DispatchPanel::showHideMsModOptions() {
	__ui->widgetOptions->setVisible(__ui->radioButtonMsmod->isChecked());
}


void DispatchPanel::showHideCustomOptions() {

	if ( __ui->radioButtonCustomParameters->isChecked() ) {
		__ui->widgetParameters->setVisible(true);
		__ui->lineEditArguments->setVisible(false);
	}
	else {
		__ui->widgetParameters->setVisible(false);
		__ui->lineEditArguments->setVisible(true);
	}
}


void DispatchPanel::selectDataDir() {
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select data source directory"));
	if ( dir.isEmpty() ) return;
	__ui->lineEditDataDir->setText(dir);
}


void DispatchPanel::selectSDSDir() {
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select SDS directory"));
	if ( dir.isEmpty() ) return;
	__ui->lineEditSDSDir->setText(dir);
}


HelpPanel::HelpPanel(QWidget* parent) :
		PanelWidget(parent) {

	setHeader(tr("Help"));
	setDescription(tr("Browse application manual and discover how it works."));

	QVBoxLayout* l = new QVBoxLayout(this);
	l->setMargin(0);
	__view = new QWebView(this);
	l->addWidget(__view);

	SDPASSERT(Environment::instancePtr());
	__view->load(QUrl::fromLocalFile(Environment::instancePtr()->docDir()
	    + QDir::separator() + "help" + QDir::separator() + "index.html"));
}


HelpPanel::~HelpPanel() {}


bool HelpPanel::saveParameters() {
	return true;
}


} // namespace Qt4
} // namespace SDP

