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
#include <sdp/gui/datamodel/databasemanager.h>
#include <sdp/gui/datamodel/panels.h>
#include <sdp/gui/datamodel/job.h>
#include <sdp/gui/datamodel/logger.h>
#include <sdp/gui/datamodel/macros.h>
#include <sdp/gui/datamodel/trigger.h>
#include <sdp/gui/datamodel/archiveobjects.h>
#include <QVariant>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDataStream>
#include <QByteArray>
#include <QDebug>


namespace {

static QString const detectionTable = "CREATE TABLE \"Detection\" (\"id\" "
	"VARCHAR PRIMARY KEY  NOT NULL  UNIQUE , \"return_code\" INTEGER NOT NULL , "
	"\"data\" BLOB NOT NULL )";
static QString const dispatchTable = "CREATE TABLE \"Dispatch\" (\"id\" "
	"VARCHAR PRIMARY KEY  NOT NULL , \"data\" BLOB NOT NULL )";
static QString const triggerTable = "CREATE TABLE \"Trigger\" (\"id\" VARCHAR "
	"PRIMARY KEY  NOT NULL , \"status\" INTEGER NOT NULL , \"data\" BLOB NOT NULL , "
	"\"detection_id\" VARCHAR NOT NULL )";

bool recordExists(QSqlDatabase& db, const QString& tableName,
                  const QString& objectID) {

	if ( !db.isOpen() )
	    return false;

	QSqlQuery query(QString("SELECT * FROM %1 WHERE id LIKE '%2'").arg(tableName).arg(objectID));
	return query.next();
}

bool removeRecord(QSqlDatabase& db, const QString& tableName,
                  const QString& objectID) {

	if ( !db.isOpen() )
	    return false;

	QSqlQuery query(QString("DELETE FROM %1 WHERE id LIKE '%2'").arg(tableName).arg(objectID));
	return query.exec();
}

}


namespace SDP {
namespace Qt4 {

DatabaseManager::DatabaseManager(const QString& filename) :
		__dbFile(filename) {

	SDPASSERT(Logger::instancePtr());
	Logger* log = Logger::instancePtr();

	log->addMessage(Logger::DEBUG, __func__, QString("Initiating application database from file %1...").arg(filename));

	if ( openDatabase(filename) ) {
		log->addMessage(Logger::INFO, __func__, "Database found and opened.");
		if ( __db.tables().isEmpty() ) {
			log->addMessage(Logger::INFO, __func__, "Database appears to be empty, setting up entities.");
			initDatabase();
		}
	}
	else {
		log->addMessage(Logger::CRITICAL, __func__, QString("Failed to open database (%1).").arg(filename));
		log->addMessage(Logger::WARNING, __func__, "Initiating new database for application.");
		initDatabase();
	}
}


DatabaseManager::~DatabaseManager() {
	if ( __db.isOpen() )
	    __db.close();
}


bool DatabaseManager::initDatabase() {

	SDPASSERT(Logger::instancePtr());
	Logger* log = Logger::instancePtr();

	if ( __db.isOpen() ) return false;

	QSqlQuery query1(::detectionTable);
	bool retcode = query1.exec();
	if ( retcode )
		log->addMessage(Logger::INFO, __func__, "Successfully created Detection table");
	else
		log->addMessage(Logger::CRITICAL, __func__, "Failed to create Detection table");

	QSqlQuery query2(::dispatchTable);
	retcode = query2.exec();
	if ( retcode )
		log->addMessage(Logger::INFO, __func__, "Successfully created Dispatch table");
	else
		log->addMessage(Logger::CRITICAL, __func__, "Failed to create Dispatch table");

	QSqlQuery query3(::triggerTable);
	retcode = query3.exec();
	if ( retcode )
		log->addMessage(Logger::INFO, __func__, "Successfully created Trigger table");
	else
		log->addMessage(Logger::CRITICAL, __func__, "Failed to create Trigger table");

	return retcode;
}


bool DatabaseManager::openDatabase(const QString& file) {

	__db = QSqlDatabase::addDatabase("QSQLITE");
	__db.setDatabaseName(file);

	return __db.open();
}


bool DatabaseManager::deleteDatabase(const QString& path) {

	__db.close();

	QString fpath;

	// NOTE: We have to store database file into user home folder in Linux
	//       therefore we add the separators properly...
	if ( path.isEmpty() )
		fpath = QString("%1%2sdp.sqlite").arg(PROJECT_SHARE_DIR);
	else
		fpath = path;

	fpath.append(QDir::separator()).append(__dbFile);
	fpath = QDir::toNativeSeparators(fpath);

	return QFile::remove(fpath);
}


bool DatabaseManager::isOpened() const {
	return __db.isOpen();
}


bool DatabaseManager::detectionExists(const QString& id) {
	return ::recordExists(__db, "main.Detection", id);
}


bool DatabaseManager::dispatchExists(const QString& id) {
	return ::recordExists(__db, "main.Dispatch", id);
}


bool DatabaseManager::triggerExists(const QString& id) {
	return ::recordExists(__db, "main.Trigger", id);
}


int DatabaseManager::commitDetection(DetectionJob* job) {

	SDPASSERT(Logger::instancePtr());
	int newID = -1;

	if ( !__db.isOpen() ) {
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__,
		    QString("Failed to store detection job %1. Database couldn't be opened.")
		        .arg(job->id()));
		return newID;
	}

	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
	stream.setVersion(QDataStream::Qt_4_8);
#else
	stream.setVersion(QDataStream::Qt_4_6);
#endif
	job->toDataStream(stream);

