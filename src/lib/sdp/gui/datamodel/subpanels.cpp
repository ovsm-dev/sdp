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


#include <sdp/gui/datamodel/subpanels.h>

#include "../api.h"
#include <sdp/gui/datamodel/config.h>
#include <sdp/gui/datamodel/macros.h>
#include <sdp/gui/datamodel/mainframe.h>
#include <sdp/gui/datamodel/parametermanager.h>
#include <sdp/gui/datamodel/logger.h>
#include <sdp/gui/datamodel/progress.h>
#include <sdp/gui/datamodel/utils.h>

#include "ui_datasourcesubpanel.h"
#include "ui_inventorysubpanel.h"
#include "ui_timewindowsubpanel.h"
#include "ui_entity.h"
#include "ui_streamsubpanel.h"
#include "ui_triggersubpanel.h"

#include <QtGui>
#include <QtConcurrentRun>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

/** @brief Table headers enum */
enum StationTableHeaders {
	thNAME, thSTATIONCODE, thLOCATIONCODE, thCHANNELCODE
};

/**
 * @brief Table headers enum values
 *        The user has to make sure that those values matches (in order) the
 *        defined StationTableHeaders enum so it can properly be setup.
 *        When the name of an element has to be retrieved, the user needs only
 *        instantiate the vector containing the names:
 *        @code
 *                std::string elementName = InventoryTableHeaders[thMAGNITUDE];
 *                or
 *                std::string elementName = InventoryHeadersString[thMAGNITUDE];
 *        @encode
 */
const char* InventoryTableHeaders[] = { "Name", "Station code", "Location code", "Channel code" };
const std::vector<const char*> InventoryHeadersString(InventoryTableHeaders, InventoryTableHeaders
    + sizeof(InventoryTableHeaders) / sizeof(InventoryTableHeaders[0]));

/**
 * @brief  Retrieves header position from header vector.
 * @param  e the STableHeader enum value
 * @return The header's position
 * @note   Valid header position are okay above zero, fake underneath.
 */
int getHeaderPosition(const enum StationTableHeaders& e) {
	char* str;
	for (size_t i = 0; i < InventoryHeadersString.size(); ++i) {
		str = (char*) InventoryHeadersString.at(i);
		if ( str == InventoryTableHeaders[e] )
		    return (int) i;
	}
	return -1;
}

//! INFO: Define some output constants used for script generation
static QString const ENDL = "\n";
static QString const TAB = "    ";

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



namespace SDP {
namespace Qt4 {

DataSourceSubPanel::DataSourceSubPanel(QWidget* parent) :
		QWidget(parent), __ui(new Ui::DataSource) {

	__ui->setupUi(this);
	__ui->frameFile->hide();

	QButtonGroup* source = new QButtonGroup(this);
	source->addButton(__ui->radioButtonFile);
	source->addButton(__ui->radioButtonArclinkServer);

	connect(__ui->radioButtonFile, SIGNAL(toggled(bool)), __ui->frameFile, SLOT(setVisible(bool)));
	connect(__ui->radioButtonFile, SIGNAL(toggled(bool)), this, SIGNAL(setTimeWindowAccessible(bool)));
	connect(__ui->radioButtonArclinkServer, SIGNAL(toggled(bool)), __ui->frameArcServer, SLOT(setVisible(bool)));
	connect(__ui->radioButtonFileInventory, SIGNAL(toggled(bool)), this, SLOT(readFileInventory(bool)));
	connect(__ui->toolButtonFile, SIGNAL(clicked()), this, SLOT(selectDataFile()));

	__ui->radioButtonArclinkServer->setChecked(true);
	__ui->radioButtonCustomInventory->setChecked(true);
	__ui->radioButtonFileInventory->setEnabled(false);

	connect(__ui->comboBoxPreset, SIGNAL(currentIndexChanged(int)), this, SLOT(loadPreset(int)));

	loadConfiguration();

	SDPASSERT(ParameterManager::instancePtr());
	ParameterManager* pm = ParameterManager::instancePtr();

	//! Session parameters
	pm->registerParameter("dataSourceFile");
	pm->registerParameter("dataSourceArcserver");
	pm->registerParameter("dataSourceFilepath");
	pm->registerParameter("dataSourceArcserverAddress");
	pm->registerParameter("dataSourceArcserverPort");
	pm->registerParameter("dataSourceArcserverTimeout");
	pm->registerParameter("dataSourceArcserverUser");
	pm->registerParameter("dataSourceArcserverPassword");
	pm->registerParameter("dataSourceArcserverInstitution");

	//! Job parameters
	pm->registerObjectParameter("DataSourceSubPanel", "dataSourceFile");
	pm->registerObjectParameter("DataSourceSubPanel", "dataSourceArcserver");
	pm->registerObjectParameter("DataSourceSubPanel", "dataSourceFilepath");
	pm->registerObjectParameter("DataSourceSubPanel", "dataSourceArcserverAddress");
	pm->registerObjectParameter("DataSourceSubPanel", "dataSourceArcserverPort");
	pm->registerObjectParameter("DataSourceSubPanel", "dataSourceArcserverTimeout");
	pm->registerObjectParameter("DataSourceSubPanel", "dataSourceArcserverUser");
	pm->registerObjectParameter("DataSourceSubPanel", "dataSourceArcserverPassword");
	pm->registerObjectParameter("DataSourceSubPanel", "dataSourceArcserverInstitution");
}


DataSourceSubPanel::~DataSourceSubPanel() {}


bool DataSourceSubPanel::saveParameters() {

	SDPASSERT(ParameterManager::instancePtr());

	ParameterManager* w = ParameterManager::instancePtr();

	w->setParameter("dataSourceFile", QVariant::fromValue(__ui->radioButtonFile->isChecked()));
	w->setParameter("dataSourceFilepath", QVariant::fromValue(__ui->lineEditFile->text()));
	w->setParameter("dataSourceArcserver", QVariant::fromValue(__ui->radioButtonArclinkServer->isChecked()));
	w->setParameter("dataSourceArcserverAddress", QVariant::fromValue(__ui->lineEditArclinkServerAddress->text()));
	w->setParameter("dataSourceArcserverPort", QVariant::fromValue(__ui->spinBoxArclinkServerPort->value()));
	w->setParameter("dataSourceArcserverUser", QVariant::fromValue(__ui->lineEditArclinkServerUser->text()));
	w->setParameter("dataSourceArcserverPassword", QVariant::fromValue(__ui->lineEditArclinkServerPassword->text()));
	w->setParameter("dataSourceArcserverInstitution", QVariant::fromValue(__ui->lineEditArclnkServerInstitution->text()));

	w->setObjectParameter("DataSourceSubPanel", "dataSourceFile", QVariant::fromValue(__ui->radioButtonFile->isChecked()));
	w->setObjectParameter("DataSourceSubPanel", "dataSourceFilepath", QVariant::fromValue(__ui->lineEditFile->text()));
	w->setObjectParameter("DataSourceSubPanel", "dataSourceArcserver", QVariant::fromValue(__ui->radioButtonArclinkServer->isChecked()));
	w->setObjectParameter("DataSourceSubPanel", "dataSourceArcserverAddress", QVariant::fromValue(__ui->lineEditArclinkServerAddress->text()));
	w->setObjectParameter("DataSourceSubPanel", "dataSourceArcserverPort", QVariant::fromValue(__ui->spinBoxArclinkServerPort->value()));
	w->setObjectParameter("DataSourceSubPanel", "dataSourceArcserverUser", QVariant::fromValue(__ui->lineEditArclinkServerUser->text()));
	w->setObjectParameter("DataSourceSubPanel", "dataSourceArcserverPassword", QVariant::fromValue(__ui->lineEditArclinkServerPassword->text()));
	w->setObjectParameter("DataSourceSubPanel", "dataSourceArcserverInstitution", QVariant::fromValue(__ui->lineEditArclnkServerInstitution->text()));

	return true;
}


bool DataSourceSubPanel::loadParameters() {

	SDPASSERT(ParameterManager::instancePtr());
	ParameterManager* w = ParameterManager::instancePtr();

	__ui->radioButtonFile->setChecked(w->objectParameter("DataSourceSubPanel", "dataSourceFile").toBool());
	__ui->lineEditFile->setText(w->objectParameter("DataSourceSubPanel", "dataSourceFilepath").toString());
	__ui->radioButtonArclinkServer->setChecked(w->objectParameter("DataSourceSubPanel", "dataSourceArcserver").toBool());
	__ui->lineEditArclinkServerAddress->setText(w->objectParameter("DataSourceSubPanel", "dataSourceArcserverAddress").toString());
	__ui->spinBoxArclinkServerPort->setValue(w->objectParameter("DataSourceSubPanel", "dataSourceArcserverPort").toInt());
	__ui->lineEditArclinkServerUser->setText(w->objectParameter("DataSourceSubPanel", "dataSourceArcserverUser").toString());
	__ui->lineEditArclinkServerPassword->setText(w->objectParameter("DataSourceSubPanel", "dataSourceArcserverPassword").toString());
	__ui->lineEditArclnkServerInstitution->setText(w->objectParameter("DataSourceSubPanel", "dataSourceArcserverInstitution").toString());

	return true;
}


bool DataSourceSubPanel::loadConfiguration() {

	SDPASSERT(MainFrame::instancePtr());
	SDPASSERT(Logger::instancePtr());

	__presets.clear();
	Config* cfg = MainFrame::instancePtr()->config();

	QStringList presets;
	try {
		presets = cfg->getStrings("dataSource.presets");
	} catch ( ... ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Failed to read dataSource.presets from configuration file");
	}
	if ( presets.count() != 0 ) {
		for (int i = 0; i < presets.size(); ++i) {

			Preset p;
			p.idx = i + 1;
			p.name = presets.at(i).trimmed();
			try {
				p.sourceType = cfg->getString("dataSource.preset." + p.name + ".source");
			} catch ( ... ) {}
			try {
				p.sourceAddress = cfg->getString("dataSource.preset." + p.name + ".address");
			} catch ( ... ) {}
			try {
				p.arclinkUser = cfg->getString("dataSource.preset." + p.name + ".arclinkUser");
			} catch ( ... ) {}
			try {
				p.arclinkPassword = cfg->getString("dataSource.preset." + p.name + ".arclinkPassword");
			} catch ( ... ) {}
			try {
				p.arclinkPort = cfg->getInt("dataSource.preset." + p.name + ".arclinkPort");
			} catch ( ... ) {}
			try {
				p.arclinkInstitution = cfg->getString("dataSource.preset." + p.name + ".arclinkInstitution");
			} catch ( ... ) {}
			try {
				p.arclinkTimeout = cfg->getInt("dataSource.preset." + p.name + ".arclinkTimeout");
			} catch ( ... ) {}

			__presets << p;
		}
	}

	__ui->comboBoxPreset->clear();
	__ui->comboBoxPreset->insertItem(0, "-");
	for (int i = 0; i < __presets.size(); ++i)
		__ui->comboBoxPreset->insertItem(i + 1, __presets.at(i).name);

	return true;
}


bool DataSourceSubPanel::sourceIsOkay() {

	if ( __ui->radioButtonFile->isChecked()
	    && __ui->lineEditFile->text().isEmpty() ) {
		QMessageBox::critical(this, tr("Data source error"),
		    tr("You have specified that the source should be a file, but yet "
			    "you haven't specified the file to use!"));
		__ui->lineEditFile->setFocus();
		return false;
	}

	return true;
}


void DataSourceSubPanel::loadPreset(int idx) {

	for (int i = 0; i < __presets.size(); ++i) {
		if ( __presets.at(i).idx == idx ) {

			if ( __presets.at(i).sourceType.contains("arclink") ) {
				__ui->frameFile->hide();
				__ui->frameArcServer->show();
				__ui->radioButtonArclinkServer->setChecked(true);

				__ui->lineEditArclinkServerAddress->setText(__presets.at(i).sourceAddress);
				__ui->lineEditArclinkServerUser->setText(__presets.at(i).arclinkUser);
				__ui->lineEditArclinkServerPassword->setText(__presets.at(i).arclinkPassword);
				__ui->lineEditArclnkServerInstitution->setText(__presets.at(i).arclinkInstitution);
				__ui->spinBoxArclinkServerPort->setValue(__presets.at(i).arclinkPort);
				__ui->spinBoxArclinkServerTimeout->setValue(__presets.at(i).arclinkTimeout);
			}
			else if ( __presets.at(i).sourceType.contains("file") ) {
				__ui->frameArcServer->hide();
				__ui->frameFile->show();
				__ui->radioButtonFile->setChecked(true);
				__ui->lineEditFile->setText(__presets.at(i).sourceAddress);
			}
		}
	}
}


void DataSourceSubPanel::selectDataFile() {

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Waveform files (*.*)"));
	if ( fileName.isEmpty() ) return;

	__ui->lineEditFile->setText(fileName);
	__ui->radioButtonFileInventory->setEnabled(true);
}


void DataSourceSubPanel::readFileInventory(bool checked) {

	/*
	 #ifndef QT_NO_CONCURRENT
	 prog.show("Loading inventory from miniSEED file");
	 QFuture<void> f = QtConcurrent::run(this, &DataSourceSubPanel::readSEEDInventory);
	 f.waitForFinished();
	 prog.hide();
	 #else
	 readSEEDInventory();
	 #endif
	 */
	if ( checked )
	    readSEEDInventory();
}


void DataSourceSubPanel::readSEEDInventory() {

	//! TODO: Find a way to verify that the file specified is a legti miniSEED
	//! file, otherwise, this all operation could go boom...

	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(Logger::instancePtr());
	SDPASSERT(InventorySubPanel::instancePtr());

	ParameterManager* pm = ParameterManager::instancePtr();
	Logger* log = Logger::instancePtr();
	InventorySubPanel* inv = InventorySubPanel::instancePtr();
	Progress prog(this, Qt::FramelessWindowHint);

	const QString inventory = pm->parameter("DATA_DIR").toString() + QDir::separator() + "streams.list";
	QString script;
	script += "#!" + pm->parameter("ENV_BIN").toString() + " python" + ENDL;
	script += ENDL;
	script += "\"\"\"" + ENDL;
	script += "############################################################" + ENDL;
	script += TAB + PROJECT_NAME + "-" + PROJECT_VERSION + ENDL;
	script += TAB + "File type: stream list script." + ENDL;
	script += TAB + "Datetime: " + QDateTime::currentDateTime().toString() + ENDL;
	script += "############################################################" + ENDL;
	script += "\"\"\"" + ENDL;
	script += ENDL;
	script += "from obspy import read" + ENDL;
	script += "st = read(\"" + __ui->lineEditFile->text() + "\")" + ENDL;
	script += "with open(\"" + inventory + "\", \"w\") as inventory:" + ENDL;
	script += TAB + "inventory.write(\"%s\" % st)" + ENDL;
	script += ENDL;

	const QString scriptFile = pm->parameter("DATA_DIR").toString() + QDir::separator() + "liststreams.py";

	if ( !Utils::writeScript(scriptFile, script) ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to write script "
		    + scriptFile + " for inventory identification from stream");
		return;
	}
	if ( !Utils::fileExists(scriptFile) ) {
		log->addMessage(Logger::CRITICAL, __func__,
		    "Didn't find written python script " + scriptFile + " for inventory identification");
		return;
	}

