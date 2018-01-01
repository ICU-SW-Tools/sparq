//
// Created by Samit Basu on 12/28/17.
//

#ifndef OLYMPUS_MC_EVENT_H
#define OLYMPUS_MC_EVENT_H

#include <memory>

namespace CDSF {

    /*
     * An event is meant to be a lightweight, copyable enum that indicates some type of command or
     * information.  We do not have a real variant type, so we allow the event to carry a shared pointer
     * to a payload.  If you do not need the payload, you pay no penalty for it.  If you need to convey
     * more than just the event ID (say, a struct capturing your current state, for example), then
     * you can use the factory function mkMsg to create it.  This function is templated, so you do not
     * need a registration mechanism for the types.  However, the association of types to enums is up
     * to you to provide.  So, for example, if you have an EventID
     *
     *    EventID::MCStatusUpdate
     *
     * And associated with it is a struct containing the status of the MC (say MCStatusStruct), then
     * your code is responsible for extracting the MCStatusStruct from the event payload.  This can be done
     * with the getMsgPayload function (also templated).  If you attempt to extract a different type
     * you will get an error.  Unfortunately, the compiler cannot tell the correct type based on the EventID
     * information.
     */

    enum class EventID {
        None,
        Start,
        Stop,
        Error
    };

    struct EventPayload {
        EventPayload() = default;
        virtual ~EventPayload() = default;
    };

    template <class T>
    struct EventPayloadT : public EventPayload {
        T data;
        explicit EventPayloadT(const T& x) : data(x) {}
    };

    typedef std::shared_ptr<EventPayload> payload_ptr_t;

    struct Event {
        const EventID id = EventID::None;
        const payload_ptr_t payload = nullptr;
        explicit Event(EventID id, payload_ptr_t ptr = nullptr) : id(id), payload(std::move(ptr)) {}
    };

    template <class T>
    Event mkEvent(EventID id, const T& data) {
        return Event(id, payload_ptr_t(new EventPayloadT<T>(data)));
    }




}

#endif //OLYMPUS_MC_EVENT_H
