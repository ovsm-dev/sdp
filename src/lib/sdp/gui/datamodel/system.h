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



#ifndef __SDP_QT4_DATAMODEL_SYSTEM_H__
#define __SDP_QT4_DATAMODEL_SYSTEM_H__


#include <QString>
#include <sdp/gui/datamodel/singleton.h>


namespace SDP {
namespace Qt4 {


class Environment : public Singleton<Environment> {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Environment();
		~Environment();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const QString& installDir() const;
		const QString& binDir() const;
		const QString& shareDir() const;
		const QString& libDir() const;
		const QString& includeDir() const;
		const QString& configDir() const;
		const QString& docDir() const;

		const QString& userDir() const;
		const QString& userShareDir() const;
		const QString& userConfigDir() const;
		void setUserDir(const QString&);
		void setUserShareDir(const QString&);
		void setUserConfigDir(const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QString __installDir;
		QString __binDir;
		QString __shareDir;
		QString __libDir;
		QString __includeDir;
		QString __configDir;
		QString __docDir;
		QString __userDir;
		QString __userShareDir;
		QString __userConfigDir;
};


} // namespace Qt4
} // namespace SDP

#endif
