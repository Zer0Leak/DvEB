// author: Edgard Lima <edgard.lima@gmail.com>

#ifndef BIGDATA_OBSERVER_H
#define BIGDATA_OBSERVER_H

#include <cstdint>

#include <functional>
#include <map>
#include <vector>
#include <utility>
#include <memory>
#include <mutex>

#include <boost/log/trivial.hpp>

namespace bigdata {

template <typename Event>
struct ObserverHandle;

template <typename Event>
class ObservableBase : public std::enable_shared_from_this<ObservableBase<Event>> {
public:
    virtual ~ObservableBase() = default;

    std::shared_ptr<ObservableBase<Event>> getptr() {
        return this->shared_from_this();
    }

    virtual bool remove(ObserverHandle<Event> * observer_info) = 0;
};

template <typename Event>
struct ObserverHandle {
    ObserverHandle(const Event & e, std::shared_ptr<ObservableBase<Event>> o) : event(e), observable(o) {}
    Event event;
    std::shared_ptr<ObservableBase<Event>> observable;
    ~ObserverHandle() {
        if (observable) {
            observable->remove(this);
        }
    }
};


template <typename Event, typename... Args>
class Observable : public ObservableBase<Event>
{

public:
    Observable() = default;
    virtual ~Observable() = default;
    Observable(const Observable &) = delete;
    Observable &operator=(const Observable &) = delete;

    template <typename Observer>
    std::shared_ptr<ObserverHandle<Event>> add(const Event &event, Observer &&observer) {
        std::lock_guard<std::mutex> lk(mutex_);
        auto observer_handle = std::make_shared<ObserverHandle<Event>>(event, this->shared_from_this());
        observers_[event].push_back(std::make_pair(observer_handle.get(), std::forward<Observer>(observer)));

        BOOST_LOG_TRIVIAL(trace) << "Observer " << observer_handle.get() << " created for "  << event;

        return observer_handle;
    }

    template <typename Observer>
    std::shared_ptr<ObserverHandle<Event>> add(Event &&event, Observer &&observer) {
        std::lock_guard<std::mutex> lk(mutex_);
        auto observer_handle = std::make_shared<ObserverHandle<Event>>(event, this->shared_from_this());
        observers_[event].push_back(std::make_pair(observer_handle.get(), std::forward<Observer>(observer)));

        BOOST_LOG_TRIVIAL(trace) << "Observer " << observer_handle.get() << " created for&& " << event;

        return observer_handle;
    }

    template <typename... Parameters>
    bool notify(const Event &event, Parameters && ... params) const {
        std::lock_guard<std::mutex> lk(mutex_);
        auto it = observers_.find(event);
        if (it != observers_.end()) {
            for (const auto &observer : it->second) {
                try {
                    BOOST_LOG_TRIVIAL(trace) << "Notifying " << event << " to " << observer.first;
                    if (observer.first->observable.get() == this) {
                        if (observer.second(event, std::forward<Parameters &&>(params)...)) {
                            return true;
                        }
                    } else {
                        BOOST_LOG_TRIVIAL(error) << "Notifying a stranger observer for " << event;
                    }
                } catch (const std::runtime_error & e) {
                    BOOST_LOG_TRIVIAL(error) << "Got an exception from Observer: " << e.what() << " for " << event;
                } catch (...) {
                    BOOST_LOG_TRIVIAL(error) << "Got an unknown exception from Observer for " << event;
                }
            }
        }
        return false;
    }


    template <typename... Parameters>
    bool notify_all_force(const Event &event, Parameters && ... params) const {
        std::lock_guard<std::mutex> lk(mutex_);
        for (auto it = observers_.begin(); it != observers_.end(); it++) {
            for (const auto &observer : it->second) {
                try {
                    BOOST_LOG_TRIVIAL(trace) << "Notifying All " << event << " to " << observer.first;
                    if (observer.first->observable.get() == this) {
                        if (observer.second(event, std::forward<Parameters &&>(params)...)) {
                            // lets continue to all
                        }
                    } else {
                        BOOST_LOG_TRIVIAL(error) << "Notifying All a stranger observer for " << event;
                    }
                } catch (const std::runtime_error & e) {
                    BOOST_LOG_TRIVIAL(error) << "Got an exception from Observer: " << e.what() << " for " << event;
                } catch (...) {
                    BOOST_LOG_TRIVIAL(error) << "Got an unknown exception from Observer for " << event;
                }
            }
        }
        return false;
    }


    virtual bool remove(ObserverHandle<Event> * observer_handle) {
        if (observer_handle == NULL) return false;

        std::lock_guard<std::mutex> lk(mutex_);
        if (observer_handle->observable.get() != this) {
            BOOST_LOG_TRIVIAL(error) << "Observer " << observer_handle << " has different observable of " << observer_handle->observable.get();
            return false;
        }

        auto callback_vector_it = observers_.find(observer_handle->event);
        if (callback_vector_it != observers_.end()) {
            for (auto it = callback_vector_it->second.begin(); it != callback_vector_it->second.end(); it++) {
                if (it->first == observer_handle) {
                    BOOST_LOG_TRIVIAL(trace) << "Observer " << observer_handle << " removed of " << observer_handle->event;
                    callback_vector_it->second.erase(it);
                    observer_handle->observable.reset();
                    return true;
                }
            }
            BOOST_LOG_TRIVIAL(error) << "Observer " << observer_handle << " is not present on vector of " << observer_handle->event;
        } else {
            BOOST_LOG_TRIVIAL(error) << "Observer " << observer_handle << " vector event of " << observer_handle->event << "is already empty";
        }
        return false;
    }

private:
    mutable std::mutex mutex_;
    std::map<Event, std::vector<std::pair<ObserverHandle<Event>*, std::function<bool(const Event &, const Args & ...)>>>> observers_;

};

}

#endif  // BIGDATA_OBSERVER_H
