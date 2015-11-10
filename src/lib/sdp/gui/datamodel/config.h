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



#ifndef __SDP_QT4_CONFIG_H__
#define __SDP_QT4_CONFIG_H__


#include <exception>
#include <string>
#include <QString>
#include <QList>
#include "../../../3rd-party/configfile/configfile.h"



namespace SDP {
namespace Qt4 {


/**
 * @class Exception
 * @brief This class implements a C++ standard exception supporting Qt QString
 *        type as a input.
 */
class Exception : public std::exception {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Exception() :
				__what("Configuration exception") {}
		explicit Exception(const QString& str) :
				__what(str) {}
		explicit Exception(const char* str) :
				__what(str) {}
		virtual ~Exception() throw () {}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		QString what() throw () {
			return __what;
		}
		const char* what() const throw () {
			return __what.toStdString().c_str();
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QString __what;
};


/**
 * @class Config
 * @brief This class implements a configuration file reader.
 *
 * From scratch, it supports genuine config file in which strings, numbers or
 * lists are referenced. Comments are also handled.
 *
 * @todo Implements configuration error detection
 */
class Config {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Config(const QString& file = QString());
		~Config();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		QString getString(const QString&) throw (Exception);
		QStringList getStrings(const QString&) throw (Exception);
		bool getBool(const QString&) throw (Exception);
		QList<bool> getBools(const QString&) throw (Exception);
		double getDouble(const QString&) throw (Exception);
		QList<double> getDoubles(const QString&) throw (Exception);
		int getInt(const QString&) throw (Exception);
		QList<int> getInts(const QString&) throw (Exception);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QString __file;
		ConfigFile* __cfgFile;
};


} // namespace Qt4
} // namespace SDP

#endif