	QSqlQuery query;

	if ( detectionExists(job->id()) ) {
		query.prepare("UPDATE main.Detection SET return_code=:jretcode, "
			"data=:jdata WHERE id = :jid");
		query.bindValue("jretcode", job->runExitCode());
		query.bindValue("jdata", qCompress(buffer));
		query.bindValue("jid", job->id());
	}
	else {
		query.prepare("INSERT INTO main.Detection (id, return_code, data) "
			"VALUES (:jid, :jretcode, :jdata)");
		query.bindValue("jid", job->id());
		query.bindValue("jretcode", job->runExitCode());
		query.bindValue("jdata", qCompress(buffer));
	}

	if ( query.exec() )
		newID = query.lastInsertId().toInt();
	else
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__,
		    QString("Query failed: %1").arg(query.executedQuery()));

	return newID;
}


int DatabaseManager::commitDispatch(DispatchJob* job) {

	SDPASSERT(Logger::instancePtr());
	int newID = -1;

	if ( !__db.isOpen() ) {
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__,
		    QString("Failed to store dispatch job %1. Database couldn't be opened.")
		        .arg(job->id()), true);
		return newID;
	}

	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
	stream.setVersion(QDataStream::Qt_4_8);
#else
	stream.setVersion(QDataStream::Qt_4_6);
#endif
	job->toDataStream(stream);

	QSqlQuery query;

	if ( dispatchExists(job->id()) ) {
		query.prepare("UPDATE main.Dispatch SET data=:jdata WHERE id = :jid");
		query.bindValue("jdata", qCompress(buffer));
		query.bindValue("jid", job->id());
	}
	else {
		query.prepare("INSERT INTO main.Dispatch (id, data) VALUES (:jid, :jdata)");
		query.bindValue("jid", job->id());
		query.bindValue("jdata", qCompress(buffer));
	}

	if ( query.exec() )
		newID = query.lastInsertId().toInt();
	else
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__,
		    QString("Query failed: %1").arg(query.executedQuery()));

	return newID;
}


int DatabaseManager::commitTrigger(Trigger* trig) {

	SDPASSERT(Logger::instancePtr());
	int newID = -1;

	if ( !__db.isOpen() ) {
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__,
		    QString("Failed to store trigger %1. Database couldn't be opened.")
		        .arg(trig->id()));
		return newID;
	}

	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
	stream.setVersion(QDataStream::Qt_4_8);
#else
	stream.setVersion(QDataStream::Qt_4_6);
#endif
	trig->toDataStream(stream);

	QSqlQuery query;

	if ( triggerExists(trig->id()) ) {
		query.prepare("UPDATE main.Trigger SET status=:tstatus, data=:tdata WHERE id = :tid");
		query.bindValue("tstatus", static_cast<int>(trig->status()));
		query.bindValue("tdata", qCompress(buffer));
		query.bindValue("tid", trig->id());
	}
	else {
		query.prepare("INSERT INTO main.Trigger (id, status, data, detection_id) "
			"VALUES (:tid, :tstatus, :tdata, :jid)");
		query.bindValue("tid", trig->id());
		query.bindValue("tstatus", static_cast<int>(trig->status()));
		query.bindValue("tdata", qCompress(buffer));
		query.bindValue("jid", trig->jobID());
	}

	if ( query.exec() )
		newID = query.lastInsertId().toInt();
	else
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__,
		    QString("Query failed: %1").arg(query.executedQuery()));

	return newID;
}


