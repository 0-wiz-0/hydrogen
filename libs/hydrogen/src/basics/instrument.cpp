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

#include <hydrogen/basics/instrument.h>

#include <hydrogen/basics/adsr.h>
#include <hydrogen/basics/sample.h>
#include <hydrogen/basics/song.h>
#include <hydrogen/basics/drumkit.h>
#include <hydrogen/helpers/filesystem.h>
#include <hydrogen/audio_engine.h>

#include <cassert>

namespace H2Core
{

const char* Instrument::__class_name = "Instrument";

Instrument::Instrument( const int id, const QString& name, ADSR* adsr )
    : Object( __class_name )
    , __id( id )
    , __name( name )
    , __gain( 1.0 )
    , __volume( 1.0 )
    , __pan_l( 1.0 )
    , __pan_r( 1.0 )
    , __peak_l( 0.0 )
    , __peak_r( 0.0 )
    , __adsr( adsr )
    , __filter_active( false )
    , __filter_cutoff( 1.0 )
    , __filter_resonance( 0.0 )
    , __random_pitch_factor( 0.0 )
    , __midi_out_note( MIDI_MIDDLE_C )
    , __midi_out_channel( -1 )
    , __stop_notes( false )
    , __active( true )
    , __soloed( false )
    , __muted( false )
    , __mute_group( -1 )
    , __queued( 0 )
{
    for ( int i=0; i<MAX_FX; i++ ) __fx_level[i] = 0.0;
	for ( int i=0; i<MAX_LAYERS; i++ ) __layers[i] = NULL;
}

Instrument::Instrument( Instrument *other )
    : Object( __class_name )
    , __id( other->get_id() )
    , __name( other->get_name() )
    , __gain( other->__gain )
    , __volume( other->get_volume() )
    , __pan_l( other->get_pan_l() )
    , __pan_r( other->get_pan_r() )
    , __peak_l( other->get_peak_l() )
    , __peak_r( other->get_peak_r() )
    , __adsr( new ADSR( *( other->get_adsr() ) ) )
    , __filter_active( other->is_filter_active() )
    , __filter_cutoff( other->get_filter_cutoff() )
    , __filter_resonance( other->get_filter_resonance() )
    , __random_pitch_factor( other->get_random_pitch_factor() )
    , __midi_out_note( other->get_midi_out_note() )
    , __midi_out_channel( other->get_midi_out_channel() )
    , __stop_notes( other->is_stop_notes() )
    , __active( other->is_active() )
    , __soloed( other->is_soloed() )
    , __muted( other->is_muted() )
    , __mute_group( other->get_mute_group() )
    , __queued( other->is_queued() )
{
    for ( int i=0; i<MAX_FX; i++ ) __fx_level[i] = other->get_fx_level(i);

	for ( int i=0; i<MAX_LAYERS; i++ ) {
		InstrumentLayer *other_layer = other->get_layer(i);
		if ( other_layer ) {
		    __layers[i] = new InstrumentLayer( other_layer );
        } else {
		    __layers[i] = 0;
        }
	}
}

Instrument::~Instrument() {
	for ( int i=0; i<MAX_LAYERS; i++) {
        delete __layers[i];
        __layers[i] = 0;
	}
	delete __adsr;
	__adsr = 0;
}

Instrument* Instrument::create_empty() { return new Instrument( EMPTY_INSTR_ID, "Empty Instrument", new ADSR() ); }

Instrument* Instrument::load_instrument( const QString& drumkit_name, const QString& instrument_name ) {
    Instrument *i = create_empty();
    i->load_from( drumkit_name, instrument_name, false );
    return i;
}

void Instrument::load_from( Drumkit* drumkit, Instrument* instrument, bool is_live ) {
	for ( int i=0; i<MAX_LAYERS; i++ ) {
		InstrumentLayer *src_layer = instrument->get_layer(i);
		InstrumentLayer *my_layer = this->get_layer(i);
        if(src_layer==0) {
			if ( is_live )
				AudioEngine::get_instance()->lock( RIGHT_HERE );
			this->set_layer( NULL, i );
			if ( is_live )
				AudioEngine::get_instance()->unlock();
		} else {
	        QString sample_path =  drumkit->get_path() + "/" + src_layer->get_sample()->get_filename();
			Sample *sample = Sample::load( sample_path );
			if (sample==0) {
				_ERRORLOG( QString("Error loading sample %1. Creating a new empty layer.").arg(sample_path));
				if ( is_live )
					AudioEngine::get_instance()->lock( RIGHT_HERE );
				this->set_layer( NULL, i );
				if ( is_live )
					AudioEngine::get_instance()->unlock();
			} else {
			    if ( is_live )
				    AudioEngine::get_instance()->lock( RIGHT_HERE );
                this->set_layer( new InstrumentLayer(src_layer, sample), i );
                if ( is_live )
				    AudioEngine::get_instance()->unlock();
            }
		}
		delete my_layer;
	}
	if ( is_live )
		AudioEngine::get_instance()->lock( RIGHT_HERE );
	this->set_id( instrument->get_id() );
	this->set_name( instrument->get_name() );
	this->set_gain( instrument->get_gain() );
	this->set_volume( instrument->get_volume() );
	this->set_pan_l( instrument->get_pan_l() );
	this->set_pan_r( instrument->get_pan_r() );
	this->set_adsr( new ADSR( *( instrument->get_adsr() ) ) );
	this->set_filter_active( instrument->is_filter_active() );
	this->set_filter_cutoff( instrument->get_filter_cutoff() );
	this->set_filter_resonance( instrument->get_filter_resonance() );
	this->set_random_pitch_factor( instrument->get_random_pitch_factor() );
	this->set_muted( instrument->is_muted() );
	this->set_mute_group( instrument->get_mute_group() );
	if ( is_live )
		AudioEngine::get_instance()->unlock();
}

void Instrument::load_from( const QString& drumkit_name, const QString& instrument_name, bool is_live ) {
    QString dir = Filesystem::drumkit_path( drumkit_name );
    if ( dir.isEmpty() ) return;
    Drumkit *drumkit = Drumkit::load( dir );
    assert( drumkit );
    Instrument *instrument = drumkit->get_instruments()->find( instrument_name );
    if ( instrument!=0 ) {
        load_from( drumkit, instrument, is_live );
    }
    delete drumkit;
}

Instrument* Instrument::load_from( XMLNode* node ) {
    int id = node->read_int( "id", EMPTY_INSTR_ID, false, false );
    if ( id==EMPTY_INSTR_ID ) return 0;
    Instrument *instrument = new Instrument( id, node->read_string( "name", "" ), 0 );
    instrument->set_volume( node->read_float( "volume", 1.0f ) );
    instrument->set_muted( node->read_bool( "isMuted", false ) );
    instrument->set_pan_l( node->read_float( "pan_L", 1.0f ) );     // TODO 0.5f ???
    instrument->set_pan_r( node->read_float( "pan_R", 1.0f ) );     // TODO 0.5f ???
    // may not exist, but can't be empty
    instrument->set_filter_active( node->read_bool( "filterActive", true, false ) );
    instrument->set_filter_cutoff( node->read_float( "filterCutoff", 1.0f, true, false ) );
    instrument->set_filter_resonance( node->read_float( "filterResonance", 0.0f, true, false ) );
    instrument->set_random_pitch_factor( node->read_float( "randomPitchFactor", 0.0f, true, false ) );
    float attack = node->read_float( "Attack", 0.0f, true, false );
    float decay = node->read_float( "Decay", 0.0f, true, false  );
    float sustain = node->read_float( "Sustain", 1.0f, true, false );
    float release = node->read_float( "Release", 1000.0f, true, false );
    instrument->set_adsr( new ADSR( attack, decay, sustain, release ) );
    instrument->set_gain( node->read_float( "gain", 1.0f, true, false ) );
    instrument->set_mute_group( node->read_int( "muteGroup", -1, true, false ) );
    instrument->set_midi_out_channel( node->read_int( "midiOutChannel", -1, true, false ) );
    instrument->set_midi_out_note( node->read_int( "midiOutNote", MIDI_MIDDLE_C, true, false ) );
    instrument->set_stop_notes( node->read_bool( "isStopNote", true ,false ) );
    for ( int i=0; i<MAX_FX; i++ ) {
        instrument->set_fx_level( node->read_float( QString("FX%1Level").arg(i+1), 0.0 ), i );
    }

    QDomNode filename_node = node->firstChildElement( "filename" );
    if ( !filename_node.isNull() ) {
        // back compatibility code
        // TODO should be automaticaly rewritten
        DEBUGLOG( "Using back compatibility code. filename node found" );
        QString sFilename = node->read_string( "filename", "");
        if( sFilename.isEmpty() ) {
            ERRORLOG( "filename back compability node is empty" );
        } else {
            Sample *sample = new Sample( sFilename, 0, 0 );
            InstrumentLayer *layer = new InstrumentLayer( sample );
            instrument->set_layer( layer, 0 );
        }
    } else {
        int n = 0;
        XMLNode layer_node = node->firstChildElement( "layer" );
        while ( !layer_node.isNull() ) {
            if ( n >= MAX_LAYERS ) {
                ERRORLOG( QString("n >= MAX_LAYERS (%1)").arg(MAX_LAYERS) );
                break;
            }
            instrument->set_layer( InstrumentLayer::load_from( &layer_node ), n );
            n++;
            layer_node = layer_node.nextSiblingElement( "layer" );
        }
    }
    return instrument;
}

bool Instrument::load_samples( const QString& path ) {
    for ( int i=0; i<MAX_LAYERS; i++ ) {
        InstrumentLayer *layer = get_layer(i);
        if(layer!=0) {
           if( !layer->load_sample(path) ) {
               return false;
           }
        }
    }
    return true;
}

bool Instrument::unload_samples() {
    for ( int i=0; i<MAX_LAYERS; i++ ) {
        InstrumentLayer *layer = get_layer(i);
        if(layer!=0) {
           if( !layer->unload_sample() ) {
               return false;
           }
        }
    }
    return true;
}

void Instrument::save_to( XMLNode* node ) {
    XMLNode instrument_node = node->ownerDocument().createElement( "instrument" );
    instrument_node.write_int( "id", __id );
    instrument_node.write_string( "name", __name );
    instrument_node.write_float( "volume", __volume );
    instrument_node.write_bool( "isMuted", __muted );
    instrument_node.write_float( "pan_L", __pan_l );
    instrument_node.write_float( "pan_R", __pan_r );
    instrument_node.write_float( "randomPitchFactor", __random_pitch_factor );
    instrument_node.write_float( "gain", __gain );
    instrument_node.write_bool( "filterActive", __filter_active );
    instrument_node.write_float( "filterCutoff", __filter_cutoff );
    instrument_node.write_float( "filterResonance", __filter_resonance );
    instrument_node.write_float( "Attack", __adsr->__attack );
    instrument_node.write_float( "Decay", __adsr->__decay );
    instrument_node.write_float( "Sustain", __adsr->__sustain );
    instrument_node.write_float( "Release", __adsr->__release );
    instrument_node.write_int( "muteGroup", __mute_group );
    instrument_node.write_int( "midiOutChannel", __midi_out_channel );
    instrument_node.write_int( "midiOutNote", __midi_out_note );
    instrument_node.write_bool( "isStopNote", __stop_notes );
    for ( int i=0; i<MAX_FX; i++ ) {
        instrument_node.write_float( QString("FX%1Level").arg(i+1), __fx_level[i] );
    }
    for ( int n = 0; n < MAX_LAYERS; n++ ) {
        InstrumentLayer* layer = get_layer(n);
        if(layer) {
            layer->save_to( &instrument_node );
        }
    }
    node->appendChild( instrument_node );
}

inline InstrumentLayer* Instrument::operator[]( int idx ) {
    assert(idx>=0 && idx <MAX_LAYERS);
    return __layers[ idx ];
}

inline InstrumentLayer* Instrument::get_layer( int idx ) {
    assert(idx>=0 && idx <MAX_LAYERS);
    return __layers[ idx ];
}

inline void Instrument::set_layer( InstrumentLayer* layer, int idx ) {
    assert(idx>=0 && idx <MAX_LAYERS);
	__layers[ idx ] = layer;
}

inline void Instrument::set_adsr( ADSR* adsr ) {
	if(__adsr) delete __adsr;
	__adsr = adsr;
}

inline void Instrument::set_midi_out_channel( int channel ) {
    if ((channel >= MIDI_OUT_CHANNEL_MIN) && (channel <= MIDI_OUT_CHANNEL_MAX)) {
        __midi_out_channel = channel;
    } else {
        ERRORLOG( QString("midi out channel %1 out of bounds").arg(channel) );
    }
}

inline void Instrument::set_midi_out_note( int note ) {
    if ((note >= MIDI_OUT_NOTE_MIN) && (note <= MIDI_OUT_NOTE_MAX)) {
        __midi_out_note = note;
    } else {
        ERRORLOG( QString("midi out note %1 out of bounds").arg(note) );
    }
}

};

/* vim: set softtabstop=4 expandtab: */
