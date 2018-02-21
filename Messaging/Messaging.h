//---------------------------------------------------------------------------
#ifndef MessagingH
#define MessagingH
//---------------------------------------------------------------------------
#include <list>
#include <memory>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
//---------------------------------------------------------------------------
namespace Messaging
{
class Bus
{
private:
    // Non-templated base class
    // Makes storing the templated objects easier
    class Subscription_
    {
    public:
        virtual ~Subscription_() {};
    };
    // Template specialization of a message subscription
    template <class T>
    class Subscription : public Subscription_
    {
    private:
        std::function<void (const T&)> handler;
    public:
        __fastcall Subscription(std::function<void (const T&)> handler)
        : handler(handler)
        {
        }
        __property std::function<void (const T&)> Handler = { read = handler };
    };

    // the map of message subscription handlers
    typedef std::list<std::unique_ptr<Subscription_>> Subscriptions;
	typedef std::unordered_map<std::type_index, std::unique_ptr<Subscriptions>> SubscriptionsMap;
	static SubscriptionsMap* handlers;

public:
    // subscribe a handler to a templated message type
    template <class T>
    static void const Subscribe(std::function<void (const T&)> handler)
    {
        auto& subscriptions = (*handlers)[typeid(T)];
        if (subscriptions == nullptr)
        {
            subscriptions = std::make_unique<Subscriptions>();
            (*handlers)[typeid(T)] = std::move(subscriptions);
        }

        auto subscription = std::make_unique<Subscription<T>>(handler);
        subscriptions->push_back(std::move(subscription));
    }

    // unsubscribe a handler from a templated message type
    template <class T>
    static void Unsubscribe(std::function<void (const T&)> handler)
    {
        const std::unique_ptr<Subscriptions>& subscriptions = (*handlers)[typeid(T)];
        if (subscriptions != nullptr)
        {
            auto& subscription = std::find(subscriptions->begin(), subscriptions->end(), handler);
            if (subscription != nullptr)
            {
                subscriptions->remove(subscription);
            }
        }
    }

    // publish a message to all subscribers of the type
    template <class T>
    static void Publish(const T& message)
    {
        const std::unique_ptr<Subscriptions>& subscriptions = (*handlers)[typeid(T)];
        if (subscriptions != nullptr)
        {
            for (const auto& subscription : *subscriptions)
            {
                ((Subscription<T>*)(subscription.get()))->Handler(message);
            }
        }
    }
};
//---------------------------------------------------------------------------
} // namespace Messaging
//---------------------------------------------------------------------------
#endif