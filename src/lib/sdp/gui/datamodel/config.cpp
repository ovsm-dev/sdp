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


#include <sdp/gui/datamodel/config.h>
#include <QtGui>



namespace SDP {
namespace Qt4 {


Config::Config(const QString& file) :
		__file(file) {

	if ( file.isEmpty() )
		__cfgFile = new ConfigFile;
	else
		__cfgFile = new ConfigFile(file.toStdString());
}


Config::~Config() {
	delete __cfgFile;
}


QString Config::getString(const QString& key) throw (Exception) {

	QString value;
	try {
		value = QString::fromStdString(__cfgFile->read<std::string>(key.toStdString()));
	} catch ( ... ) {
		throw Exception("Key " + key + " not found in configuration file");
	}

	return value;
}


QStringList Config::getStrings(const QString& key) throw (Exception) {

	QStringList values;
	try {
		values = QString::fromStdString(__cfgFile->read<std::string>(key.toStdString())).split(',');
	} catch ( ... ) {
		throw Exception("Key " + key + " not found in configuration file");
	}

	return values;
}


bool Config::getBool(const QString& key) throw (Exception) {

	bool value;
	try {
		value = __cfgFile->read<bool>(key.toStdString());
	} catch ( ... ) {
		throw Exception("Key " + key + " not found in configuration file");
	}

	return value;
}


QList<bool> Config::getBools(const QString& key) throw (Exception) {

	QList<bool> list;
	QStringList values;
	try {
		values = QString::fromStdString(__cfgFile->read<std::string>(key.toStdString())).split(',');
	} catch ( ... ) {
		throw Exception("Key " + key + " not found in configuration file");
	}

	for (int i = 0; i < values.size(); ++i) {
		if ( values.at(i).contains("true") )
			list << true;
		else if ( values.at(i).contains("false") )
			list << false;
		else
			throw Exception("Unrecognized format: bool has to be true of false");
	}

	return list;
}


double Config::getDouble(const QString& key) throw (Exception) {

	double value;
	try {
		value = QString::fromStdString(__cfgFile->read<std::string>(key.toStdString())).toDouble();
	} catch ( ConfigFile::key_not_found& ) {
		throw Exception("Key " + key + " not found in configuration file");
	}

	return value;
}


QList<double> Config::getDoubles(const QString& key) throw (Exception) {

	QList<double> list;
	QStringList values;

	try {
		values = QString::fromStdString(__cfgFile->read<std::string>(key.toStdString())).split(',');
	} catch ( ConfigFile::key_not_found& ) {
		throw Exception("Key " + key + " not found in configuration file");
	}

	for (int i = 0; i < values.size(); ++i)
		list << values.at(i).toDouble();

	return list;
}


int Config::getInt(const QString& key) throw (Exception) {

	int value;
	try {
		value = QString::fromStdString(__cfgFile->read<std::string>(key.toStdString())).toInt();
	} catch ( ConfigFile::key_not_found& ) {
		throw Exception("Key " + key + " not found in configuration file");
	}

	return value;
}


QList<int> Config::getInts(const QString& key) throw (Exception) {

	QList<int> list;
	QStringList values;
	try {
		values = QString::fromStdString(__cfgFile->read<std::string>(key.toStdString())).split(',');
	} catch ( ConfigFile::key_not_found& ) {
		throw Exception("Key " + key + " not found in configuration file");
	}

	for (int i = 0; i < values.size(); ++i)
		list << values.at(i).toInt();

	return list;
}


} // namespce Qt4
} // namespace SDP
