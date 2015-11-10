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
#include <sdp/gui/datamodel/parametermanager.h>
#include <sdp/gui/datamodel/logger.h>
#include <QtGui>


namespace SDP {
namespace Qt4 {


ParameterManager::ParameterManager() {}


ParameterManager::~ParameterManager() {}


void ParameterManager::clear() {
	__parameters.clear();
}


bool ParameterManager::registerParameter(const QString& name, QVariant data) {

	if ( __parameters.contains(name) ) {
		qWarning() << QString(__func__) << " parameter " << name << " already stored";
		return false;
	}
	__parameters.insert(name, data);

	return true;
}


bool ParameterManager::removeParameter(const QString& name) {

	if ( !__parameters.contains(name) ) {
		qWarning() << QString(__func__) << " parameter " << name << " not stored";
		return false;
	}
	__parameters.remove(name);

	return true;
}


bool ParameterManager::setParameter(const QString& name, QVariant data) {

	if ( !__parameters.contains(name) ) {
		qWarning() << QString(__func__) << " parameter " << name << " not found";
		return false;
	}
	__parameters[name] = data;

	return true;
}


QVariant ParameterManager::parameter(const QString& name) {

	if ( !__parameters.contains(name) ) {
		qWarning() << QString(__func__) << " requested unknown parameter " << name;
		return QVariant();
	}

	return __parameters.value(name);
}


bool ParameterManager::exists(const QString& name) {
	return __parameters.contains(name);
}


bool ParameterManager::saveParameters(const QString& filename) {

	QFile mfile(filename);
	if ( !mfile.open(QFile::WriteOnly) ) {
		qWarning() << "Could not open file for writing";
		return false;
	}

	QDataStream out(&mfile);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
	out.setVersion(QDataStream::Qt_4_8);
#else
	out.setVersion(QDataStream::Qt_4_6);
#endif
	out << QString(PROJECT_NAME).toLatin1()
	    << QString(PROJECT_COPYRIGHT).toLatin1()
	    << QString(PROJECT_VERSION).toLatin1()
	    << QString(PROJECT_WEBSITE).toLatin1()
	    << __parameters;
	mfile.close();

	return true;
}


bool ParameterManager::loadParameters(const QString& filename) {

	QFile mfile(filename);
	if ( !mfile.open(QFile::ReadOnly) ) {
		qWarning() << "Could not open file for reading";
		return false;
	}

	QByteArray name, copyright, version, website;
	ParameterList l;
	QDataStream in(&mfile);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
	in.setVersion(QDataStream::Qt_4_8);
#else
	in.setVersion(QDataStream::Qt_4_6);
#endif
	in >> name >> copyright >> version >> website >> l;
	mfile.close();

	for (ParameterList::iterator it = l.begin(); it != l.end(); ++it)
		qDebug() << (it).key() << " / " << (it).value();

	return true;
}


bool ParameterManager::registerObjectParameter(const QString& objectName,
                                               const QString& name,
                                               QVariant data) {

	Logger* log = Logger::instancePtr();

	if ( __objectParameters.contains(objectName) ) {

		ParameterList& l = __objectParameters[objectName];
		if ( l.contains(name) ) {
			log->addMessage(Logger::WARNING, __func__, QString("Object %1 parameter already stored in list.")
			    .arg(objectName).arg(name));
			return false;
		}
		else {
			l.insert(name, data);
		}
	}

	else {
		ParameterList l;
		l.insert(name, data);
		__objectParameters.insert(objectName, l);
	}

	return true;
}


bool ParameterManager::removeObjectParameter(const QString& objectName,
                                             const QString& name) {

	Logger* log = Logger::instancePtr();

	if ( !__objectParameters.contains(objectName) ) {
		log->addMessage(Logger::WARNING, __func__, QString("Couldn't find Object %1 in list")
		    .arg(objectName));
		return false;
	}
	else {
		ParameterList& l = __objectParameters[objectName];
		if ( !l.contains(name) ) {
			log->addMessage(Logger::WARNING, __func__, QString("Object %1 doesn't hold a parameter named %2")
			    .arg(objectName).arg(name));
			return false;
		}
		else {
			l.remove(name);
		}
	}

	return true;
}


bool ParameterManager::setObjectParameter(const QString& objectName,
                                          const QString& name, QVariant data) {

	Logger* log = Logger::instancePtr();

	if ( !__objectParameters.contains(objectName) ) {
		log->addMessage(Logger::WARNING, __func__, QString("Couldn't find Object %1 in list")
		    .arg(objectName));
		return false;
	}

	ParameterList& l = __objectParameters[objectName];
	if ( !l.contains(name) ) {
		log->addMessage(Logger::WARNING, __func__, QString("Couldn't find Object's %1 parameter %2 in list")
		    .arg(objectName).arg(name));
		return false;
	}

	l[name] = data;

	return true;
}


QVariant ParameterManager::objectParameter(const QString& objectName,
                                           const QString& name) {

	Logger* log = Logger::instancePtr();

	if ( !__objectParameters.contains(objectName) ) {
		log->addMessage(Logger::WARNING, __func__, QString("Couldn't find Object %1 in list")
		    .arg(objectName));
		return false;
	}

	ParameterList& l = __objectParameters[objectName];
	if ( !l.contains(name) ) {
		log->addMessage(Logger::WARNING, __func__, QString("Couldn't find Object's %1 parameter %2 in list")
		    .arg(objectName).arg(name));
		return false;
	}

	return l[name];
}


const ParameterManager::ParameterList&
ParameterManager::getObjectParameters(const QString& objectName) {
	return __objectParameters[objectName];
}


void ParameterManager::setObjectParameters(const ParameterList& parameters,
                                           const QString& objectName) {

	Logger* log = Logger::instancePtr();

	if ( !__objectParameters.contains(objectName) ) {
		log->addMessage(Logger::WARNING, __func__, QString("Failed to set Object %1 parameters")
		    .arg(objectName));
		return;
	}

	__objectParameters[objectName] = parameters;
}


} // namespace Qt4
} // namespace SDP
