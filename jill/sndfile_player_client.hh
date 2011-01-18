/*
 * JILL - C++ framework for JACK
 *
 * includes code from klick, Copyright (C) 2007-2009  Dominic Sacre  <dominic.sacre@gmx.de>
 * additions Copyright (C) 2010 C Daniel Meliza <dmeliza@uchicago.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef _SNDFILE_PLAYER_CLIENT_HH
#define _SNDFILE_PLAYER_CLIENT_HH

#include "simple_client.hh"
#include "sndfile_player.hh"
#include "util/logger.hh"
#include <boost/shared_ptr.hpp>
#include <string>

namespace jill {

/**
 * @ingroup clientgroup
 * @brief play soundfiles to output port
 *
 * An implementation of @a SimpleClient that plays back sound files.
 * Reading, resampling, and buffering sound files is handled by @a
 * SndfilePlayer. Has a single output port.
 *
 * To load a new waveform:
 * load_file(key, filename);
 *
 * To select a loaded file for playback:
 * select(key);
 *
 * To start playback of the last loaded item:
 * run();
 * In nonblocking mode:
 * oneshot();
 *
 */
class SndfilePlayerClient : public SimpleClient {

public:
	SndfilePlayerClient(const char * client_name);
	virtual ~SndfilePlayerClient();

	/**
	 * Parse a list of input ports; if one input is of the form
	 * sndfile:path_to_sndfile, it will create a SndfilePlayerClient
	 * for the sndfile and replace the item in the list with the
	 * correct port name. Only works with one or zero inputs of
	 * this type
	 *
	 * @param ports   an iterable, modifiable sequence of port names
	 * @return a shared pointer to the SndfilePlayerClient created, if necessary
	 */
	template <class Iterable>
	static boost::shared_ptr<SndfilePlayerClient> from_port_list(Iterable &ports, util::logstream *os) {
		using std::string;
		boost::shared_ptr<SndfilePlayerClient> ret;
		typename Iterable::iterator it;
		for (it = ports.begin(); it != ports.end(); ++it) {
			size_t p = it->find("sndfile:");
			if (p!=string::npos) {
				string path = it->substr(8,string::npos);
				it->assign(path + ":out");
				ret.reset(new SndfilePlayerClient(path.c_str()));
				ret->_sounds.set_logger(os);
				ret->load_file(path, path.c_str());
				std::cout << ret->_sounds << std::endl;
				return ret;
			}
		}
		return ret;
	}

	/**
	 *  Load data from a file into the buffer. The data are
	 *  resampled to match the server's sampling rate, so this can
	 *  be a computationally expensive call.
	 *
	 *  @param key    an identifying key
	 *  @param path   the location of the file to load
	 *
	 *  @return number of samples loaded (adjusted for resampling)
	 */
	nframes_t load_file(const SndfilePlayer::key_type & key, const char * audiofile) {
		return _sounds.load_file(key, audiofile);
	}

	/**
	 * Select which item to play.
	 *
	 * @param key      an identifying key
	 *
	 * @throws NoSuchKey  if the key is invalid
	 * @return            the number of frames in the selected dataset
	 */
	nframes_t select(const SndfilePlayer::key_type & key) {
		return _sounds.select(key);
	}

private:

	SndfilePlayer _sounds;
	virtual void _stop(const char *reason);
	virtual void _reset() { 
		_sounds.reset();
		_status_msg = "Completed playback";
	}
	virtual bool _is_running() const { return (_sounds.position() > 0); }


};

} // namespace jill

#endif