	prog.show("Loading inventory from miniSEED file");

	QProcess proc(this);
	proc.setWorkingDirectory(pm->parameter("DATA_DIR").toString());
	proc.start(QString("%1 -u %2")
	    .arg(pm->parameter("PYTHON_BIN").toString())
	    .arg(scriptFile), QProcess::Unbuffered | QProcess::ReadOnly);
	if ( !proc.waitForStarted() ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to wait for python process to start");
		prog.hide();
		return;
	}

	if ( !proc.waitForFinished() ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to wait python process to terminate");
		prog.hide();
		return;
	}

	if ( !Utils::fileExists(inventory) ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to read python generated output " + inventory);
		prog.hide();
		return;
	}


	QFile file(inventory);
	if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		log->addMessage(Logger::WARNING, __func__,
		    "Failed to open inventory file " + inventory);
		prog.hide();
		return;
	}

	QTextStream in(&file);
	while ( !in.atEnd() ) {
		QString line = in.readLine();
		if ( line.isEmpty() ) continue;
		if ( !line.contains('|') ) continue;
		QStringList list = line.split('|');
		if ( list.size() != 3 ) continue;

		QStringList data = list[0].trimmed().split('.');
		if ( data.size() < 2 ) {
			log->addMessage(Logger::WARNING, __func__,
			    "Wrong inventory net+sta+loc+chan (" + line + ")");
			continue;
		}
		inv->addNetwork(data.at(0));
		inv->addStation(data.at(0), data.at(1),
		    data.size() == 4 ? data.at(2) : "",
		    data.size() == 4 ? data.at(3) : "");
	}
	file.close();

	prog.hide();

	QMessageBox::information(this, tr("Populate inventory"),
	    QString("Inventory has been fed with %1 entities").arg(inv->entities().size()));
}


StreamSubPanel::StreamSubPanel(QWidget* parent) :
		QWidget(parent), __ui(new Ui::Stream) {

	__ui->setupUi(this);

	__ui->lineEditCustom->setEnabled(false);
	__ui->radioButtonZ->setChecked(true);

	connect(__ui->radioButtonCustom, SIGNAL(toggled(bool)), __ui->lineEditCustom, SLOT(setEnabled(bool)));
	connect(__ui->comboBoxPreset, SIGNAL(currentIndexChanged(int)), this, SLOT(loadPreset(int)));

	loadConfiguration();

	SDPASSERT(ParameterManager::instancePtr());
	ParameterManager* pm = ParameterManager::instancePtr();

	//! Session parameters
	pm->registerParameter("streamChannelAll");
	pm->registerParameter("streamChannelZOnly");
	pm->registerParameter("streamChannelNSOnly");
	pm->registerParameter("streamChannelEWOnly");
	pm->registerParameter("streamChannelCustom");
	pm->registerParameter("streamChannelCustomValue");

	//! Job parameters
	pm->registerObjectParameter("StreamSubPanel", "streamChannelAll");
	pm->registerObjectParameter("StreamSubPanel", "streamChannelZOnly");
	pm->registerObjectParameter("StreamSubPanel", "streamChannelNSOnly");
	pm->registerObjectParameter("StreamSubPanel", "streamChannelEWOnly");
	pm->registerObjectParameter("StreamSubPanel", "streamChannelCustom");
	pm->registerObjectParameter("StreamSubPanel", "streamChannelCustomValue");
}


StreamSubPanel::~StreamSubPanel() {}


