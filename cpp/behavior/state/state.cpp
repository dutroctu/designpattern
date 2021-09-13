#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

// back-end
#include <boost/msm/back/state_machine.hpp>
// front-end
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;

vector<string> state_names{
    "off hook"s,
    "connecting"s,
    "connected"s,
    "on hold"s,
    "destroyed"s
};

struct CallDialed {};
struct HungUp {};
struct CallConnected {};
struct PlacedOnHold {};
struct TakenOffHold {};
struct LeftMessage {};
struct PhoneThrownIntoWall {};

struct PhoneStateMachine : state_machine_def<PhoneStateMachine>
{
    bool angry{true};

    struct OffHook : state<> {};
    struct Connecting : state<> 
    {
        template <class Event, class FSM>
        void on_entry(Event const& evt, FSM&)
        {
            cout << "We are connecting..." << endl;
        }
    };

    struct PhoneBeingDestroyed
    {
        template<class EVT, class FSM, class SourceState, class TargetState>
        void operator()(EVT const&, FSM&, SourceState&, TargetState&)
        {
            cout << "Phone breaks into a million pieces" << endl;
        }
    };
    struct CanDestroyPhone
    {
        template<class EVT, class FSM, class SourceState, class TargetState>
        bool operator()(EVT const&, FSM& fsm, SourceState&, TargetState&)
        {
            return fsm.angry;
        }
    };

    struct Connected : state<> {};
    struct OnHold : state<> {};
    struct PhoneDestroyed : state<> {};

    struct transition_table : mpl::vector<
        Row<OffHook, CallDialed, Connecting>,
        Row<Connecting, CallConnected, Connected>,
        Row<Connected, PlacedOnHold, OnHold>,
        //                                                                         Guard
        Row<OnHold, PhoneThrownIntoWall, PhoneDestroyed, PhoneBeingDestroyed, CanDestroyPhone>
        >{};

    typedef OffHook initial_state;
};

int main()
{
    msm::back::state_machine<PhoneStateMachine> phone;

    auto info = [&]()
    {
        auto i = phone.current_state()[0];
        cout << "The phone is currently " << state_names[i] << "\n";
    };

    info();
    phone.process_event(CallDialed{});
    info();
    phone.process_event(CallConnected{});
    info();
    phone.process_event(PlacedOnHold{});
    info();
    phone.process_event(PhoneThrownIntoWall{});
    info();

    getchar();
    return 0;
}