bool DatabaseManager::removeDetection(DetectionJob* job, const bool& cascade) {

	SDPASSERT(Logger::instancePtr());
	Logger* log = Logger::instancePtr();

	if ( !detectionExists(job->id()) ) {
		log->addMessage(Logger::WARNING, __func__,
		    QString("Failed to remove detection job %1 from database. Item not found.").arg(job->id()));
		return false;
	}

	if ( cascade ) {
		QSqlQuery selTrig(QString("SELECT COUNT(*) FROM main.Trigger WHERE detection_id LIKE '%1'").arg(job->id()));
		if ( selTrig.next() )
		    log->addMessage(Logger::INFO, __func__, QString("%1 triggers of detection object %2 will be removed from database.")
		        .arg(selTrig.value(0).toString()).arg(job->id()));

		QSqlQuery rmTrig(QString("DELETE FROM main.Trigger WHERE detection_id LIKE '%1'").arg(job->id()));
		if ( rmTrig.exec() )
		    log->addMessage(Logger::INFO, __func__, QString("Removed detection object's %1 triggers from database.").arg(job->id()));
	}

	Logger::instancePtr()->addMessage(Logger::INFO, __func__,
	    QString("Removing detection object %1 from database.").arg(job->id()), true);

	return ::removeRecord(__db, "main.Detection", job->id());
}


bool DatabaseManager::removeDispatch(DispatchJob* job) {

	SDPASSERT(Logger::instancePtr());
	if ( !dispatchExists(job->id()) ) return false;

	Logger::instancePtr()->addMessage(Logger::INFO, __func__,
	    QString("Removing dispatch %1 from database.").arg(job->id()), true);

	return ::removeRecord(__db, "main.Dispatch", job->id());
}


bool DatabaseManager::removeTrigger(Trigger* trig) {

	SDPASSERT(Logger::instancePtr());
	if ( !triggerExists(trig->id()) ) return false;

	Logger::instancePtr()->addMessage(Logger::INFO, __func__,
	    QString("Removing trigger %1 from database.").arg(trig->id()), true);

	return ::removeRecord(__db, "main.Trigger", trig->id());
}


DetectionJob* DatabaseManager::getDetection(const QString& id) {

	SDPASSERT(Logger::instancePtr());
	DetectionJob* job = NULL;

	if ( !__db.isOpen() ) {
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__,
		    QString("Failed to fetch job %1. Database couldn't be opened.")
		        .arg(id), true);
		return job;
	}

	QSqlQuery query(QString("SELECT data FROM main.Detection WHERE id LIKE '%1'").arg(id));

	if ( query.next() ) {
		QByteArray data = qUncompress(query.value(0).toByteArray());
		QDataStream stream(&data, QIODevice::ReadOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
		stream.setVersion(QDataStream::Qt_4_8);
#else
		stream.setVersion(QDataStream::Qt_4_6);
#endif
		job = DetectionJob::create();
		job->fromDataStream(stream);
	}

	return job;
}


DispatchJob* DatabaseManager::getDispatch(const QString& id) {

	SDPASSERT(Logger::instancePtr());
	DispatchJob* job = NULL;

	if ( !__db.isOpen() ) {
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__,
		    QString("Failed to fetch job %1. Database couldn't be opened.")
		        .arg(id), true);
		return job;
	}

	QSqlQuery query(QString("SELECT data FROM main.Dispatch WHERE id LIKE '%1'").arg(id));
	if ( query.next() ) {
		QByteArray data = qUncompress(query.value(0).toByteArray());
		QDataStream stream(&data, QIODevice::ReadOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
		stream.setVersion(QDataStream::Qt_4_8);
#else
		stream.setVersion(QDataStream::Qt_4_6);
#endif
		job = DispatchJob::create();
		job->fromDataStream(stream);
	}

	return job;
}