bool StreamSubPanel::saveParameters() {

	SDPASSERT(ParameterManager::instancePtr());

	ParameterManager* pm = ParameterManager::instancePtr();

	pm->setParameter("streamChannelAll", QVariant::fromValue(__ui->radioButtonAll->isChecked()));
	pm->setParameter("streamChannelZOnly", QVariant::fromValue(__ui->radioButtonZ->isChecked()));
	pm->setParameter("streamChannelNSOnly", QVariant::fromValue(__ui->radioButtonNS->isChecked()));
	pm->setParameter("streamChannelEWOnly", QVariant::fromValue(__ui->radioButtonEW->isChecked()));
	pm->setParameter("streamChannelCustom", QVariant::fromValue(__ui->radioButtonCustom->isChecked()));
	pm->setParameter("streamChannelCustomValue", QVariant::fromValue(__ui->lineEditCustom->text()));

	pm->setObjectParameter("StreamSubPanel", "streamChannelAll", QVariant::fromValue(__ui->radioButtonAll->isChecked()));
	pm->setObjectParameter("StreamSubPanel", "streamChannelZOnly", QVariant::fromValue(__ui->radioButtonZ->isChecked()));
	pm->setObjectParameter("StreamSubPanel", "streamChannelNSOnly", QVariant::fromValue(__ui->radioButtonNS->isChecked()));
	pm->setObjectParameter("StreamSubPanel", "streamChannelEWOnly", QVariant::fromValue(__ui->radioButtonEW->isChecked()));
	pm->setObjectParameter("StreamSubPanel", "streamChannelCustom", QVariant::fromValue(__ui->radioButtonCustom->isChecked()));
	pm->setObjectParameter("StreamSubPanel", "streamChannelCustomValue", QVariant::fromValue(__ui->lineEditCustom->text()));

	return true;
}


bool StreamSubPanel::loadParameters() {

	SDPASSERT(ParameterManager::instancePtr());
	ParameterManager* pm = ParameterManager::instancePtr();

	__ui->radioButtonAll->setChecked(pm->objectParameter("StreamSubPanel", "streamChannelAll").toBool());
	__ui->radioButtonZ->setChecked(pm->objectParameter("StreamSubPanel", "streamChannelZOnly").toBool());
	__ui->radioButtonNS->setChecked(pm->objectParameter("StreamSubPanel", "streamChannelNSOnly").toBool());
	__ui->radioButtonEW->setChecked(pm->objectParameter("StreamSubPanel", "streamChannelEWOnly").toBool());
	__ui->radioButtonCustom->setChecked(pm->objectParameter("StreamSubPanel", "streamChannelCustom").toBool());
	__ui->lineEditCustom->setText(pm->objectParameter("StreamSubPanel", "streamChannelCustomValue").toString());

	return true;
}


bool StreamSubPanel::loadConfiguration() {

	SDPASSERT(MainFrame::instancePtr());
	SDPASSERT(Logger::instancePtr());

	__presets.clear();
	Config* cfg = MainFrame::instancePtr()->config();

	QStringList presets;
	try {
		presets = cfg->getStrings("stream.presets");
	} catch ( ... ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Failed to read dataSource.presets from configuration file");
	}
	if ( presets.count() != 0 ) {
		for (int i = 0; i < presets.size(); ++i) {

			Preset p;
			p.idx = i + 1;
			p.name = presets.at(i).trimmed();
			try {
				p.type = cfg->getString("stream.preset." + p.name + ".preferredChannel");
			} catch ( ... ) {}
			try {
				p.customStream = cfg->getString("stream.preset." + p.name + ".customChannels");
			} catch ( ... ) {}

			__presets << p;
		}
	}

	__ui->comboBoxPreset->clear();
	__ui->comboBoxPreset->insertItem(0, "-");
	for (int i = 0; i < __presets.size(); ++i)
		__ui->comboBoxPreset->insertItem(i + 1, __presets.at(i).name);

	return true;
}


bool StreamSubPanel::streamIsOkay() {

	if ( __ui->radioButtonCustom->isChecked()
	    && __ui->lineEditCustom->text().isEmpty() ) {
		QMessageBox::critical(this, tr("Stream selection error"),
		    tr("You have checked the custom stream channel box, but yet "
			    "you haven't typed in the channel(s) to be exploited!"));
		__ui->lineEditCustom->setFocus();
		return false;
	}

	return true;
}


void StreamSubPanel::loadPreset(int idx) {

	for (int i = 0; i < __presets.size(); ++i) {
		if ( __presets.at(i).idx == idx ) {

			if ( __presets.at(i).type.contains("ALL") ) {
				__ui->radioButtonAll->setChecked(true);
				__ui->lineEditCustom->setEnabled(false);
			}
			else if ( __presets.at(i).type.contains("Z") ) {
				__ui->radioButtonZ->setChecked(true);
				__ui->lineEditCustom->setEnabled(false);
			}
			else if ( __presets.at(i).type.contains("NS") ) {
				__ui->radioButtonNS->setChecked(true);
				__ui->lineEditCustom->setEnabled(false);
			}
			else if ( __presets.at(i).type.contains("EW") ) {
				__ui->radioButtonEW->setChecked(true);
				__ui->lineEditCustom->setEnabled(false);
			}
			else if ( __presets.at(i).type.contains("CUSTOM") ) {
				__ui->radioButtonCustom->setChecked(true);
				__ui->lineEditCustom->setEnabled(true);
				__ui->lineEditCustom->setText(__presets.at(i).customStream);
			}
		}
	}
}


