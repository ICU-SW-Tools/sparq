#include <iostream>
#include <sparq/ActiveObject.h>
#include <sparq/PubSub.h>
#include <sparq/TimeQ.h>
#include "NewFSM.h"
#include <sparq/PODVariant.h>
#include <cstring>

using namespace sparq;

class TimedQuitter : public ActiveObject<int> {
public:
    TimedQuitter() {
        this->push(0);
    }
    void foo() {
        std::cout << "FOO\n";
    }
    bool process(const int &t) {
        std::cout << "Got Message " << t << "\n";
        switch (t) {
            case 0: // Startup message;
                this->addTimer(1e6, 1);
                break;
            case 1:
                this->addTimer(1e4, 2);
                break;
            case 2:
                this->addTimer(1e3, std::bind(&TimedQuitter::foo, this));
                this->addTimer(1e4, 4);
                break;
            case 4: // Shutdown message;
                return false;
        }
        return true;
    }
};


class Quitter : public ActiveObject<int> {
public:
    bool process(const int &p) {
        std::cout << "Got Message " << p << "! Quitting" << std::endl;
        return false;
    }
};

void foo(int t) {
    std::cout << "Foo says " << t << "\n";
}

void goo(int t) {
    std::cout << "Goo says " << t << "\n";
}

void poo() {
    std::cout << "Poo\n";
}

struct h0 {};
struct h1 : h0{};
struct h2 : h0 {};
struct h3 : h0 {};


class TestMan {
public:

    void operator()(const h1 &) {
        std::cout << "h1 fed\n";
    }

    void operator()(const h2 &) {
        std::cout << "h2 fed\n";
    }

    void operator()(const h0 &) {
        std::cout << "h0 fed\n";
    }
};

struct EV1 {};

struct EV2 {
    int len;
};

struct EV3 {
    char foo[32];
};

struct EV4 {
    EV2 len;
    EV3 name;
};

struct EV5 : EV4 {
    int64_t bar;
};

struct Printer {
    void operator()(const EV1 &) {
        std::cout << "got EV1\n";
    }
    void operator()(const EV2 &) {
        std::cout << "got EV2\n";
    }
    void operator()(const EV3 &) {
        std::cout << "got EV3\n";
    }
    void operator()(const EV4 &) {
        std::cout << "got EV4\n";
    }
};