Trigger* DatabaseManager::getTrigger(const QString& id) {

	SDPASSERT(Logger::instancePtr());
	Trigger* trig = NULL;

	if ( __db.isOpen() ) {
		Logger::instancePtr()->addMessage(Logger::CRITICAL, __func__,
		    QString("Failed to fetch trigger %1. Database couldn't be opened.")
		        .arg(id), true);
		return trig;
	}

	QSqlQuery query(QString("SELECT data FROM main.Trigger WHERE id LIKE '%1'").arg(id));

	if ( query.next() ) {
		QByteArray data = qUncompress(query.value(0).toByteArray());
		QDataStream stream(&data, QIODevice::ReadOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
		stream.setVersion(QDataStream::Qt_4_8);
#else
		stream.setVersion(QDataStream::Qt_4_6);
#endif
		trig = Trigger::create("");
		trig->fromDataStream(stream);
	}

	return trig;
}


DatabaseManager::DetectionList DatabaseManager::detections() {

	DetectionList l;

	if ( !__db.isOpen() )
	    return l;

	QSqlQuery query(QString("SELECT data FROM main.Detection"));

	while ( query.next() ) {
		QByteArray data = qUncompress(query.value(0).toByteArray());
		QDataStream stream(&data, QIODevice::ReadOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
		stream.setVersion(QDataStream::Qt_4_8);
#else
		stream.setVersion(QDataStream::Qt_4_6);
#endif
		DetectionJob* job = DetectionJob::create();
		job->fromDataStream(stream);
		l << job;
	}

	return l;
}


DatabaseManager::DispatchList DatabaseManager::dispatchs() {

	DispatchList l;

	if ( !__db.isOpen() )
	    return l;

	QSqlQuery query(QString("SELECT data FROM main.Dispatch"));

	while ( query.next() ) {
		QByteArray data = qUncompress(query.value(0).toByteArray());
		QDataStream stream(&data, QIODevice::ReadOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
		stream.setVersion(QDataStream::Qt_4_8);
#else
		stream.setVersion(QDataStream::Qt_4_6);
#endif
		DispatchJob* job = DispatchJob::create();
		job->fromDataStream(stream);
		l << job;
	}

	return l;
}


DatabaseManager::TriggerList DatabaseManager::triggers() {

	TriggerList l;

	if ( !__db.isOpen() )
	    return l;

	QSqlQuery query(QString("SELECT data FROM main.Trigger"));

	while ( query.next() ) {
		QByteArray data = qUncompress(query.value(0).toByteArray());
		QDataStream stream(&data, QIODevice::ReadOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
		stream.setVersion(QDataStream::Qt_4_8);
#else
		stream.setVersion(QDataStream::Qt_4_6);
#endif
		Trigger* trig = Trigger::create("");
		trig->fromDataStream(stream);
		l << trig;
	}

	return l;
}


DatabaseManager::TriggerList
DatabaseManager::triggers(const QString& detectionID) {

	TriggerList l;

	if ( !__db.isOpen() )
	    return l;

	QSqlQuery query(QString("SELECT data FROM main.Trigger WHERE detection_id LIKE '%1'")
	    .arg(detectionID));

	while ( query.next() ) {
		QByteArray data = qUncompress(query.value(0).toByteArray());
		QDataStream stream(&data, QIODevice::ReadOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
		stream.setVersion(QDataStream::Qt_4_8);
#else
		stream.setVersion(QDataStream::Qt_4_6);
#endif
		Trigger* trig = Trigger::create("");
		trig->fromDataStream(stream);
		l << trig;
	}

	return l;
}


DatabaseManager::TriggerList DatabaseManager::unreviewedTriggers() {

	TriggerList l;

	if ( !__db.isOpen() )
	    return l;

	QSqlQuery query(QString("SELECT data FROM main.Trigger WHERE status=0"));

	while ( query.next() ) {
		QByteArray data = qUncompress(query.value(0).toByteArray());
		QDataStream stream(&data, QIODevice::ReadOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
		stream.setVersion(QDataStream::Qt_4_8);
#else
		stream.setVersion(QDataStream::Qt_4_6);
#endif
		Trigger* trig = Trigger::create("");
		trig->fromDataStream(stream);
		l << trig;
	}

	return l;
}


DatabaseManager::TriggerList DatabaseManager::uncommittedTriggers() {

	TriggerList l;

	if ( !__db.isOpen() )
	    return l;

	QSqlQuery query(QString("SELECT data FROM main.Trigger WHERE status=0 OR status=1 OR status=2"));

	while ( query.next() ) {
		QByteArray data = qUncompress(query.value(0).toByteArray());
		QDataStream stream(&data, QIODevice::ReadOnly);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
		stream.setVersion(QDataStream::Qt_4_8);
#else
		stream.setVersion(QDataStream::Qt_4_6);
#endif
		Trigger* trig = Trigger::create("");
		trig->fromDataStream(stream);
		l << trig;
	}

	return l;
}

} // namespace Qt4
} // namesapce SDP
