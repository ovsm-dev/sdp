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



#ifndef __SDP_QT4_PARAMETERMANAGER_H__
#define __SDP_QT4_PARAMETERMANAGER_H__


#include <sdp/gui/datamodel/singleton.h>
#include <QHash>
#include <QString>
#include <QVariant>
#include <QByteArray>


namespace SDP {
namespace Qt4 {

/**
 * @class ParameterManager
 *
 * This class implements a low level parameter manager in which data of any
 * types, kinds, can be stored. Each parameter is identified by a unique id
 * (its name).
 *
 * @note This class inherits from Singleton class, which means that only one
 * instance is allowed by application, and also, that its public interface
 * is accessible by any object requesting it.
 */
class ParameterManager : public Singleton<ParameterManager> {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		typedef QHash<QString, QVariant> ParameterList;
		typedef QHash<QString, ParameterList> ObjectParameterList;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		ParameterManager();
		~ParameterManager();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		//! Clears the entire parameter list
		void clear();

		/**
		 * @brief Registers a parameter.
		 * @param name parameter's name
		 * @param data parameter's data
		 * @return true if success, false otherwise.
		 */
		bool registerParameter(const QString&, QVariant = QVariant());

		/**
		 * @brief Removes a parameter
		 * @param name Parameter's name
		 * @return true if success, false otherwise
		 */
		bool removeParameter(const QString&);

		/**
		 * @brief Assigns data to a registered parameter
		 * @param name parameter's name
		 * @param data parameter's data
		 * @return true if success, false otherwise.
		 */
		bool setParameter(const QString&, QVariant = QVariant());

		/**
		 * @brief Fetches a parameter's data.
		 * @param name parameter's name
		 * @return the constant reference to parameter's data.
		 */
		QVariant parameter(const QString&);

		/**
		 * @brief Checks if a parameter exists
		 * @param name Parameter's name
		 * @return true on success, false otherwise.
		 */
		bool exists(const QString&);

		/**
		 * @brief Exports parameters inside external file
		 * @param file The full file's path
		 * @return True on success, false otherwise
		 **/
		bool saveParameters(const QString&);

		/**
		 * @brief Reads parameters from external file
		 * @param file The full file's path
		 * @return True on success, false otherwise
		 **/
		bool loadParameters(const QString&);

		/**
		 * @brief Registers an object parameter
		 * @param objectName The object's name
		 * @param name Parameter's name
		 * @param data Parameter's data
		 * @return true if success, false otherwise.
		 */
		bool registerObjectParameter(const QString&, const QString&, QVariant = QVariant());
		bool removeObjectParameter(const QString&, const QString&);
		bool setObjectParameter(const QString&, const QString&, QVariant = QVariant());
		QVariant objectParameter(const QString&, const QString&);
		const ParameterList& getObjectParameters(const QString&);
		void setObjectParameters(const ParameterList&, const QString&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		ParameterList __parameters;
		ObjectParameterList __objectParameters;
};

} // namespace Qt4
} // namespace SDP

#endif
