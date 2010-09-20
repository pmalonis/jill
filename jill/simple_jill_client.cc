
#include "simple_jill_client.hh"
#include "util/string.hh"
#include <jack/jack.h>
#include <cerrno>

using namespace jill;

SimpleJillClient::SimpleJillClient(const std::string &client_name, 
				   const std::string &input_name, 
				   const std::string &output_name)
	: JillClient(client_name), _output_port(0), _input_port(0)
{
	long port_flags;

	jack_set_process_callback(_client, &process_callback_, static_cast<void*>(this));

	if (!input_name.empty()) {
		port_flags = JackPortIsInput | ((output_name.empty()) ? JackPortIsTerminal : 0);
		if ((_input_port = jack_port_register(_client, input_name.c_str(), JACK_DEFAULT_AUDIO_TYPE,
						      port_flags, 0))==NULL)
			throw AudioError("can't register input port");
	}

	if (!output_name.empty()) {
		port_flags = JackPortIsOutput | ((input_name.empty()) ? JackPortIsTerminal : 0);
		if ((_output_port = jack_port_register(_client, output_name.c_str(), JACK_DEFAULT_AUDIO_TYPE,
						       port_flags, 0))==NULL)
			throw AudioError("can't register output port");
	}

	if (jack_activate(_client))
		throw AudioError("can't activate client");

}

SimpleJillClient::~SimpleJillClient()
{
	jack_deactivate(_client);
}	

int 
SimpleJillClient::process_callback_(nframes_t nframes, void *arg)
{
	SimpleJillClient *this_ = static_cast<SimpleJillClient*>(arg);

	if (this_->_process_cb) {
		try {
			sample_t *in=NULL, *out=NULL;
			if (this_->_input_port)
				in = (sample_t *)jack_port_get_buffer(this_->_input_port, nframes);
			if (this_->_output_port)
				out = (sample_t *)jack_port_get_buffer(this_->_output_port, nframes);
			nframes_t time = jack_last_frame_time(this_->_client);

			this_->_process_cb(in, out, nframes, time);
		}
		catch (const std::runtime_error &e) {
			this_->_err_msg = e.what();
			this_->_quit = true;
		}
	}

	return 0;
}


void 
SimpleJillClient::_connect_input(const std::string & port, const std::string *)
{
	if (_input_port) {
		int error = jack_connect(_client, port.c_str(), jack_port_name(_input_port));
		if (error && error != EEXIST)
			throw AudioError(util::make_string() << "can't connect "
					 << jack_port_name(_input_port) << " to " << port.c_str());
	}
	else
		throw AudioError("interface does not have an input port");
}


void 
SimpleJillClient::_connect_output(const std::string & port, const std::string *)
{
	if (_output_port) {
		int error = jack_connect(_client, jack_port_name(_output_port), port.c_str());
		if (error && error != EEXIST)
			throw AudioError(util::make_string() << "can't connect "
					 << jack_port_name(_output_port) << " to " << port.c_str());
	}
	else
		throw AudioError("interface does not have an output port");
}


void 
SimpleJillClient::_disconnect_all()
{
	jack_port_disconnect(_client, _output_port);
	jack_port_disconnect(_client, _input_port);
}