ModalDialog::ModalDialog(const Type& t, QWidget* parent) :
		QDialog(parent), __ui(new Ui::Entity), __object(NULL), __t(t),
		__mode(Creation) {

	__ui->setupUi(this);

	if ( t == Network ) {
		__ui->label->setText(tr("Add new network to inventory"));
		__ui->frame->setVisible(false);
		resize(100, 100);
	}
	else {
		__ui->label->setText(tr("Add new station to inventory"));

		if ( InventorySubPanel::instancePtr() )
		    __ui->labelNetworkCode->setText(InventorySubPanel::instancePtr()->selectedEntity()->name);
	}

	connect(__ui->buttonBox, SIGNAL(accepted()), this, SLOT(checkForm()));
	connect(__ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


ModalDialog::ModalDialog(void* e, QWidget* parent) :
		QDialog(parent), __ui(new Ui::Entity), __object(e), __mode(Edition) {

	__ui->setupUi(this);

	InventorySubPanel::Entity* entity = reinterpret_cast<InventorySubPanel::Entity*>(e);

	if ( entity ) {
		if ( !entity->parent ) {
			__ui->label->setText(tr("Edit network"));
			__ui->lineEditCode->setText(entity->name);
			__ui->frame->setVisible(false);
			resize(100, 100);
		}
		else {
			__ui->label->setText(tr("Edit station"));
			__ui->lineEditCode->setText(entity->name);
			__ui->labelNetworkCode->setText(entity->parent->name);
			__ui->lineEditLocationCode->setText(entity->locationCode);
			__ui->lineEditChannelCode->setText(entity->channelCode);
		}
	}

	connect(__ui->buttonBox, SIGNAL(accepted()), this, SLOT(checkForm()));
	connect(__ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


ModalDialog::~ModalDialog() {}


void ModalDialog::checkForm() {

	if ( __ui->lineEditCode->text().isEmpty() ) {
		QMessageBox::critical(this, tr("Error"), tr("Entity name must be filled"));
		return;
	}

	if ( __mode == Creation ) {
		InventorySubPanel::Entity* ent = NULL;
		bool isOkay = false;

		if ( __t == Network ) {

			if ( InventorySubPanel::instancePtr() )
			    ent = InventorySubPanel::instancePtr()->getNetwork(__ui->lineEditCode->text());
			if ( ent ) {
				QMessageBox::warning(this, tr("Warning"),
				    QString("It seems like %1 network exists in inventory.")
				        .arg(__ui->lineEditCode->text()));
				return;
			}
			isOkay = true;
			emit newNetwork(__ui->lineEditCode->text());
		}
		else {

			if ( InventorySubPanel::instancePtr() )
			    ent = InventorySubPanel::instancePtr()->getStation(__ui->labelNetworkCode->text(), __ui->lineEditCode->text());

			if ( ent ) {
				QMessageBox::warning(this, tr("Warning"),
				    QString("It seems like station %1 from %2 network exists in inventory.")
				        .arg(__ui->lineEditCode->text())
				        .arg(__ui->labelNetworkCode->text()));
				return;
			}

			if ( __ui->lineEditChannelCode->text().isEmpty() )
			    QMessageBox::information(this, tr("Information"),
			        tr("You haven't specified any channel code for this station.\n"
				        "Empty channel code will be filled with '*'."));

			if ( __ui->lineEditCode->text().count() > 5 ) {
				QMessageBox::critical(this, tr("Error"),
				    tr("The specified station code is not correct. chars > 5"));
				return;
			}

			if ( __ui->lineEditChannelCode->text().count() > 3 ) {
				QMessageBox::critical(this, tr("Error"),
				    tr("The specified channel code is not correct. chars > 3"));
				return;
			}

			isOkay = true;
			emit newStation(__ui->labelNetworkCode->text(),
			    __ui->lineEditCode->text(), __ui->lineEditLocationCode->text(),
			    __ui->lineEditChannelCode->text());
		}

		if ( isOkay )
		    accept();
	}
	else {
		if ( __ui->lineEditChannelCode->text().isEmpty() )
		    QMessageBox::information(this, tr("Information"),
		        tr("You haven't specified any channel code for this station.\n"
			        "Empty channel code will be filled with '*'."));

		InventorySubPanel::Entity* entity = reinterpret_cast<InventorySubPanel::Entity*>(__object);
		if ( entity && entity->parent )
		    emit editStation(entity->parent->name, entity->name,
		        __ui->lineEditCode->text(), __ui->lineEditLocationCode->text(),
		        __ui->lineEditChannelCode->text());

		accept();
	}
}



InventorySubPanel::Entity::Entity(const QString& n_, Entity* p) :
		parent(p), item(NULL), name(n_) {}


InventorySubPanel::Entity::Entity::~Entity() {
	parent = NULL;
}


InventorySubPanel::InventorySubPanel(QWidget* parent) :
		QWidget(parent), __ui(new Ui::Inventory), __selectedEntity(NULL) {

	__ui->setupUi(this);

	QStringList headers;
	for (size_t i = 0; i < InventoryHeadersString.size(); ++i)
		headers << InventoryHeadersString[i];

	__ui->treeWidgetInventory->setColumnCount(InventoryHeadersString.size());
	__ui->treeWidgetInventory->setHeaderLabels(headers);
	__ui->treeWidgetInventory->sortByColumn(0, Qt::AscendingOrder);

	connect(__ui->treeWidgetInventory, SIGNAL(customContextMenuRequested(const QPoint&)),
	    this, SLOT(showContextMenu(const QPoint&)));
	connect(__ui->radioButtonUseDataless, SIGNAL(toggled(bool)), __ui->lineEdit, SLOT(setEnabled(bool)));
	connect(__ui->radioButtonUseStationsSummary, SIGNAL(toggled(bool)), __ui->lineEdit, SLOT(setEnabled(bool)));
	connect(__ui->radioButtonUseDataless, SIGNAL(toggled(bool)), __ui->toolButton, SLOT(setEnabled(bool)));
	connect(__ui->radioButtonUseStationsSummary, SIGNAL(toggled(bool)), __ui->toolButton, SLOT(setEnabled(bool)));
	connect(__ui->toolButton, SIGNAL(clicked()), this, SLOT(loadInventoryFromFile()));

	__ui->lineEdit->setEnabled(false);
	__ui->toolButton->setEnabled(false);
	__ui->radioButtonUseCustom->setChecked(true);
}


InventorySubPanel::~InventorySubPanel() {
	qDeleteAll(__entities);
}


bool InventorySubPanel::saveParameters() {

	size_t stationCount = 0;
	for (int i = 0; i < __entities.size(); ++i)
		if ( __entities.at(i)->parent )
		    ++stationCount;
	if ( stationCount ) return true;

	return false;
}


bool InventorySubPanel::loadParameters() {

	//! Clean up entities in list
	QList<Entity*> children;
	for (int i = 0; i < __entities.size(); ++i)
		if ( __entities.at(i)->parent )
		    children << __entities.at(i);

	for (int i = 0; i < children.size(); ++i)
		removeStation(children.at(i)->parent->name, children.at(i)->name);

	for (int i = 0; i < __entities.size(); ++i)
		if ( !__entities.at(i)->parent )
		    delete __entities.takeAt(i);

	__entities.clear();

	//! Clean up entities in tree
	__ui->treeWidgetInventory->clear();



	return true;
}


InventorySubPanel::Entity*
InventorySubPanel::getNetwork(const QString& code) {

	for (EntityList::const_iterator it = __entities.constBegin();
	        it != __entities.constEnd(); ++it)
		if ( (*it)->name == code && !(*it)->parent )
		    return (*it);

	return NULL;
}


InventorySubPanel::Entity*
InventorySubPanel::getStation(const QString& networkCode,
                              const QString& stationCode) {

	for (EntityList::const_iterator it = __entities.constBegin();
	        it != __entities.constEnd(); ++it) {
		if ( !(*it)->parent ) continue;
		if ( (*it)->parent->name == networkCode && (*it)->name == stationCode )
		    return (*it);
	}

	return NULL;
}


void InventorySubPanel::removeNetwork(const QString& code) {

	QList<Entity*> children;
	for (int i = 0; i < __entities.size(); ++i) {
		if ( __entities.at(i)->parent )
		    if ( __entities.at(i)->parent->name == code )
		        children << __entities.at(i);
	}

	for (int i = 0; i < children.size(); ++i)
		removeStation(children.at(i)->parent->name, children.at(i)->name);

	for (int i = 0; i < __entities.size(); ++i) {
		if ( __entities.at(i)->name == code && !__entities.at(i)->parent )
		    delete __entities.takeAt(i);
	}
}


void InventorySubPanel::removeStation(const QString& networkCode,
                                      const QString& stationCode) {

	for (int i = 0; i < __entities.size(); ++i) {
		if ( __entities.at(i)->parent )
		    if ( __entities.at(i)->parent->name == networkCode
		        && __entities.at(i)->name == stationCode )
		        delete __entities.takeAt(i);
	}
}


QString InventorySubPanel::pythonStations(const QString& netVar,
                                          const QString& staVar) {

	bool isFirstItem = true;
	QString script = netVar + " = [ ";
	for (int i = 0; i < __entities.size(); ++i) {
		if ( !__entities.at(i)->parent ) continue;
		if ( isFirstItem ) {
			script += "\"" + __entities.at(i)->parent->name + "\"";
			isFirstItem = false;
		}
		else
			script += ", \"" + __entities.at(i)->parent->name + "\"";
	}
	script += " ]" + ENDL;

	isFirstItem = true;
	script += staVar + " = [ ";
	for (int i = 0; i < __entities.size(); ++i) {
		if ( !__entities.at(i)->parent ) continue;
		if ( isFirstItem ) {
			script += "\"" + __entities.at(i)->name + "\"";
			isFirstItem = false;
		}
		else
			script += ", \"" + __entities.at(i)->name + "\"";
	}
	script += " ]" + ENDL;

	return script;
}


int InventorySubPanel::networkCount() {

	int networkCount = 0;
	for (int i = 0; i < __entities.size(); ++i) {
		if ( __entities.at(i)->parent ) continue;
		networkCount++;
	}

	return networkCount;
}


int InventorySubPanel::stationCount() {

	int stationCount = 0;
	for (int i = 0; i < __entities.size(); ++i) {
		if ( !__entities.at(i)->parent ) continue;
		stationCount++;
	}

	return stationCount;
}


void InventorySubPanel::loadASCIIDataless(const QString& filename,
                                          ASCIIDatalessFormat f) {

	SDPASSERT(Logger::instancePtr());
	Logger* log = Logger::instancePtr();

	QFile file(filename);
	if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		log->addMessage(Logger::WARNING, __func__, "Failed to open response station file " + filename);
		return;
	}

	QTextStream in(&file);
	while ( !in.atEnd() ) {

		QString line = in.readLine();

		if ( line.isEmpty() ) continue;

		if ( f == Custom ) {

			//! This small hack is to make sure the file format is as we'd like
			//! it to be : 14 fields delimited by comma. The said format should
			//! be like this :
			//! ZU,CAPH,19.7355,-72.1883,17.0,2013-08-2T00:00:00,2013-10-10T23:59:00,HHZ, ,HHN, ,HHE, ,
			//! Although I don't recall this being a famous format, that's at
			//! the moment the only material we can sink our teeth into.
			//! TODO: Make this way more configurable in the future, and if
			//! possible, eventually read pure dataless binary files...

			QStringList xline = line.split(',');
			if ( xline.size() != 14 ) continue;

			RespStation s;
			s.network = xline.at(0).trimmed();
			s.code = xline.at(1).trimmed();
			s.latitude = xline.at(2).toDouble();
			s.longitude = xline.at(3).toDouble();
			s.elevation = xline.at(4).toDouble();
			s.start = QDateTime::fromString(xline.at(5).trimmed(), Qt::ISODate);
			s.end = QDateTime::fromString(xline.at(6).trimmed(), Qt::ISODate);
			s.locchan.push_back(PairedString(xline.at(7).trimmed(), xline.at(8).trimmed()));
			s.locchan.push_back(PairedString(xline.at(9).trimmed(), xline.at(10).trimmed()));
			s.locchan.push_back(PairedString(xline.at(11).trimmed(), xline.at(12).trimmed()));

			if ( __stations.contains(s) ) continue;

			__stations << s;
			log->addMessage(Logger::DEBUG, __func__,
			    "Fed inventory with station " + s.code + " from network " + s.network, false);
		}
		else if ( f == RdseedStationSummary ) {

			//! Only read first 44 chars
			if ( line.size() < 45 ) continue;
			QString formatted = line.replace(" ", ",");
			QStringList xline = line.left(44).split(",");

			RespStation s;
			s.network = xline.at(1).trimmed();
			s.code = xline.at(0).trimmed();
			s.latitude = xline.at(2).toDouble();
			s.longitude = xline.at(3).toDouble();
			s.elevation = xline.at(4).toDouble();
//			s.start = QDateTime::fromString(xline.at(5).trimmed(), Qt::ISODate);
//			s.end = QDateTime::fromString(xline.at(6).trimmed(), Qt::ISODate);
//			s.locchan.push_back(PairedString(xline.at(7).trimmed(), xline.at(8).trimmed()));
//			s.locchan.push_back(PairedString(xline.at(9).trimmed(), xline.at(10).trimmed()));
//			s.locchan.push_back(PairedString(xline.at(11).trimmed(), xline.at(12).trimmed()));

			if ( __stations.contains(s) ) continue;

			__stations << s;
			log->addMessage(Logger::DEBUG, __func__,
			    "Fed inventory with station " + s.code + " from network " + s.network, false);
		}
	}

	file.close();
}


void InventorySubPanel::loadBinaryDataless(const QString& filename) {

	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(Logger::instancePtr());
//	SDPASSERT(Progress::instancePtr());

	ParameterManager* pm = ParameterManager::instancePtr();
	Logger* log = Logger::instancePtr();
	Progress prog(this, Qt::FramelessWindowHint);

	if ( !Utils::fileExists(pm->parameter("RDSEED_BIN").toString()) ) {
		QMessageBox::critical(this, tr("rdseed error"),
		    QString("This operation requires rdseed, please verify that you've "
			    "correctly specified its location. (User param: %1)")
		        .arg(pm->parameter("RDSEED_BIN").toString()));
		return;
	}

	//! Generate stations summary info
	QString scriptData = "#!" + pm->parameter("BASH_BIN").toString() + "\n";
	scriptData += pm->parameter("RDSEED_BIN").toString() + " -Sf " + filename
	    + " -q " + pm->parameter("DATA_DIR").toString() + QDir::separator() + "\n";
	const QString scriptFile = pm->parameter("DATA_DIR").toString() + QDir::separator() + "convertdataless.sh";

	if ( !Utils::writeScript(scriptFile, scriptData) ) {
		log->addMessage(Logger::CRITICAL, __func__,
		    "Failed to write rdseed script for file " + filename);
		return;
	}
	if ( !Utils::fileExists(scriptFile) ) {
		log->addMessage(Logger::CRITICAL, __func__,
		    "Didn't find written rdseed script " + scriptFile + " for file " + filename);
		return;
	}

	prog.show("Loading binary dataless");

	QProcess proc(this);
	proc.setWorkingDirectory(pm->parameter("DATA_DIR").toString());
	proc.start(QString("%1 %2")
	    .arg(pm->parameter("BASH_BIN").toString())
	    .arg(scriptFile), QProcess::Unbuffered | QProcess::ReadOnly);
	if ( !proc.waitForStarted() ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to wait for rdseed process to start");
		prog.hide();
		return;
	}

	if ( !proc.waitForFinished() ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to wait rdseed process to terminate");
		prog.hide();
		return;
	}

	const QString output = pm->parameter("DATA_DIR").toString() + QDir::separator() + "rdseed.stations";
	if ( !Utils::fileExists(output) ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to read rdseed generated output " + output);
		prog.hide();
		return;
	}

	loadASCIIDataless(output, RdseedStationSummary);
	prog.hide();
}


bool InventorySubPanel::inventoryIsOkay() {

	if ( __ui->radioButtonUseDataless->isChecked()
	    && __ui->lineEdit->text().isEmpty() ) {
		QMessageBox::critical(this, tr("Inventory error"),
		    tr("You have choose to use a SEED dataless, but yet you "
			    "haven't specified any file containing the latter!"));
		__ui->lineEdit->setFocus();
		return false;
	}

	return true;
}


void InventorySubPanel::addNetwork(const QString& code) {

	SDPASSERT(Logger::instancePtr());
	if ( getNetwork(code) ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Skipped adding network " + code + " in inventory, item already present", false);
		return;
	}

	Entity* ent = new Entity(code);
	__entities << ent;

	QTreeWidgetItem* obj = new QTreeWidgetItem(__ui->treeWidgetInventory);
	obj->setText(getHeaderPosition(thNAME), code);
	obj->setIcon(getHeaderPosition(thNAME), QIcon(":images/network.png"));
	obj->setData(getHeaderPosition(thNAME), Qt::UserRole, Utils::VariantPtr<
	        Entity>::asQVariant(__entities.last()));

	ent->item = obj;
}


void InventorySubPanel::addStation(const QString& networkCode,
                                   const QString& stationCode,
                                   const QString& locationCode,
                                   const QString& channelCode) {

	SDPASSERT(Logger::instancePtr());
	Entity* station = getStation(networkCode, stationCode);
	if ( station ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Skipped adding station " + networkCode + QDir::separator() + stationCode
		        + " in inventory, item already present", false);
		return;
	}

	Entity* parent = getNetwork(networkCode);
	if ( !parent ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Failed to find a parent for station " + networkCode + "." + stationCode, false);
		return;
	}

	QTreeWidgetItem* itm = parent->item;
	if ( !itm ) itm = __ui->treeWidgetInventory->currentItem();

	if ( !itm ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Failed to acquire parent item for station " + networkCode + "." + stationCode);
		return;
	}

	Entity* ent = new Entity(stationCode, parent);
	ent->locationCode = locationCode;
	ent->channelCode = channelCode;

	__entities << ent;
	QTreeWidgetItem* obj = new QTreeWidgetItem;
	obj->setData(getHeaderPosition(thNAME), Qt::UserRole, Utils::VariantPtr<
	        Entity>::asQVariant(parent));
	obj->setIcon(getHeaderPosition(thNAME), QIcon(":images/station.png"));
	obj->setText(getHeaderPosition(thSTATIONCODE), stationCode);
	obj->setText(getHeaderPosition(thLOCATIONCODE), locationCode);
	obj->setText(getHeaderPosition(thCHANNELCODE), channelCode);

	Entity* node = Utils::VariantPtr<Entity>::asPtr(itm->data(getHeaderPosition(thNAME), Qt::UserRole));
	if ( node ) {
		if ( node->parent )
			itm->parent()->addChild(obj);
		else
			itm->addChild(obj);
	}
}



void InventorySubPanel::editStation(const QString& networkCode,
                                    const QString& oldStationCode,
                                    const QString& stationCode,
                                    const QString& locationCode,
                                    const QString& channelCode) {

	SDPASSERT(Logger::instancePtr());
	Entity* parent = getNetwork(networkCode);
	if ( !parent ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Did not find network " + networkCode + " from which station should be edited");
		return;
	}

	Entity* station = getStation(networkCode, oldStationCode);
	if ( !station ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Did not find station " + oldStationCode + " that has to be edited");
		return;
	}

	QTreeWidgetItem* itm = __ui->treeWidgetInventory->currentItem();
	if ( itm ) {
		station->name = stationCode;
		itm->setText(getHeaderPosition(thSTATIONCODE), stationCode);
		itm->setText(getHeaderPosition(thLOCATIONCODE), locationCode);
		itm->setText(getHeaderPosition(thCHANNELCODE), channelCode);
	}
}


void InventorySubPanel::showContextMenu(const QPoint& pos) {

	Q_UNUSED(pos);

	QMenu* menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose);
	QAction* netAction = menu->addAction(QIcon(":images/network.png"),
	    tr("Add new network"), this, SLOT(addNetwork()));

	QTreeWidgetItem* itm = __ui->treeWidgetInventory->currentItem();
	if ( itm ) {

		Entity* network = getNetwork(itm->text(getHeaderPosition(thNAME)));
		if ( network ) {
			menu->addAction(QIcon(":images/station.png"),
			    QString("Add station to %1's network").arg(itm->text(getHeaderPosition(thNAME))),
			    this, SLOT(addStation()));
			menu->addAction(QIcon(":images/remove.png"),
			    QString("Remove %1's network").arg(itm->text(getHeaderPosition(thNAME))),
			    this, SLOT(removeEntity()));
			__selectedEntity = network;
		}
		else {
			Entity* station = getStation(itm->parent()->text(getHeaderPosition(thNAME)),
			    itm->text(getHeaderPosition(thSTATIONCODE)));
			if ( station ) {
				netAction->setEnabled(false);
				menu->addAction(QIcon(":images/remove.png"),
				    QString("Edit %1 station").arg(itm->text(getHeaderPosition(thSTATIONCODE))),
				    this, SLOT(editEntity()));
				menu->addAction(QIcon(":images/remove.png"),
				    QString("Remove %1 station").arg(itm->text(getHeaderPosition(thSTATIONCODE))),
				    this, SLOT(removeEntity()));
				__selectedEntity = station;
			}
		}
	}

	menu->exec(QCursor::pos());
}


void InventorySubPanel::loadInventoryFromFile() {

	SDPASSERT(Logger::instancePtr());
	QStringList files = QFileDialog::getOpenFileNames(this, tr("Select dataless files directory"));

	QApplication::processEvents();

	for (int i = 0; i < files.count(); ++i) {

		if ( __ui->radioButtonUseDataless->isChecked() )
			loadBinaryDataless(files.at(i));
		else if ( __ui->radioButtonUseStationsSummary->isChecked() )
		    loadASCIIDataless(files.at(i));
	}

	if ( __stations.count() == 0 ) {
		Logger::instancePtr()->addMessage(Logger::DEBUG, __func__, "No station added into inventory.");
		return;
	}

	for (int i = 0; i < __stations.size(); ++i) {

		addNetwork(__stations.at(i).network);

		QString chans;
		int idx = 0;
		for (PairedStringList::const_iterator it = __stations.at(i).locchan.constBegin();
		        it != __stations.at(i).locchan.constEnd(); ++it, ++idx) {
			if ( idx != __stations.at(i).locchan.size() - 1 )
				chans += (*it).first + ", ";
			else
				chans += (*it).first;
		}

		if ( __stations.at(i).locchan.size() != 0 )
			addStation(__stations.at(i).network, __stations.at(i).code,
			    __stations.at(i).locchan[0].second, chans);
		else
			addStation(__stations.at(i).network, __stations.at(i).code, "", "");
	}

	__ui->treeWidgetInventory->expandAll();

	if ( files.size() == 1 )
		__ui->lineEdit->setText(files.at(0));
	else if ( files.size() > 1 )
	    __ui->lineEdit->setText(files.at(0) + "+");
}


void InventorySubPanel::addNetwork() {
	ModalDialog m(ModalDialog::Network, this);
	connect(&m, SIGNAL(newNetwork(const QString&)), this, SLOT(addNetwork(const QString&)));
	m.exec();
}


void InventorySubPanel::addStation() {
	ModalDialog m(ModalDialog::Station, this);
	connect(&m, SIGNAL(newStation(const QString&, const QString&, const QString&, const QString&)),
	    this, SLOT(addStation(const QString&, const QString&, const QString&, const QString&)));
	m.exec();
}


void InventorySubPanel::removeEntity() {

	QTreeWidgetItem* itm = __ui->treeWidgetInventory->currentItem();
	if ( itm ) {

		Entity* network = getNetwork(itm->text(getHeaderPosition(thNAME)));
		if ( network ) {

			if ( QMessageBox::question(this, tr("Remove network"),
			    QString("All stations of %1's network will be removed consequently.\n"
				    "Do you really want to remove network %1 ?")
			        .arg(itm->text(getHeaderPosition(thNAME))),
			    QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes ) {
				removeNetwork(itm->text(getHeaderPosition(thNAME)));
				delete itm;
			}
		}
		else {
			Entity* station = getStation(itm->parent()->text(getHeaderPosition(thNAME)),
			    itm->text(getHeaderPosition(thSTATIONCODE)));
			if ( station ) {
				if ( QMessageBox::question(this, tr("Remove station"),
				    QString("Do you really want to remove station %1 ?")
				        .arg(itm->text(getHeaderPosition(thNAME))),
				    QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes ) {
					removeStation(itm->parent()->text(getHeaderPosition(thNAME)), itm->text(getHeaderPosition(thSTATIONCODE)));
					delete itm;
				}
			}
		}
	}
}


void InventorySubPanel::editEntity() {

	ModalDialog m((void*) __selectedEntity, this);
	connect(&m, SIGNAL(editStation(const QString&, const QString&,
	    const QString&, const QString&, const QString&)),
	    this, SLOT(editStation(const QString&, const QString&,
	        const QString&, const QString&, const QString&)));
	m.exec();
}


TimeWindowSubPanel::TimeWindowSubPanel(QWidget* parent) :
		QWidget(parent), __ui(new Ui::TimeWindow) {

	__ui->setupUi(this);

	__ui->radioButtonHalfDay->setChecked(true);
//	__ui->radioButtonCustomSL->setChecked(true);

	__ui->dateTimeEditEnd->setEnabled(false);

	__ui->frameCustomSample->setEnabled(false);

	//! These will enable the custom buttons when they're to be used
	connect(__ui->radioButtonCustomTW, SIGNAL(clicked(bool)), __ui->dateTimeEditEnd, SLOT(setEnabled(bool)));
	connect(__ui->radioButtonCustomSL, SIGNAL(clicked(bool)), __ui->frameCustomSample, SLOT(setEnabled(bool)));

	//! These will disable the custom buttons when they're not to be used
	connect(__ui->radioButtonOneDay, SIGNAL(clicked()), this, SLOT(setCustomBoxesVisible()));
	connect(__ui->radioButtonTwoDays, SIGNAL(clicked()), this, SLOT(setCustomBoxesVisible()));
	connect(__ui->radioButtonHalfDay, SIGNAL(clicked()), this, SLOT(setCustomBoxesVisible()));
	connect(__ui->radioButtonNone, SIGNAL(clicked()), this, SLOT(setCustomBoxesVisible()));
	connect(__ui->radioButtonHalfHour, SIGNAL(clicked()), this, SLOT(setCustomBoxesVisible()));
	connect(__ui->radioButtonOneHour, SIGNAL(clicked()), this, SLOT(setCustomBoxesVisible()));
	connect(__ui->comboBoxPreset, SIGNAL(currentIndexChanged(int)), this, SLOT(loadPreset(int)));

	//! User should have initialized DataSourceSubPanel before this one
	SDPASSERT(DataSourceSubPanel::instancePtr());
	connect(DataSourceSubPanel::instancePtr(), SIGNAL(setTimeWindowAccessible(bool)), this, SLOT(setTimeWindowSelectable(bool)));

	loadConfiguration();

	SDPASSERT(ParameterManager::instancePtr());
	ParameterManager* pm = ParameterManager::instancePtr();

	//! These will hold actual data
	pm->registerParameter("streamStartTime");
	pm->registerParameter("streamEndTime");
	pm->registerParameter("streamSampleDuration");
	pm->registerParameter("streamThreadNumber");

	pm->registerObjectParameter("TimeWindowSubPanel", "streamStartTime");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamEndTime");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamSampleDuration");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamThreadNumber");

	//! These will hold buttons state
	pm->registerParameter("streamEndHalfDay");
	pm->registerParameter("streamEndOneDay");
	pm->registerParameter("streamEndTwoDays");
	pm->registerParameter("streamEndCustom");
	pm->registerParameter("streamSampleNone");
	pm->registerParameter("streamSampleHalfHour");
	pm->registerParameter("streamSampleOneHour");
	pm->registerParameter("streamSampleCustom");
	pm->registerParameter("streamThreadOne");
	pm->registerParameter("streamThreadTwo");
	pm->registerParameter("streamThreadFour");

	pm->registerObjectParameter("TimeWindowSubPanel", "streamEndHalfDay");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamEndOneDay");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamEndTwoDays");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamEndCustom");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamSampleNone");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamSampleHalfHour");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamSampleOneHour");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamSampleCustom");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamThreadOne");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamThreadTwo");
	pm->registerObjectParameter("TimeWindowSubPanel", "streamThreadFour");
}