int main() {

    std::cout << "EV1: " << std::is_pod<EV1>::value << "\n";
    std::cout << "EV2: " << std::is_pod<EV2>::value << "\n";
    std::cout << "EV3: " << std::is_pod<EV3>::value << "\n";
    std::cout << "EV4: " << std::is_pod<EV4>::value << "\n";
    std::cout << "EV5: " << std::is_pod<EV5>::value << "\n";

    using PDfoo = PODVariant<EV1, EV2, EV3, EV4>;

    EV3 ev3;
    strcpy(ev3.foo,"Hello");

    EV4 ev4;
    ev4.len.len = 32;
    strcpy(ev4.name.foo,"Hello");

    PDfoo x;
    x = ev4;

    std::cout << "Sizeof x " << sizeof(x) << "\n";

    std::cout << "x contains EV1: " << x.contains<EV1>() << "\n";

    std::cout << "x contains EV2: " << x.contains<EV2>() << "\n";

    std::cout << "x contains EV4: " << x.contains<EV4>() << "\n";

    PDfoo y = x;

    std::cout << "y contains EV4: " << y.contains<EV4>() << "\n";

    ev4.len.len = 16;

    y = ev4;

    Printer printer;

    y.visit(printer);

//    std::cout << "x == y: " << (x==y) << "\n";

    std::vector<PDfoo> fooQ;
    fooQ.push_back(PDfoo(EV1()));
    fooQ.push_back(PDfoo(EV2()));
    fooQ.push_back(PDfoo(EV3()));
    fooQ.push_back(PDfoo(EV4()));

    std::cout << "fooQ[0] is EV1: " << fooQ[0].contains<EV1>() << "\n";
    std::cout << "fooQ[1] is EV2: " << fooQ[1].contains<EV2>() << "\n";
    std::cout << "fooQ[2] is EV3: " << fooQ[2].contains<EV3>() << "\n";
    std::cout << "fooQ[3] is EV4: " << fooQ[3].contains<EV4>() << "\n";

    using ev = PODVariant<h1, h2, h3>;

    std::vector<ev> eventQ;
    eventQ.push_back(ev(h1()));
    eventQ.push_back(ev(h3()));
    eventQ.push_back(ev(h2()));
    eventQ.push_back(ev(h2()));
    eventQ.push_back(ev(h1()));

    const PODVariant<h1,h2,h3> test = h1();

    std::cout << "Variant test has: " << test.contains<h1>() << "\n";

    TestMan p;
    test.visit(p);

    for (const auto &x : eventQ) {
        x.visit(p);
    }

    FooBar::FooBarFSM::myState.subscribe("main", [=](const std::string &s) {
        std::cout << "----> " << s << "\n";
    });

    FooBar::FooBarFSM thing;
    thing.initialize();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    thing.push(FooBar::Start());
    //thing.push(FooBar::Stop());
    thing.join();

    FooBar::FooBarFSM::myState.unsubscribe("main");

/*    FooBar::FooBarAFSM thing;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    thing.push(FooBar::Start());
    thing.push(FooBar::Stop());

    Sample::initialize();
    auto start = std::chrono::high_resolution_clock::now();
    for (int i=0;i<100000000;i++) {
        Sample::dispatch(EventMsg(eventID::Go));
        Sample::dispatch(EventMsg(eventID::Stop));
    }
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "Elapsed time for 100e6 runs: " << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() << " usec\n";

    start = std::chrono::high_resolution_clock::now();
    for (int i=0;i<100000000;i++) {
        Sample::dispatch(GoMsg(32));
        Sample::dispatch(EventMsg(eventID::Stop));
    }
    elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "Elapsed time for 100e6 runs: " << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() << " usec\n";

    start = std::chrono::high_resolution_clock::now();
    auto e_go = new go(32);
    auto e_stop = new stop();
    for (int i=0;i<100000000;i++) {
        Sample::dispatch(e_go);
        Sample::dispatch(e_stop);
    }
    elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "Elapsed time for 100e6 runs: " << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() << " usec\n";
*/
    /*
    Elevator::initialize();
    start = std::chrono::high_resolution_clock::now();
    for (int i=0;i<100000000;i++) {
        Elevator::dispatch(Go());
        Elevator::dispatch(Stop());
    }
    elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "Elapsed time for 100e6 runs: " << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() << " usec\n";
    std::cout << "Elevator run count: " << Elevator::run_count << "\n";
    Widget w;
    w.initialize();
    start = std::chrono::high_resolution_clock::now();
    DGo go;
    DStop stop;
    for (int i=0;i<100000000;i++) {
        go.apply(&w);
        stop.apply(&w);
    }
    elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "Elapsed time for 100e6 runs: " << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() << " usec\n";
    std::cout << "Elevator run count: " << w.counter << "\n";
    auto e = Go();
    std::cout << "Size of " << sizeof(e) << " Go\n";
*/

    Quitter q;
    std::cout << "Hello, World!" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    q.push(5);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    PubSub<int> comm;
    comm.subscribe("foo",foo);
    comm.subscribe("goo",goo);
    comm.publish(3);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    comm.unsubscribe("foo");
    comm.publish(2);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    comm.unsubscribe("goo");
    comm.publish(1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    comm.stop();

    TimedQuitter s;
    s.join();

    TimeQ t;
    const int ms = 1000;
    auto now = Clock::now();
    t.add(mkTimeStamp(500*ms, poo));
    t.add(mkTimeStamp(50*ms, poo));
    t.add(mkTimeStamp(3500*ms, poo));
    t.add(mkTimeStamp(5*ms, poo));
    t.add(mkTimeStamp(4500*ms, poo));
    t.add(mkTimeStamp(1*ms, poo));
    auto g = t.add(mkTimeStamp(100*ms, poo));
    t.cancel(g);
    std::cout << "\n\n";
    while (!t.empty()) {
        std::this_thread::sleep_until(t.top().when);
        std::cout << t.top() << ":" << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - now) << "\n";
        t.pop();
    }
    t.drain();


    return 0;
}