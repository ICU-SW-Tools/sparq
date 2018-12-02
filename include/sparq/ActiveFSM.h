//
// Created by Samit Basu on 12/28/17.
//

#ifndef SPARQ_ACTIVEFSM_H
#define SPARQ_ACTIVEFSM_H

#include "FSM.h"
#include "SafeQ.h"
#include "TimeQ.h"
#include "PODVariant.h"
#include <type_traits>
#include <thread>

#ifdef SPARQ_DEBUG
#include <syslog.h>
#endif

namespace sparq {
    /*
     * An Active FSM couples two paradigms together.  One is an active object, which is a thread that owns
     * it's own resources and that can only communicate via message passing.  The second paradigm is the
     * Finite State Machine, which provides event-driven control logic.  To couple the two together, we use
     * a polymorphic event class (which is a wrapper around an enum and a heap-allocated payload when needed)
     * which is then used to "feed" the FSM.  In a sense, the FSM is the handler for the events that are
     * generated by the active object.  An Active FSM supports two modes of communication.  Input is done via
     * a message queue.  To send a start command to an AFSM, for example, you post a "EventID::Start" message
     * to it's input queue.  You can send any number of commands - they are dispatched and handled in order
     * and held in an input FIFO until the AFSM thread can service the command.
     *
     * The other communication mechanism is outputs, which are broadcasts (pub-sub) by design.  So an AFSM can
     * receive commands on its input on a FIFO and dispatch outputs on a pub-sub.  For example, suppose we
     * have a simple AFSM that can start, stop, and error.  It has an idle state, a running state, and an
     * error state.  Furthermore, it publishes its state as a struct message on an output called "mystate".
     *
     * We first define the event type for this AFSM.  We can use any set of types, provided they are compatible
     * with a variant type:
     *
     * typedef variant<Event1, Event2, Event3, Event4> MyEventType;
     *
     * The AFSM would be constructed as follows:
     *
     * We also define default handlers for events at the Widget level.  These allow us to address events
     * that are otherwise unhandled in the various operational states.
     *
     * struct Widget : public AFSM<Widget, MyEventType> {
     *    static Topic<StateStruct> mystate;
     *    void operator()(const MyEventType &) { // Provide default handling here }
     * };
     *
     * We can now subclass Widget to provide states.  Each state supports an entry and exit function
     * that is executed when the state becomes active.  And we provide operator() overloads for each
     * event type
     *
     * struct Idle : Widget {
     *    void entry() {
     *       // Do entry stuff here
     *       mystate.publish(StateStruct(Idle));
     *    }
     *    void exit() {
     *       //  Do exit stuff here
     *    }
     *    void operator()(const Event1 &) {
     *       // React to event 1 here.  If we want to transition to a new state, we
     *       // use the transit function
     *       transit<Busy>();
     *    }
     *    void operator()(const Event2 &) {
     *       // React to event 2 here.  Etc.
     *    }
     * }
     *
     * To send events to the AFSM, you use the "push" operator
     *
     *     Widget::push(Event1());
     *
     * Which can take any event that belongs to MyEventType.
     *
     * You can connect two AFSMs by connecting a Topic from one to the input of another.  For example,
     * if we have AFSM1, which publishes an event of type Foo, and Foo is on the input variant list of
     * AFSM2, we can
     *
     *     AFSM1::topic.subscribe(AFSM2)
     *
     * Which will push events into AFSM2's inputQ.
     */

/*    template <class StateMachine, class EventType>
    class ActiveFSM : public ActiveObject<EventType> {
    public:
        void initialize() override {
            StateMachine::initialize();
        }
        bool process(const EventType &t) override {
            StateMachine::dispatch(t);
            return true; // TODO - need way to terminate object
        }
    };
*/

    template <typename ... Ts>
    using EventT = PODVariant<Ts ...>;

    // Adopted from TinyFSM - https://github.com/digint/tinyfsm, MIT License

    // We want to use inheritance and polymorphism without necessarily incurring
    // the cost/risk associated with heap allocation.  This is tricky - the solution
    // used by TinyFSM is static instances of the state classes.  In effect, they become
    // namespaces for a bunch of (static) functions.  It's a clever technique, but must be
    // used with care.

    // This is a templated struct that holds the static instance of the state class
    // and provides typedefs for the templated types.
    template<class F, class S>
    struct _state_instance_active {
        static S value;
        typedef S value_type;
        typedef _state_instance_active<F, S> type;
    };

    template<class F, class S>
    typename _state_instance_active<F, S>::value_type _state_instance_active<F, S>::value;

