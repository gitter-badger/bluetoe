#ifndef BLUETOE_LINK_LAYER_SCHEDULED_RADIO_HPP
#define BLUETOE_LINK_LAYER_SCHEDULED_RADIO_HPP

#include <cstdint>
#include <bluetoe/link_layer/buffer.hpp>

namespace bluetoe {
namespace link_layer {

    class delta_time;
    class write_buffer;
    class read_buffer;

    /*
     * @brief Type responsible for radio I/O and timeing
     *
     * The API provides a set of scheduling functions, to schedule advertising or to schedule connection events. All scheduling functions take a point in time
     * to switch on the receiver / transmitter and to transmit and to receive. This points are defined as relative offsets to a previous point in time T0. The
     * first T0 is defined by the return of the constructor. After that, every scheduling function have to define what the next T0 is, that the next
     * functions relative point in time, is based on.
     */
    template < std::size_t TransmitSize, std::size_t ReceiveSize, typename CallBack >
    class scheduled_radio : public ll_data_pdu_buffer< TransmitSize, ReceiveSize, < scheduled_radio< TransmitSize, ReceiveSize, CallBack > >
    {
    public:
        /**
         * initializes the hardware and defines the first time point as anker for the next call to a scheduling function.
         */
        scheduled_radio();

        /**
         * @brief schedules for transmission of advertising data and starts to receive 150µs later
         *
         * The function will return immediately. Depending on whether a response is received or the receiving times out,
         * CallBack::adv_received() or CallBack::adv_timeout() is called. In both cases, every following call to a scheduling
         * function is based on the time, the tranmision was scheduled. So the new T0 = T0 + when. In case of a CRC error,
         * CallBack::adv_timeout() will be called immediately .
         *
         * This function is intended to be used for sending advertising PDUs. If the given receive buffer is empty, the timeout callback
         * will be called when the PDU was sent.
         *
         * @param channel channel to transmit and to receive on
         * @param transmit data to be transmitted
         * @param when point in time, when the first bit of data should be started to be transmitted
         * @param receive buffer where the radio will copy the received data, before calling Callback::adv_receive(). This parameter can be empty if no receiving is intended.
         */
        void schedule_advertisment_and_receive(
            unsigned                                    channel,
            const bluetoe::link_layer::write_buffer&    transmit,
            bluetoe::link_layer::delta_time             when,
            const bluetoe::link_layer::read_buffer&     receive );

        /**
         * @brief schedules a connection event
         *
         * The function will return immediately and schedule the receiver to start at start_receive.
         * CallBack::timeout() is called when between start_receive and end_receive no valid pdu is received. The new T0 is then the old T0.
         * CallBack::end_event() is called when the connection event is over. The new T0 is the time point where the first PDU was
         * reveived from the Master.
         *
         * In any case is one (and only one) of the callbacks called (timeout(), end_event()). The context of the callback call is run().
         *
         * Data to be transmitted and received is passed by the inherited ll_data_pdu_buffer.
         */
        void schedule_connection_event(
            unsigned                                    channel,
            bluetoe::link_layer::delta_time             start_receive,
            bluetoe::link_layer::delta_time             end_receive,
            bluetoe::link_layer::delta_time             connection_interval );

        /**
         * @brief set the access address initial CRC value for transmitted and received PDU
         *
         * The values should be changed, when there is no outstanding scheduled transmission or receiving.
         * The values will be applied with the next call to schedule_advertisment_and_receive() or schedule_receive_and_transmit().
         */
        void set_access_address_and_crc_init( std::uint32_t access_address, std::uint32_t crc_init );

        /**
         * @brief function to return a device specific value that is persistant and unique for the device (CPU id or such)
         */
        std::uint32_t static_random_address_seed() const;

        /**
         * @brief allocates the CPU to the scheduled_radio
         *
         * All callbacks given by the CallBack parameter are called from within this CPU context.
         * The function will return from time to time, when an external event happend. It's up to concrete
         * implementations to identify and to define situations where the CPU should be released back to the
         * calling application.
         */
        void run();

        /**
         * @brief forces the run() function to return at least once
         *
         * The function is intended to be used from interrupt handler to synchronize with the main thread.
         */
        void wake_up();

        /**
         * @brief type to allow ll_data_pdu_buffer to synchronize the access to the buffer data structures.
         */
        class lock_guard;
    };
}

}

#endif // include guard
