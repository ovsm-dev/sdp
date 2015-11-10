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



#ifndef __SDP_QT4_CACHE_H__
#define __SDP_QT4_CACHE_H__


#include <sdp/gui/datamodel/singleton.h>
#include <QVector>


QT_FORWARD_DECLARE_CLASS(QObject);


namespace SDP {
namespace Qt4 {


class Trigger;

/**
 * @class Cache
 * @brief This class implements a basic cache engine for storing qobjects.
 */
class Cache : public Singleton<Cache> {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef QVector<QObject*> Tuple;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Cache();
		~Cache();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool addObject(QObject*);
		template<typename T> T getObject(const QString&);
		QVector<Trigger*> getTriggers(const QString& jobID);
		bool removeObject(const QString&,
		                  const bool& rmInDB = true,
		                  const bool& rmDBChildren = true);
		void clear();
		int byteSize();
		int count();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		Tuple __cache;
};


} // namespace Qt4
} // namespace SDP

#endif
