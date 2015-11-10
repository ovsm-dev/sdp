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
#include <sdp/gui/datamodel/archiveobjects.h>
#include <sdp/gui/datamodel/job.h>
#include <sdp/gui/datamodel/logger.h>
#include <sdp/gui/datamodel/macros.h>
#include <sdp/gui/datamodel/parametermanager.h>
#include <sdp/gui/datamodel/utils.h>
#include <QDir>
#include <QDirIterator>
#include <QFile>


namespace SDP {
namespace Qt4 {


ArchivedStation::ArchivedStation(ArchivedOrigin* parent, const QString& net,
                                 const QString& code, const QString& locationCode,
                                 const QString& channelCode, const QString& pix) :
		__parent(parent), __network(net), __code(code), __locationCode(locationCode),
		__channelCode(channelCode), __pixmap(pix) {}


bool ArchivedStation::operator==(const ArchivedStation& as) {
	return (this->__network == as.__network) && (this->__code == as.__code)
	    && (this->__locationCode == as.__locationCode)
	    && (this->__channelCode == as.__channelCode)
	    && (this->__pixmap == as.__pixmap);
}


ArchivedStation::~ArchivedStation() {}


ArchivedOrigin* ArchivedStation::origin() {
	return __parent;
}


const QString& ArchivedStation::network() const {
	return __network;
}


const QString& ArchivedStation::code() const {
	return __code;
}


const QString& ArchivedStation::locationCode() const {
	return __locationCode;
}


const QString& ArchivedStation::channelCode() const {
	return __channelCode;
}


const QString& ArchivedStation::pixmap() const {
	return __pixmap;
}


ArchivedOrigin::ArchivedOrigin(ArchivedRun* parent, const QDateTime& dt,
                               const QString& wp) :
		__parent(parent), __time(dt), __webpage(wp) {}


bool ArchivedOrigin::operator==(const ArchivedOrigin& o) {
	return (this->__time == o.__time) && (this->__webpage == o.__webpage);
}


ArchivedOrigin::~ArchivedOrigin() {
	qDeleteAll(__stations);
}


ArchivedOrigin* ArchivedOrigin::load(ArchivedRun* parent, const QString& path,
                                     const QString& id) {

	SDPASSERT(ParameterManager::instancePtr());

	ParameterManager* pm = ParameterManager::instancePtr();

	QStringList stations;
	QDirIterator files(path, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);

	//! Locate files with extension '.png', there should be one by station for
	//! the origin we're creating...
	while ( files.hasNext() ) {
		files.next();
		if ( files.fileName().contains(id) && files.fileName().contains(".png") )
		    stations << files.fileName();
	}

	if ( stations.isEmpty() ) return NULL;

	ArchivedOrigin* o = new ArchivedOrigin(parent, QDateTime::fromString(id, Qt::ISODate),
	    path + QDir::separator() + pm->parameter("TRIGGER_PREFIX").toString() + id + ".htm");

	//! Read various origins times and associated stations
	for (QStringList::const_iterator it = stations.constBegin();
	        it != stations.constEnd(); ++it) {

		const QStringList l = (*it).split('-');
		if ( l.size() != 7 ) continue;

		//! Add new station to origin, note that we remove the 4 last characters
		//! because they are the extension of the file
		o->appendStation(new ArchivedStation(o, l.at(3), l.at(4), l.at(5),
		    l.at(6).left(l.at(6).size() - 4),
		    path + QDir::separator() + (*it)));
	}

	return o;
}

ArchivedRun* ArchivedOrigin::run() {
	return __parent;
}


const QDateTime& ArchivedOrigin::time() const {
	return __time;
}


const ArchivedStationList& ArchivedOrigin::stations() const {
	return __stations;
}


const QString& ArchivedOrigin::webpage() const {
	return __webpage;
}


void ArchivedOrigin::appendStation(ArchivedStation* as) {
	if ( !as ) return;
	if ( !__stations.contains(as) )
	    __stations << as;
}


ArchivedRun::ArchivedRun(const Job* job) :
		__job(job) {}


ArchivedRun::~ArchivedRun() {
	qDeleteAll(__origins);
}


ArchivedRun* ArchivedRun::loadRun(const Job* job) {

	SDPASSERT(ParameterManager::instancePtr());
	SDPASSERT(Logger::instancePtr());

	ParameterManager* pm = ParameterManager::instancePtr();
	Logger* log = Logger::instancePtr();

	if ( !Utils::dirExists(job->runDir()) ) return NULL;

	//! Try and extract the timewindow
	QStringList l = job->runDir().split('-');
	if ( l.size() != 3 ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to read timewindow from filepath " + job->runDir(), false);
		return NULL;
	}

	QString trigFile = pm->parameter("TRIGGER_FILE").toString();
	trigFile.replace("@JOB_RUN_DIR@", job->runDir());

	if ( !Utils::fileExists(trigFile) ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to read triggers files from filepath " + trigFile, false);
		return NULL;
	}

	QStringList triggers;
	QFile inputFile(trigFile);
	if ( inputFile.open(QIODevice::ReadOnly) ) {
		QTextStream in(&inputFile);
		while ( !in.atEnd() ) {
			QString line = in.readLine();
			if ( !triggers.contains(line) )
			    triggers << line;
		}
	}
	inputFile.close();

	if ( triggers.isEmpty() ) {
		log->addMessage(Logger::DEBUG, __func__, "Path: " + job->runDir() + " is empty.", false);
		return NULL;
	}

	QString sfile = job->runDir() + QDir::separator() + "detect.py";
	if ( !Utils::fileExists(sfile) ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to read detection script from filepath " + trigFile, false);
		return NULL;
	}

	QString script;
	QFile inputScript(sfile);
	if ( inputScript.open(QIODevice::ReadOnly) ) {
		QTextStream in(&inputFile);
		while ( !in.atEnd() )
			script.append(in.readLine());
	}
	inputScript.close();

	ArchivedRun* run = new ArchivedRun(job);
	run->__tw.start = QDateTime::fromString(l.at(1), "yyyyMMddHHmmss");
	run->__tw.end = QDateTime::fromString(l.at(2), "yyyyMMddHHmmss");
	run->__script = script;

	//! Read various origins times and associated stations
	for (QStringList::const_iterator it = triggers.constBegin();
	        it != triggers.constEnd(); ++it) {
		ArchivedOrigin* o = ArchivedOrigin::load(run, job->runDir(), (*it));
		run->appendOrigin(o);
	}

	return run;
}


const Job* ArchivedRun::job() {
	return __job;
}


const ArchivedOriginList& ArchivedRun::origins() const {
	return __origins;
}


const QString& ArchivedRun::script() const {
	return __script;
}


const TimeWindow& ArchivedRun::timeWindow() const {
	return __tw;
}


void ArchivedRun::appendOrigin(ArchivedOrigin* o) {
	if ( !o ) return;
	if ( !__origins.contains(o) )
	    __origins << o;
}


} // namespace Qt4
} // namespace SDP
