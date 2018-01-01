//
// Created by Samit Basu on 12/27/17.
//

#ifndef OLYMPUS_MC_FSM_H
#define OLYMPUS_MC_FSM_H

#include <type_traits>

namespace CDSF {
    // Adopted from TinyFSM - https://github.com/digint/tinyfsm, MIT License

    // We want to use inheritance and polymorphism without necessarily incurring
    // the cost/risk associated with heap allocation.  This is tricky - the solution
    // used by TinyFSM is static instances of the state classes.  In effect, they become
    // namespaces for a bunch of (static) functions.  It's a clever technique, but must be
    // used with care.

    // This is a templated struct that holds the static instance of the state class
    // and provides typedefs for the templated types.
    template<class F, class S>
    struct _state_instance {
        static S value;
        typedef S value_type;
        typedef _state_instance<F, S> type;
    };

    template<class F, class S>
    typename _state_instance<F, S>::value_type _state_instance<F, S>::value;


    // This FSM is very lightweight, but it has singleton semantics.  That seems suboptimal
    // from a testing/infrastructure perspective.
    template<class F>
    class FSM {
        typedef F *state_ptr_t;

        static state_ptr_t current_state;

        template<typename S>
        static void enter() {
            current_state = state_ptr<S>();
            current_state->entry();
        }

    protected:
        virtual void entry() {};

        virtual void exit() {};

        template<class S>
        static constexpr state_ptr_t state_ptr() {
            return &_state_instance<F, S>::value;
        }

        template<typename S>
        void transit() {
            current_state->exit();
            current_state = state_ptr<S>();
            current_state->entry();
        }

        template<typename S, typename ActionFunction>
        void transit(ActionFunction actionFunction) {
            static_assert(std::is_void<typename std::result_of<ActionFunction()>::type >::value,
                          "result type of 'action_function()' is not 'void'");
            current_state->exit();
            actionFunction();
            current_state = state_ptr<S>();
            current_state->entry();
        };

        template<typename S, typename ActionFunction, typename ConditionFunction>
        void transit(ActionFunction actionFunction, ConditionFunction conditionFunction) {
            static_assert(std::is_same<typename std::result_of<ConditionFunction()>::type, bool>::value,
            "result of 'condition_funciton()' is not 'bool'");
            if (conditionFunction()) {
                transit<S>(actionFunction);
            }
        };

    public:
        static void initialize();

        template <class E>
        static void dispatch(const E &event) {
            current_state->react(event);
        }
    };


    template <class F>
    typename FSM<F>::state_ptr_t FSM<F>::current_state;

#define FSM_INITIAL_STATE(_FSM, _STATE) \
    template<> void FSM<_FSM>::initialize() { \
     enter<_STATE>(); \
    }

}

#endif //OLYMPUS_MC_FSM_H
