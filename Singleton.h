//
// Created by Samit Basu on 12/17/17.
//

#ifndef OLYMPUS_MC_SINGLETON_H
#define OLYMPUS_MC_SINGLETON_H
namespace CDSF {

    template <typename T>
    class Singleton
    {
    public:
        static T& it() {
            static T instance;
            return instance;
        }
    protected:
        Singleton() {}
        ~Singleton() {}

    public:
        Singleton(Singleton const &) = delete;
        Singleton& operator=(Singleton const &) = delete;
    };
}

#endif //OLYMPUS_MC_SINGLETON_H
