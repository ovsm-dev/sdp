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



#ifndef __SDP_QT4_DATABASEMANAGER_H__
#define __SDP_QT4_DATABASEMANAGER_H__


#include <sdp/gui/datamodel/singleton.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QList>


namespace SDP {
namespace Qt4 {

class Job;
class DetectionJob;
class DispatchJob;
class Trigger;
struct DispatchInformation;
struct DetectionInformation;

/**
 * @class DatabaseManager
 *
 * This class implements a database interface. At the moment, only SQLite
 * databases are supported.
 *
 * Serialized objects like Job, Trigger are stored and restored inside the
 * specified file or memory block. By default, their data is compressed for
 * a better in-entity space management.
 *
 * @note This class inherits from Singleton class, which means that only one
 * instance is allowed by application run, and also, that its public interface
 * is accessible by any object requesting it at runtime.
 */
class DatabaseManager : public Singleton<DatabaseManager> {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef QList<DetectionJob*> DetectionList;
		typedef QList<DispatchJob*> DispatchList;
		typedef QList<Trigger*> TriggerList;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit DatabaseManager(const QString&);
		~DatabaseManager();

	public:
		// ------------------------------------------------------------------
		//  Public General interface
		// ------------------------------------------------------------------
		bool initDatabase();
		bool openDatabase(const QString&);
		bool deleteDatabase(const QString&);
		bool isOpened() const;

	public:
		// ------------------------------------------------------------------
		//  Public DataModel interface
		// ------------------------------------------------------------------
		bool detectionExists(const QString& id);
		bool dispatchExists(const QString& id);
		bool triggerExists(const QString& id);

		int commitDetection(DetectionJob*);
		int commitDispatch(DispatchJob*);
		int commitTrigger(Trigger*);

		bool removeDetection(DetectionJob*, const bool& cascade = false);
		bool removeDispatch(DispatchJob*);
		bool removeTrigger(Trigger*);

		DetectionJob* getDetection(const QString& id);
		DispatchJob* getDispatch(const QString& id);
		Trigger* getTrigger(const QString& id);

		DetectionList detections();
		DispatchList dispatchs();
		TriggerList triggers();
		TriggerList triggers(const QString& detectionID);
		TriggerList unreviewedTriggers();
		TriggerList uncommittedTriggers();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QSqlDatabase __db;
		QString __dbFile;
};

} // namespace Qt4
} // namespace SDP

#endif