TimeWindowSubPanel::~TimeWindowSubPanel() {}


bool TimeWindowSubPanel::saveParameters() {

	SDPASSERT(ParameterManager::instancePtr());

	ParameterManager* m = ParameterManager::instancePtr();

	m->setParameter("streamStartTime", QVariant::fromValue(__ui->dateTimeEditStart->dateTime()));
	m->setParameter("streamEndHalfDay", QVariant::fromValue(__ui->radioButtonHalfDay->isChecked()));
	m->setParameter("streamEndOneDay", QVariant::fromValue(__ui->radioButtonOneDay->isChecked()));
	m->setParameter("streamEndTwoDays", QVariant::fromValue(__ui->radioButtonTwoDays->isChecked()));
	m->setParameter("streamEndCustom", QVariant::fromValue(__ui->radioButtonCustomTW->isChecked()));
	m->setParameter("streamSampleNone", QVariant::fromValue(__ui->radioButtonNone->isChecked()));
	m->setParameter("streamSampleHalfHour", QVariant::fromValue(__ui->radioButtonHalfHour->isChecked()));
	m->setParameter("streamSampleCustom", QVariant::fromValue(__ui->radioButtonCustomSL->isChecked()));
	m->setParameter("streamSampleOneHour", QVariant::fromValue(__ui->radioButtonOneHour->isChecked()));

	m->setObjectParameter("TimeWindowSubPanel", "streamStartTime", QVariant::fromValue(__ui->dateTimeEditStart->dateTime()));
	m->setObjectParameter("TimeWindowSubPanel", "streamEndHalfDay", QVariant::fromValue(__ui->radioButtonHalfDay->isChecked()));
	m->setObjectParameter("TimeWindowSubPanel", "streamEndOneDay", QVariant::fromValue(__ui->radioButtonOneDay->isChecked()));
	m->setObjectParameter("TimeWindowSubPanel", "streamEndTwoDays", QVariant::fromValue(__ui->radioButtonTwoDays->isChecked()));
	m->setObjectParameter("TimeWindowSubPanel", "streamEndCustom", QVariant::fromValue(__ui->radioButtonCustomTW->isChecked()));
	m->setObjectParameter("TimeWindowSubPanel", "streamSampleNone", QVariant::fromValue(__ui->radioButtonNone->isChecked()));
	m->setObjectParameter("TimeWindowSubPanel", "streamSampleHalfHour", QVariant::fromValue(__ui->radioButtonHalfHour->isChecked()));
	m->setObjectParameter("TimeWindowSubPanel", "streamSampleCustom", QVariant::fromValue(__ui->radioButtonCustomSL->isChecked()));
	m->setObjectParameter("TimeWindowSubPanel", "streamSampleOneHour", QVariant::fromValue(__ui->radioButtonOneHour->isChecked()));

	if ( __ui->radioButtonHalfDay->isChecked() ) {
		m->setParameter("streamEndTime", QVariant::fromValue(__ui->dateTimeEditStart->dateTime().addSecs(12 * 3600)));
		m->setObjectParameter("TimeWindowSubPanel", "streamEndTime", QVariant::fromValue(__ui->dateTimeEditStart->dateTime().addSecs(12 * 3600)));
	}
	else if ( __ui->radioButtonOneDay->isChecked() ) {
		m->setParameter("streamEndTime", QVariant::fromValue(__ui->dateTimeEditStart->dateTime().addDays(1)));
		m->setObjectParameter("TimeWindowSubPanel", "streamEndTime", QVariant::fromValue(__ui->dateTimeEditStart->dateTime().addDays(1)));
	}
	else if ( __ui->radioButtonTwoDays->isChecked() ) {
		m->setParameter("streamEndTime", QVariant::fromValue(__ui->dateTimeEditStart->dateTime().addDays(2)));
		m->setObjectParameter("TimeWindowSubPanel", "streamEndTime", QVariant::fromValue(__ui->dateTimeEditStart->dateTime().addDays(2)));
	}
	else if ( __ui->radioButtonCustomTW->isChecked() ) {
		m->setParameter("streamEndTime", QVariant::fromValue(__ui->dateTimeEditEnd->dateTime()));
		m->setObjectParameter("TimeWindowSubPanel", "streamEndTime", QVariant::fromValue(__ui->dateTimeEditEnd->dateTime()));
	}

	if ( __ui->radioButtonNone->isChecked() ) {
		m->setParameter("streamSampleDuration", QVariant::fromValue(0));
		m->setObjectParameter("TimeWindowSubPanel", "streamSampleDuration", QVariant::fromValue(0));
	}
	else if ( __ui->radioButtonHalfHour->isChecked() ) {
		m->setParameter("streamSampleDuration", QVariant::fromValue(1200));
		m->setObjectParameter("TimeWindowSubPanel", "streamSampleDuration", QVariant::fromValue(1200));
	}
	else if ( __ui->radioButtonOneHour->isChecked() ) {
		m->setParameter("streamSampleDuration", QVariant::fromValue(3600));
		m->setObjectParameter("TimeWindowSubPanel", "streamSampleDuration", QVariant::fromValue(3600));
	}
	else if ( __ui->radioButtonCustomSL->isChecked() ) {
		if ( __ui->comboBoxUnit->currentText().contains("sec") ) {
			m->setParameter("streamSampleDuration", QVariant::fromValue(__ui->lineEditSL->text().toInt()));
			m->setObjectParameter("TimeWindowSubPanel", "streamSampleDuration", QVariant::fromValue(__ui->lineEditSL->text().toInt()));
		}
		else if ( __ui->comboBoxUnit->currentText().contains("min") ) {
			m->setParameter("streamSampleDuration", QVariant::fromValue(__ui->lineEditSL->text().toInt() * 60));
			m->setObjectParameter("TimeWindowSubPanel", "streamSampleDuration", QVariant::fromValue(__ui->lineEditSL->text().toInt() * 60));
		}
		else if ( __ui->comboBoxUnit->currentText().contains("hour") ) {
			m->setParameter("streamSampleDuration", QVariant::fromValue(__ui->lineEditSL->text().toInt() * 3600));
			m->setObjectParameter("TimeWindowSubPanel", "streamSampleDuration", QVariant::fromValue(__ui->lineEditSL->text().toInt() * 3600));
		}
	}

	return true;
}


