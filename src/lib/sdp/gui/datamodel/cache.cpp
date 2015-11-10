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
#include <sdp/gui/datamodel/cache.h>
#include <sdp/gui/datamodel/parametermanager.h>
#include <sdp/gui/datamodel/databasemanager.h>
#include <sdp/gui/datamodel/system.h>
#include <sdp/gui/datamodel/logger.h>
#include <sdp/gui/datamodel/job.h>
#include <sdp/gui/datamodel/trigger.h>
#include <sdp/gui/datamodel/macros.h>


namespace SDP {
namespace Qt4 {


Cache::Cache() {
	SDPASSERT(Logger::instancePtr());
	Logger::instancePtr()->addMessage(Logger::DEBUG, __func__,
	    "Initiating application cache");
}


Cache::~Cache() {
	SDPASSERT(Logger::instancePtr());
	Logger::instancePtr()->addMessage(Logger::DEBUG, __func__,
	    QString("Objects remaining before destroying application cache: %1").arg(__cache.size()));
}


bool Cache::addObject(QObject* obj) {

	bool retcode = false;
	if ( !__cache.contains(obj) ) {
		__cache << obj;
		retcode = true;
	}

	return retcode;
}


template<> DetectionJob* Cache::getObject(const QString& id) {

	for (int i = 0; i < __cache.size(); ++i) {
		if ( DetectionJob* obj = dynamic_cast<DetectionJob*>(__cache.at(i)) ) {
			if ( obj->id() == id )
			    return obj;
		}
	}

	return NULL;
}


template<> DispatchJob* Cache::getObject(const QString& id) {

	for (int i = 0; i < __cache.size(); ++i) {
		if ( DispatchJob* obj = dynamic_cast<DispatchJob*>(__cache.at(i)) ) {
			if ( obj->id() == id )
			    return obj;
		}
	}

	return NULL;
}


template<> Trigger* Cache::getObject(const QString& id) {

	for (int i = 0; i < __cache.size(); ++i) {
		if ( Trigger* obj = dynamic_cast<Trigger*>(__cache.at(i)) ) {
			if ( obj->id() == id )
			    return obj;
		}
	}

	return NULL;
}


QVector<Trigger*> Cache::getTriggers(const QString& jobID) {

	QVector<Trigger*> vector;
	for (int i = 0; i < __cache.size(); ++i) {
		if ( Trigger* obj = dynamic_cast<Trigger*>(__cache.at(i)) ) {
			if ( obj->jobID() == jobID )
			    vector << obj;
		}
	}

	return vector;
}


bool Cache::removeObject(const QString& id, const bool& rmInDB,
                         const bool& rmDBChildren) {

	SDPASSERT(Logger::instancePtr());
	Logger* log = Logger::instancePtr();

	bool retcode = false;

	for (int i = 0; i < __cache.size(); ++i) {

		if ( DetectionJob* det = dynamic_cast<DetectionJob*>(__cache.at(i)) ) {
			if ( det->id() == id ) {
				log->addMessage(Logger::INFO, __func__,
				    "Removing detection object with id " + det->id() + " from cache");
				if ( rmInDB )
				    DatabaseManager::instancePtr()->removeDetection(det, rmDBChildren);
				__cache.at(i)->deleteLater();
				__cache.remove(i);
				retcode = true;
			}
		}
		else if ( DispatchJob* dis = dynamic_cast<DispatchJob*>(__cache.at(i)) ) {
			if ( dis->id() == id ) {
				log->addMessage(Logger::INFO, __func__,
				    "Removing dispatch object with id " + dis->id() + " from cache");
				if ( rmInDB )
				    DatabaseManager::instancePtr()->removeDispatch(dis);
				__cache.at(i)->deleteLater();
				__cache.remove(i);
				retcode = true;
			}
		}
		else if ( Trigger* trig = dynamic_cast<Trigger*>(__cache.at(i)) ) {
			if ( trig->id() == id ) {
				log->addMessage(Logger::INFO, __func__,
				    "Removing trigger object with id " + trig->id() + " from cache");
				if ( rmInDB )
				    DatabaseManager::instancePtr()->removeTrigger(trig);
				__cache.at(i)->deleteLater();
				__cache.remove(i);
				retcode = true;
			}
		}
	}

	return retcode;
}


void Cache::clear() {

	for (int i = __cache.size(); i != 0; --i) {
		delete __cache[i];
		__cache.remove(i);
	}
}


int Cache::byteSize() {
	int size = 0;
	for (int i = 0; i < __cache.size(); ++i)
		size += sizeof(__cache.at(i));
	return size;
}


int Cache::count() {
	return __cache.size();
}


} // namespace Qt4
} // namespace SDP