    // This FSM is very lightweight, but it has singleton semantics.  That seems suboptimal
    // from a testing/infrastructure perspective.
    template<class F, class EventType>
    class AFSM {

        typedef F *state_ptr_t;

        struct AFSMInternals {
            state_ptr_t current_state;
            SafeQ<EventType> event_Q;
            TimeQ timers = TimeQ();
            std::thread *t1 = nullptr;
            bool quit = false;
        };

        static AFSMInternals self;

        template<typename S>
        static void enter() {
            self.quit = false;
            self.current_state = state_ptr<S>();
            self.current_state->entry();
            self.t1 = new std::thread(&AFSM::run);
        }

        static void run() {
            while (!self.quit) {
                if (self.timers.empty()) {
                    auto obj = self.event_Q.pop();
                    self.current_state->react(obj);
                } else {
                    EventType obj;
                    bool msg = self.event_Q.tryPopUntil(&obj,self.timers.next());
                    self.timers.update();
                    if (msg)
                        self.current_state->react(obj);
                }
            }
        }

    protected:
        virtual void entry() {};

        virtual void exit() {};

        virtual std::string name() {return "";}

        static void signal_quit() {
            self.quit = true;
            self.event_Q.signal();
        }

        template<class S>
        static constexpr state_ptr_t state_ptr() {
            return &_state_instance_active<F, S>::value;
        }

        template<typename S>
        void transit() {
            self.current_state->exit();
#ifdef SPARQ_DEBUG
            if (!self.current_state->name().empty())
              syslog(LOG_DEBUG,"-%s",self.current_state->name().c_str());
#endif
            self.current_state = state_ptr<S>();
#ifdef SPARQ_DEBUG
            if (!self.current_state->name().empty())
              syslog(LOG_DEBUG,"+%s",self.current_state->name().c_str());
#endif
            self.current_state->entry();
        }

        template<typename S, typename ActionFunction>
        void transit(ActionFunction actionFunction) {
            static_assert(std::is_void<typename std::result_of<ActionFunction()>::type>::value,
                          "result type of 'action_function()' is not 'void'");
            self.current_state->exit();
            actionFunction();
            self.current_state = state_ptr<S>();
            self.current_state->entry();
        }

        template<typename S, typename ActionFunction, typename ConditionFunction>
        void transit(ActionFunction actionFunction, ConditionFunction conditionFunction) {
            static_assert(std::is_same<typename std::result_of<ConditionFunction()>::type, bool>::value,
                          "result of 'condition_funciton()' is not 'bool'");
            if (conditionFunction()) {
                transit<S>(actionFunction);
            }
        }

        timerid_t addTimer(uint64_t micros, const EventType &msg) {
            return self.timers.add(mkTimeStamp(micros, [=]() {
                this->push(msg);
            }));
        }

        bool cancelTimer(timerid_t id) {
            return self.timers.cancel(id);
        }

    public:
        static void initialize();

        static void push(const EventType &event) {
            //std::cout << "Pushed event " << typeid(event).name() << " to " << __PRETTY_FUNCTION__ << "\n";
            self.event_Q.push(event);
        }

        static void join() {
            self.t1->join();
        }

        virtual void operator()(const EventType &t) {
            std::cout << "Default handler called \n";
        }

        virtual void react(const EventType &t) {
            t.visit(*(self.current_state));
        }
    };


    template<class F, class Event>
    typename AFSM<F, Event>::AFSMInternals AFSM<F, Event>::self;


#define AFSM_INITIAL_STATE(_FSM, _EVENT, _STATE) \
    namespace sparq { \
    template<> void AFSM<_FSM, _EVENT>::initialize() { \
        enter<_STATE>(); \
      } \
    }


// Syntactic sugar
#define defineActiveFSM(name,event) struct name : public sparq::AFSM<name, event>
#define defaultEventHandler(x) virtual void operator()(const x& event)
#define onEvent(x) virtual void operator()(const x& event) override
#define onEntry() virtual void entry() override
#define onExit() virtual void exit() override
#define defineState(fsm,substate) struct substate : public fsm
#define predefineState(fsm,substate) struct substate
#define implementEventHandler(name,x) void name::operator()(const x& event)
#define mapEventToState(name,event,state) void name::operator()(const event&) {std::cout << "Got Event " << #event << " -> " << #state << "\n"; transit<state>();}
#define debugName(str) virtual std::string name() override {return str;}
}

#endif //SPARQ_ACTIVEFSM_H