bool TimeWindowSubPanel::loadParameters() {

	SDPASSERT(ParameterManager::instancePtr());

	ParameterManager* m = ParameterManager::instancePtr();
	__ui->dateTimeEditStart->setDateTime(m->objectParameter("TimeWindowSubPanel", "streamStartTime").toDateTime());
	__ui->dateTimeEditEnd->setDateTime(m->objectParameter("TimeWindowSubPanel", "streamEndTime").toDateTime());
	__ui->radioButtonHalfDay->setChecked(m->objectParameter("TimeWindowSubPanel", "streamEndHalfDay").toBool());
	__ui->radioButtonOneDay->setChecked(m->objectParameter("TimeWindowSubPanel", "streamEndOneDay").toBool());
	__ui->radioButtonTwoDays->setChecked(m->objectParameter("TimeWindowSubPanel", "streamEndTwoDays").toBool());
	__ui->radioButtonCustomTW->setChecked(m->objectParameter("TimeWindowSubPanel", "streamEndCustom").toBool());
	__ui->radioButtonNone->setChecked(m->objectParameter("TimeWindowSubPanel", "streamSampleNone").toBool());
	__ui->radioButtonHalfHour->setChecked(m->objectParameter("TimeWindowSubPanel", "streamSampleHalfHour").toBool());
	__ui->radioButtonCustomSL->setChecked(m->objectParameter("TimeWindowSubPanel", "streamSampleCustom").toBool());
	__ui->radioButtonOneHour->setChecked(m->objectParameter("TimeWindowSubPanel", "streamSampleOneHour").toBool());
	__ui->lineEditSL->setText(m->objectParameter("TimeWindowSubPanel", "streamSampleDuration").toString());

	return true;
}


