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


#include <sdp/gui/datamodel/macros.h>

#include <QStringList>
#include <QString>


QString outputFunction(QString name) {

	QString output;
	QStringList classParts = name.split("::");
	QStringList nameAndReturnType = classParts.first().split(" ");

	QString returnType = ""; //ctor, dtor don't have return types
	if ( nameAndReturnType.count() > 1 )
	    returnType = nameAndReturnType.first() + " ";
	QString className = nameAndReturnType.last();

	QStringList funcAndParamas = classParts.last().split("(");
	funcAndParamas.last().chop(1);
	QString functionName = funcAndParamas.first();
	QStringList params = funcAndParamas.last().split(",");

	output.append("\033[036m");
	output.append(returnType);
	output.append("\033[0m\033[32m");
	output.append(className);
	output.append("\033[0m::");
	output.append("\033[34m");
	output.append(functionName);
	output.append("\033[0m(");

	for (QStringList::const_iterator param = params.begin();
	        param != params.constEnd(); ++param) {

		if ( param != params.begin() ) {
			output.append("\033[0m,");
		}
		output.append("\033[036m");
		output.append((*param));
	}
	output.append("\033[0m)");

	return output;
}
