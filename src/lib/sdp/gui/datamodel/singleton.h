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


#ifndef __SDP_QT4_SINGLETON_H__
#define __SDP_QT4_SINGLETON_H__


#include <QtGlobal>
#include <unistd.h>


namespace SDP {
namespace Qt4 {

/**
 * @class Singleton
 *
 * This is a little bit complicated, but the best singleton implementation
 * I've ever seen.
 * It counts the relative inherited class's address and stores it in
 * __instance. Notice that derived class can inherit from more than one
 * Singleton class, but in that case 'this' of the derived class can differs
 * from 'this' of the Singleton. Solution of this problem is to get
 * non-existent object from address 0x1, casting it to both types and
 * check the difference, which is in fact the distance between
 * Singleton<T> and its derived type T.
 *
 * @note This implementation is meant to check existing instance of classes,
 *       therefore it won't create a new instance of the object of interest.
 **/
template<typename T> class Singleton {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		Singleton() {
			Q_ASSERT(!__instance);
			intptr_t offset = (intptr_t) (T*) 1 - (intptr_t) (Singleton*) (T*) 1;
			__instance = (T*) ((intptr_t) this + offset);
		}

		~Singleton() {
			Q_ASSERT(__instance);
			__instance = NULL;
		}

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		inline static T& instance() {
			Q_ASSERT(__instance);
			return *__instance;
		}

		inline static T* instancePtr() {
			Q_ASSERT(__instance);
			return __instance;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		static T* __instance;
};

template<typename T> T* Singleton<T>::__instance = NULL;


} // namespace Qt4
} // namespace SDP

#endif
