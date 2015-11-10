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
#include <sdp/gui/datamodel/trigger.h>
#include <sdp/gui/datamodel/cache.h>
#include <sdp/gui/datamodel/macros.h>



namespace SDP {
namespace Qt4 {

const TriggerStatusValue& operator*(TriggerStatus ts) {
	SDPASSERT(ts >= 0 && ts < TRIGGER_STATUS_COUNT);
	SDPASSERT(g_TriggerStatusValues[ts].status == ts);
	return g_TriggerStatusValues[ts];
}

QDataStream& operator<<(QDataStream& stream, const Trigger::Station& s) {
	stream << s.networkCode << s.code << s.locationCode << s.channelCode << s.snapshotFile;
	return stream;
}

QDataStream& operator>>(QDataStream& stream, Trigger::Station& s) {
	stream >> s.networkCode >> s.code >> s.locationCode >> s.channelCode >> s.snapshotFile;
	return stream;
}


Trigger::Trigger(const QString& jobID, QObject* parent) :
		QObject(parent), __tableWidget(NULL), __creationTime(QDateTime::currentDateTime()),
		__jobID(jobID), __status(WaitingForRevision), __retCode(-2) {}


Trigger* Trigger::create(const QString& jobID) {

	SDPASSERT(Cache::instancePtr());
	Trigger* trig = new Trigger(jobID);

	if ( Cache::instancePtr()->addObject(trig) )
		return trig;
	else {
		delete trig;
		trig = NULL;
	}

	return trig;
}


bool Trigger::operator==(Trigger& trig) {
	return this->id() == trig.id();
}


Trigger::~Trigger() {
	//! Remove self from cache if one is available
	if ( Cache::instancePtr() )
	    Cache::instancePtr()->removeObject(id());
}


void Trigger::fromDataStream(QDataStream& stream) {

	quint32 tmp = 0;
	stream >> __creationTime >> __originTime >> __stations >> __resumePage
	    >> __info >> __jobID >> tmp >> __retCode;

	__status = static_cast<TriggerStatus>(tmp);
}


void Trigger::toDataStream(QDataStream& stream) {
	stream << __creationTime << __originTime << __stations << __resumePage
	    << __info << __jobID << static_cast<quint32>(__status) << __retCode;
}


const QString Trigger::id() {

	if ( __originTime.isValid() )
		return QString("Trigger#%1").arg(__originTime.toString("yyyyMMddHHmmss.zzz"));
	else
		return QString("Trigger#%1").arg(__creationTime.toString("yyyyMMddHHmmss.zzz"));
}

void Trigger::setStatus(const TriggerStatus& s) {
	__status = s;
}


const TriggerStatus& Trigger::status() const {
	return __status;
}


const QDateTime& Trigger::creationTime() const {
	return __creationTime;
}


void Trigger::setOriginTime(const QDateTime& dt) {
	__originTime = dt;
}


const QDateTime& Trigger::originTime() const {
	return __originTime;
}


const int& Trigger::runExitCode() const {
	return __retCode;
}


void Trigger::setTableWidget(void* tw) {
	__tableWidget = tw;
}


void* Trigger::tableWidget() {
	return __tableWidget;
}


void Trigger::setStations(const StationList& l) {
	__stations = l;
}


const Trigger::StationList& Trigger::stations() const {
	return __stations;
}


void Trigger::setResumePage(const QString& p) {
	__resumePage = p;
}


const QString& Trigger::resumePage() const {
	return __resumePage;
}


void Trigger::setInformation(const QString& p) {
	__info = p;
}


const QString& Trigger::information() const {
	return __info;
}


const QString& Trigger::jobID() const {
	return __jobID;
}

} // namespace Qt4
} // namespace SDP
