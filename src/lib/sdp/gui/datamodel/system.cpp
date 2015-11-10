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

#include <sdp/gui/datamodel/system.h>
#include <QDir>


namespace SDP {
namespace Qt4 {


Environment::Environment() {

	__installDir = QDir::homePath() + QDir::separator() + "sdp";
	__binDir = __installDir + QDir::separator() + "bin";
	__shareDir = __installDir + QDir::separator() + "share";
	__libDir = __installDir + QDir::separator() + "lib";
	__includeDir = __installDir + QDir::separator() + "include";
	__configDir = __installDir + QDir::separator() + "etc";
	__docDir = __shareDir + QDir::separator() + "doc";
	__userDir = QDir::homePath() + QDir::separator() + ".sdp";
	__userShareDir = __userDir + QDir::separator() + "share";
	__userConfigDir = __userDir + QDir::separator() + "etc";
}


Environment::~Environment() {}


const QString& Environment::installDir() const {
	return __installDir;
}


const QString& Environment::binDir() const {
	return __binDir;
}


const QString& Environment::shareDir() const {
	return __shareDir;
}


const QString& Environment::libDir() const {
	return __libDir;
}


const QString& Environment::includeDir() const {
	return __includeDir;
}


const QString& Environment::configDir() const {
	return __configDir;
}


const QString& Environment::docDir() const {
	return __docDir;
}


const QString& Environment::userDir() const {
	return __userDir;
}


const QString& Environment::userShareDir() const {
	return __userShareDir;
}


const QString& Environment::userConfigDir() const {
	return __userConfigDir;
}


void Environment::setUserDir(const QString& v) {
	__userDir = v;
}


void Environment::setUserShareDir(const QString& v) {
	__userShareDir = v;
}


void Environment::setUserConfigDir(const QString& v) {
	__userConfigDir = v;
}


} // namespace Qt4
} // namespace SDP
