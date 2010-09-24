#include "simple_sndfile.hh"
#include "string.hh"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

using namespace jill::util;

SimpleSndfile::SimpleSndfile() {}

SimpleSndfile::SimpleSndfile(const char *filename, size_type samplerate)
{
	_open(filename, samplerate);
}

SimpleSndfile::SimpleSndfile(const std::string &filename, size_type samplerate)
{
	_open(filename.c_str(), samplerate);
}

void
SimpleSndfile::_open(const char *filename, size_type samplerate)
{
	std::memset(&_sfinfo, 0, sizeof(_sfinfo));

	_sfinfo.samplerate = samplerate;
	_sfinfo.channels = 1;

	// detect desired file format based on filename extension
	std::string ext = boost::filesystem::extension(boost::filesystem::path(filename));
	if (ext == "wav") {
		_sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	} else if (ext == "aiff" || ext == "aif") {
		_sfinfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
	} else if (ext == "flac") {
		_sfinfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
#ifdef HAVE_SNDFILE_OGG
	} else if (ext == "ogg" || ext == "oga") {
		_sfinfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
#endif
	} else if (ext == "raw" || ext == "pcm") {
		_sfinfo.format = SF_FORMAT_RAW | SF_FORMAT_PCM_16;
	} else {
		throw FileError(make_string() << "failed to recognize file extension '" << ext << "'");
	}

	// open output file for writing
	SNDFILE *f = sf_open(filename, SFM_WRITE, &_sfinfo);
	if (!f) {
		throw FileError(make_string() << "couldn't open '" << filename << "' for output");
	}
	_sndfile.reset(f, sf_close);
	_entry.filename = filename;
	_entry.nframes = 0;
}

void
SimpleSndfile::_close()
{
	if(_sndfile) sf_close(_sndfile.get());
	_sndfile.reset();
	_entry.filename.clear();
	_entry.nframes = 0;
}

bool
SimpleSndfile::_valid() const
{
	return _sndfile;
}


SimpleSndfile::size_type
SimpleSndfile::_write(const float *buf, size_type nframes)
{
	size_type n = sf_writef_float(_sndfile.get(), buf, nframes);
	_entry.nframes += n;
	return n;
}


SimpleSndfile::size_type
SimpleSndfile::_write(const double *buf, size_type nframes) {
	size_type n = sf_writef_double(_sndfile.get(), buf, nframes);
	_entry.nframes += n;
	return n;
}


SimpleSndfile::size_type
SimpleSndfile::_write(const short *buf, size_type nframes) {
	size_type n = sf_writef_short(_sndfile.get(), buf, nframes);
	_entry.nframes += n;
	return n;
}


SimpleSndfile::size_type
SimpleSndfile::_write(const int *buf, size_type nframes) {
	size_type n = sf_writef_int(_sndfile.get(), buf, nframes);
	_entry.nframes += n;
	return n;
}