bool TimeWindowSubPanel::loadConfiguration() {

	SDPASSERT(MainFrame::instancePtr());
	SDPASSERT(Logger::instancePtr());

	__presets.clear();
	Config* cfg = MainFrame::instancePtr()->config();

	QStringList presets;
	try {
		presets = cfg->getStrings("timeWindow.presets");
	}
	catch ( ... ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Failed to read timeWindow.presets from configuration file");
	}
	if ( presets.count() != 0 ) {
		for (int i = 0; i < presets.size(); ++i) {

			Preset p;
			p.idx = i + 1;
			p.name = presets.at(i).trimmed();
			try {
				p.sampleLength = cfg->getInt("timeWindow.preset." + p.name + ".sampleLength");
			} catch ( ... ) {}
			try {
				p.start = QDateTime::fromString(cfg->getString("timeWindow.preset." + p.name + ".start"), Qt::ISODate);
			} catch ( ... ) {}
			try {
				p.end = QDateTime::fromString(cfg->getString("timeWindow.preset." + p.name + ".end"), Qt::ISODate);
			} catch ( ... ) {}
			try {
				p.sampleLengthUnit = cfg->getString("timeWindow.preset." + p.name + ".sampleLengthUnit");
			} catch ( ... ) {}

			__presets << p;
		}
	}

	__ui->comboBoxPreset->clear();
	__ui->comboBoxPreset->insertItem(0, "-");
	for (int i = 0; i < __presets.size(); ++i)
		__ui->comboBoxPreset->insertItem(i + 1, __presets.at(i).name);

	return true;
}


bool TimeWindowSubPanel::timeWindowIsOkay() {

#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
	//! [2] The time window
	if ( __ui->radioButtonCustomTW->isChecked()
	    && __ui->dateTimeEditEnd->dateTime().toMSecsSinceEpoch()
	        < __ui->dateTimeEditStart->dateTime().toMSecsSinceEpoch() ) {
		QMessageBox::critical(this, tr("Time window error"),
		    tr("The selected end time should be greater than the start time."));
		__ui->dateTimeEditEnd->setFocus();
		return false;
	}
#else
	//! [2] The time window
	if ( __ui->radioButtonCustomTW->isChecked()
			&& __ui->dateTimeEditEnd->dateTime().toTime_t()
			< __ui->dateTimeEditStart->dateTime().toTime_t() ) {
		QMessageBox::critical(this, tr("Time window error"),
				tr("The selected end time should be greater than the start time."));
		__ui->dateTimeEditEnd->setFocus();
		return false;
	}
#endif

	return true;
}


bool TimeWindowSubPanel::sampleLengthIsOkay() {

	if ( __ui->radioButtonCustomSL->isChecked()
	    && __ui->lineEditSL->text().isEmpty() ) {
		QMessageBox::critical(this, tr("Time window error"),
		    tr("You have checked the custom sample length box, but yet you "
			    "haven't typed in any number!"));
		__ui->lineEditSL->setFocus();
		return false;
	}

	return true;
}


void TimeWindowSubPanel::setTimeWindowSelectable(bool accessible) {
	__ui->widgetTW->setEnabled(!accessible);
}


void TimeWindowSubPanel::loadPreset(int idx) {

	for (int i = 0; i < __presets.size(); ++i) {
		if ( __presets.at(i).idx == idx ) {

			__ui->radioButtonCustomSL->setChecked(true);
			__ui->radioButtonCustomTW->setChecked(true);
			__ui->dateTimeEditEnd->setEnabled(true);
			__ui->frameCustomSample->setEnabled(true);

			__ui->dateTimeEditStart->setDateTime(__presets.at(i).start);
			__ui->dateTimeEditEnd->setDateTime(__presets.at(i).end);
			__ui->lineEditSL->setText(QString::number(__presets.at(i).sampleLength));

			if ( __presets.at(i).sampleLengthUnit.contains("sec") )
				__ui->comboBoxUnit->setCurrentIndex(0);
			else if ( __presets.at(i).sampleLengthUnit.contains("min") )
				__ui->comboBoxUnit->setCurrentIndex(1);
			else if ( __presets.at(i).sampleLengthUnit.contains("hour") )
			    __ui->comboBoxUnit->setCurrentIndex(2);
		}
	}
}


void TimeWindowSubPanel::setCustomBoxesVisible() {

	QObject* sender = QObject::sender();
	SDPASSERT(sender);

	if ( sender == __ui->radioButtonHalfDay || sender == __ui->radioButtonOneDay
	    || sender == __ui->radioButtonTwoDays )
	    __ui->dateTimeEditEnd->setEnabled(false);

	if ( sender == __ui->radioButtonNone || sender == __ui->radioButtonHalfHour
	    || sender == __ui->radioButtonOneHour )
	    __ui->frameCustomSample->setEnabled(false);
}


TriggerSubPanel::TriggerSubPanel(QWidget* parent) :
		QWidget(parent), __ui(new Ui::Trigger) {

	__ui->setupUi(this);

	SDPASSERT(ParameterManager::instancePtr());
	ParameterManager* pm = ParameterManager::instancePtr();

	pm->registerParameter("CarlSTATrig");
	pm->registerParameter("ClassicSTALTA");
	pm->registerParameter("DelayedSTALTA");
	pm->registerParameter("RecursiveSTALTA");
	pm->registerParameter("CarlSTATrigQuiet");
	pm->registerParameter("CarlSTATrigRatio");
	pm->registerParameter("LTA");
	pm->registerParameter("STA");
	pm->registerParameter("TreshOFF");
	pm->registerParameter("TreshON");
	pm->registerParameter("SUM");

	pm->registerObjectParameter("TriggerSubPanel", "CarlSTATrig");
	pm->registerObjectParameter("TriggerSubPanel", "ClassicSTALTA");
	pm->registerObjectParameter("TriggerSubPanel", "DelayedSTALTA");
	pm->registerObjectParameter("TriggerSubPanel", "RecursiveSTALTA");
	pm->registerObjectParameter("TriggerSubPanel", "CarlSTATrigQuiet");
	pm->registerObjectParameter("TriggerSubPanel", "CarlSTATrigRatio");
	pm->registerObjectParameter("TriggerSubPanel", "LTA");
	pm->registerObjectParameter("TriggerSubPanel", "STA");
	pm->registerObjectParameter("TriggerSubPanel", "TreshOFF");
	pm->registerObjectParameter("TriggerSubPanel", "TreshON");
	pm->registerObjectParameter("TriggerSubPanel", "SUM");

	__ui->widgetCST->hide();
	connect(__ui->radioButtonCarlSTATrig, SIGNAL(toggled(bool)), __ui->widgetCST, SLOT(setVisible(bool)));
	connect(__ui->comboBoxPreset, SIGNAL(currentIndexChanged(int)), this, SLOT(loadPreset(int)));

	loadConfiguration();
}


