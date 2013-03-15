#include "multichannel_writer.hh"
#include "../dsp/period_ringbuffer.hh"

#if DEBUG
#include <iostream>
#endif

using namespace jill;
using namespace jill::file;

multichannel_writer::multichannel_writer(nframes_t buffer_size)
        : _stop(0), _xruns(0), _buffer(new dsp::period_ringbuffer(buffer_size))
{
        pthread_mutex_init(&_lock, 0);
        pthread_cond_init(&_ready, 0);
}

multichannel_writer::~multichannel_writer()
{
        pthread_mutex_destroy(&_lock);
        pthread_cond_destroy(&_ready);
}

nframes_t
multichannel_writer::push(void const * arg, period_info_t const & info)
{
        nframes_t r = _buffer->push(arg, info);
        if (r != info.nframes) xrun();
        // signal writer thread
        if (pthread_mutex_trylock (&_lock) == 0) {
                pthread_cond_signal (&_ready);
                pthread_mutex_unlock (&_lock);
        }
        return r;
}

void
multichannel_writer::xrun()
{
        __sync_add_and_fetch(&_xruns, 1);
}

// int
// multichannel_writer::xruns() const
// {
//         return _xruns;
// }

void
multichannel_writer::stop()
{
        __sync_add_and_fetch(&_stop, 1);
        // have to notify writer thread if it's waiting
        if (pthread_mutex_trylock (&_lock) == 0) {
                pthread_cond_signal (&_ready);
                pthread_mutex_unlock (&_lock);
        }
}

void
multichannel_writer::start()
{
        int ret = pthread_create(&_thread_id, NULL, multichannel_writer::thread, this);
        if (ret != 0)
                throw std::runtime_error("Failed to start writer thread");
}

void
multichannel_writer::join()
{
        pthread_join(_thread_id, NULL);
}

void
multichannel_writer::resize_buffer(nframes_t buffer_size)
{
        // the write thread will keep this locked until the buffer is empty
        pthread_mutex_lock(&_lock);
        _buffer->resize(buffer_size);
        pthread_mutex_unlock(&_lock);
}

void *
multichannel_writer::thread(void * arg)
{
        multichannel_writer * self = static_cast<multichannel_writer *>(arg);
        period_info_t const * period;

	pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_mutex_lock (&self->_lock);

        while (1) {
                period = self->_buffer->peek();
                if (period) {
                        self->write(period);
                        self->_buffer->release();
                }
                else if (!self->_stop) {
                        // wait for more data
                        pthread_cond_wait (&self->_ready, &self->_lock);
                }
                else {
                        break;
                }
        }
        self->flush();
        pthread_mutex_unlock(&self->_lock);
        return 0;
}

void
multichannel_writer::write(period_info_t const * info)
{
#if DEBUG
        std::cout << "got period: time=" << info->time << ", nframes=" << info->nframes << std::endl;
#endif
}

void
multichannel_writer::flush()
{
#if DEBUG
        std::cout << "terminating writer" << std::endl;
#endif
}