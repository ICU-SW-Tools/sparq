//
// Created by Samit Basu on 12/17/17.
//

#ifndef SPARQ_SINGLETON_H
#define SPARQ_SINGLETON_H
namespace sparq {

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

#endif //SPARQ_SINGLETON_H