TriggerSubPanel::~TriggerSubPanel() {}


bool TriggerSubPanel::saveParameters() {

	SDPASSERT(ParameterManager::instancePtr());

	ParameterManager* w = ParameterManager::instancePtr();

	w->setParameter("CarlSTATrig", QVariant::fromValue(__ui->radioButtonCarlSTATrig->isChecked()));
	w->setParameter("ClassicSTALTA", QVariant::fromValue(__ui->radioButtonClassicSTALTA->isChecked()));
	w->setParameter("DelayedSTALTA", QVariant::fromValue(__ui->radioButtonDelayedSTALTA->isChecked()));
	w->setParameter("RecursiveSTALTA", QVariant::fromValue(__ui->radioButtonRecursiveSTALTA->isChecked()));
	w->setParameter("CarlSTATrigQuiet", QVariant::fromValue(__ui->doubleSpinBoxCarlSTATrigQuiet->value()));
	w->setParameter("CarlSTATrigRatio", QVariant::fromValue(__ui->doubleSpinBoxCarlSTATrigRatio->value()));
	w->setParameter("LTA", QVariant::fromValue(__ui->doubleSpinBoxLTA->value()));
	w->setParameter("STA", QVariant::fromValue(__ui->doubleSpinBoxSTA->value()));
	w->setParameter("TreshOFF", QVariant::fromValue(__ui->doubleSpinBoxTreshOFF->value()));
	w->setParameter("TreshON", QVariant::fromValue(__ui->doubleSpinBoxTreshON->value()));
	w->setParameter("SUM", QVariant::fromValue(__ui->spinBoxTreshSUM->value()));

	w->setObjectParameter("TriggerSubPanel", "CarlSTATrig", QVariant::fromValue(__ui->radioButtonCarlSTATrig->isChecked()));
	w->setObjectParameter("TriggerSubPanel", "ClassicSTALTA", QVariant::fromValue(__ui->radioButtonClassicSTALTA->isChecked()));
	w->setObjectParameter("TriggerSubPanel", "DelayedSTALTA", QVariant::fromValue(__ui->radioButtonDelayedSTALTA->isChecked()));
	w->setObjectParameter("TriggerSubPanel", "RecursiveSTALTA", QVariant::fromValue(__ui->radioButtonRecursiveSTALTA->isChecked()));
	w->setObjectParameter("TriggerSubPanel", "CarlSTATrigQuiet", QVariant::fromValue(__ui->doubleSpinBoxCarlSTATrigQuiet->value()));
	w->setObjectParameter("TriggerSubPanel", "CarlSTATrigRatio", QVariant::fromValue(__ui->doubleSpinBoxCarlSTATrigRatio->value()));
	w->setObjectParameter("TriggerSubPanel", "LTA", QVariant::fromValue(__ui->doubleSpinBoxLTA->value()));
	w->setObjectParameter("TriggerSubPanel", "STA", QVariant::fromValue(__ui->doubleSpinBoxSTA->value()));
	w->setObjectParameter("TriggerSubPanel", "TreshOFF", QVariant::fromValue(__ui->doubleSpinBoxTreshOFF->value()));
	w->setObjectParameter("TriggerSubPanel", "TreshON", QVariant::fromValue(__ui->doubleSpinBoxTreshON->value()));
	w->setObjectParameter("TriggerSubPanel", "SUM", QVariant::fromValue(__ui->spinBoxTreshSUM->value()));

	return true;
}


bool TriggerSubPanel::loadParameters() {

	SDPASSERT(ParameterManager::instancePtr());

	ParameterManager* w = ParameterManager::instancePtr();

	__ui->radioButtonCarlSTATrig->setChecked(w->objectParameter("TriggerSubPanel", "CarlSTATrig").toBool());
	__ui->radioButtonClassicSTALTA->setChecked(w->objectParameter("TriggerSubPanel", "ClassicSTALTA").toBool());
	__ui->radioButtonDelayedSTALTA->setChecked(w->objectParameter("TriggerSubPanel", "DelayedSTALTA").toBool());
	__ui->radioButtonRecursiveSTALTA->setChecked(w->objectParameter("TriggerSubPanel", "RecursiveSTALTA").toBool());
	__ui->doubleSpinBoxCarlSTATrigQuiet->setValue(w->objectParameter("TriggerSubPanel", "CarlSTATrigQuiet").toDouble());
	__ui->doubleSpinBoxCarlSTATrigRatio->setValue(w->objectParameter("TriggerSubPanel", "CarlSTATrigRatio").toDouble());
	__ui->doubleSpinBoxLTA->setValue(w->objectParameter("TriggerSubPanel", "LTA").toDouble());
	__ui->doubleSpinBoxSTA->setValue(w->objectParameter("TriggerSubPanel", "STA").toDouble());
	__ui->doubleSpinBoxTreshOFF->setValue(w->objectParameter("TriggerSubPanel", "TreshOFF").toDouble());
	__ui->doubleSpinBoxTreshON->setValue(w->objectParameter("TriggerSubPanel", "TreshON").toDouble());
	__ui->spinBoxTreshSUM->setValue(w->objectParameter("TriggerSubPanel", "SUM").toInt());

	return true;
}


bool TriggerSubPanel::loadConfiguration() {

	SDPASSERT(MainFrame::instancePtr());
	SDPASSERT(Logger::instancePtr());

	__presets.clear();
	Config* cfg = MainFrame::instancePtr()->config();

	QStringList presets;
	try {
		presets = cfg->getStrings("trigger.presets");
	}
	catch ( ... ) {
		Logger::instancePtr()->addMessage(Logger::WARNING, __func__,
		    "Failed to read trigger.presets from configuration file");
	}
	if ( presets.count() != 0 ) {
		for (int i = 0; i < presets.size(); ++i) {

			Preset p;
			p.idx = i + 1;
			p.name = presets.at(i).trimmed();

			try {
				p.trigType = cfg->getString("trigger.preset." + p.name + ".Trigger");
			} catch ( ... ) {}
			try {
				p.treshOn = cfg->getDouble("trigger.preset." + p.name + ".TreshON");
			} catch ( ... ) {}
			try {
				p.treshOff = cfg->getDouble("trigger.preset." + p.name + ".TreshOFF");
			} catch ( ... ) {}
			try {
				p.lta = cfg->getDouble("trigger.preset." + p.name + ".LTA");
			} catch ( ... ) {}
			try {
				p.sta = cfg->getDouble("trigger.preset." + p.name + ".STA");
			} catch ( ... ) {}
			try {
				p.ratio = cfg->getDouble("trigger.preset." + p.name + ".Ratio");
			} catch ( ... ) {}
			try {
				p.quiet = cfg->getDouble("trigger.preset." + p.name + ".Quiet");
			} catch ( ... ) {}

			__presets << p;
		}
	}

	__ui->comboBoxPreset->clear();
	__ui->comboBoxPreset->insertItem(0, "-");
	for (int i = 0; i < __presets.size(); ++i)
		__ui->comboBoxPreset->insertItem(i + 1, __presets.at(i).name);

	return true;
}


void TriggerSubPanel::loadPreset(int idx) {

	for (int i = 0; i < __presets.size(); ++i) {
		if ( __presets.at(i).idx == idx ) {

			if ( __presets.at(i).trigType == "classic" ) {
				__ui->radioButtonClassicSTALTA->setChecked(true);
				__ui->widgetCST->setVisible(false);
			}
			else if ( __presets.at(i).trigType.contains("recurcive") ) {
				__ui->radioButtonRecursiveSTALTA->setChecked(true);
				__ui->widgetCST->setVisible(false);
			}
			else if ( __presets.at(i).trigType.contains("delayed") ) {
				__ui->radioButtonDelayedSTALTA->setChecked(true);
				__ui->widgetCST->setVisible(false);
			}
			else if ( __presets.at(i).trigType.contains("carlstatrig") ) {
				__ui->radioButtonRecursiveSTALTA->setChecked(true);
				__ui->widgetCST->setVisible(true);
			}

			__ui->doubleSpinBoxCarlSTATrigQuiet->setValue(__presets.at(i).quiet);
			__ui->doubleSpinBoxCarlSTATrigRatio->setValue(__presets.at(i).ratio);
			__ui->doubleSpinBoxLTA->setValue(__presets.at(i).lta);
			__ui->doubleSpinBoxSTA->setValue(__presets.at(i).sta);
			__ui->doubleSpinBoxTreshON->setValue(__presets.at(i).treshOn);
			__ui->doubleSpinBoxTreshOFF->setValue(__presets.at(i).treshOff);
			__ui->spinBoxTreshSUM->setValue(__presets.at(i).sum);
		}
	}
}

} // namespace Qt4
} // namespace SDP

