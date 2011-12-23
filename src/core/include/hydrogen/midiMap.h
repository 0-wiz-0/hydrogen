/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef MIDIMAP_H
#define MIDIMAP_H


#include <map>
#include <cassert>
#include <hydrogen/object.h>
#include <hydrogen/midi_action.h>

#include <QtCore/QMutex>

class Action;

class MidiActionMap : public H2Core::Object
{
    H2_OBJECT
	public:
                typedef std::map< QString, MidiAction* > map_t;
		static MidiActionMap* __instance;
		~MidiActionMap();

		static void create_instance();
		static void reset_instance();  // convenience accessor to reset()
		static MidiActionMap* get_instance() { assert(__instance); return __instance; }

		void reset();  // Reinitializes the object.

                void registerMMCEvent( QString, MidiAction* );
                void registerNoteEvent( int , MidiAction* );
                void registerCCEvent( int , MidiAction * );

                map_t getMMCMap();

                MidiAction* getMMCAction( QString );
                MidiAction* getNoteAction( int note );
                MidiAction * getCCAction( int parameter );

		void setupNoteArray();

	private:
		MidiActionMap();

                MidiAction* __note_array[ 128 ];
                MidiAction* __cc_array[ 128 ];

                map_t mmcMap;
		QMutex __mutex;
};
#endif
