/*
 * JILL - C++ framework for JACK
 *
 * additions Copyright (C) 2013 C Daniel Meliza <dan || meliza.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef _STIMFILE_HH
#define _STIMFILE_HH

#include <string>
#include <boost/scoped_array.hpp>
#include <sndfile.h>
#include "../stimulus.hh"

namespace jill { namespace file {

/**
 * A stimulus stored on disk in a file. This implementation of stimulus_t uses
 * libsndfile to load the samples from disk, and libsamplerate to resample (if
 * needed). The loaded samples are stored in an array managed by the object.
 */
class stimfile : public jill::stimulus_t {

public:
        /**
         * Initialize object with path of stimfile.
         *
         * @param path   the location of the stimulus file
         *
         * @throws jill::FileError if the file doesn't exist
         */
        stimfile(std::string const & path);
        ~stimfile();

        char const * name() const { return _name.c_str(); }

        nframes_t nframes() const { return _nframes; }
        nframes_t samplerate() const { return _samplerate; }

        sample_t const * buffer() const { return (_buffer) ? _buffer.get() : 0; }

        /**
         * Load samples from disk and resample as needed
         *
         * @param samplerate - the target samplerate, or 0 to use the file's rate
         */
        void load_samples(nframes_t samplerate=0);

private:
        std::string _name;
        SF_INFO _sfinfo;
        SNDFILE *_sndfile;

        nframes_t _nframes;
        nframes_t _samplerate;

        boost::scoped_array<sample_t> _buffer;
};

}} // namespace jill::file

#endif
