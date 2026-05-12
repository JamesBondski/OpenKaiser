export module Events;

import std;

namespace OpenKaiser {

    export class Connection {
    private:
        std::function<void()> disconnect_;
    public:
        Connection() = default;
        Connection(std::function<void()> disconnect) : disconnect_(std::move(disconnect)) {}

        ~Connection() { 
            disconnect();
        }

        void disconnect() {
            if (disconnect_) {
                disconnect_();
                disconnect_ = nullptr;
            }
        }

        // Move-only
        Connection(Connection&& other) noexcept = default;
        Connection& operator=(Connection&& other) noexcept = default;
        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;
    };

    export class ScopedConnections {
    private:
        std::vector<Connection> connections_;

    public:
        ScopedConnections() = default;
        ~ScopedConnections() = default;

        // Move-only
        ScopedConnections(ScopedConnections&& other) noexcept = default;
        ScopedConnections& operator=(ScopedConnections&& other) noexcept = default;
        ScopedConnections(const ScopedConnections&) = delete;
        ScopedConnections& operator=(const ScopedConnections&) = delete;

        ScopedConnections& operator+=(Connection&& connection) {
            connections_.push_back(std::move(connection));
            return *this;
        }

        void disconnect_all() {
            connections_.clear();
        }

        size_t count() const {
            return connections_.size();
        }
    };

    export template<typename... Args>
    class Event {
    private:
        std::map<int, std::function<void(Args...)>> callbacks_;
        int next_id_ = 0;

    public:
        Event() = default;
        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        Connection subscribe(std::function<void(Args...)> callback) {
            int id = next_id_++;
            callbacks_[id] = std::move(callback);
            return Connection([this, id]() { 
                callbacks_.erase(id); 
            });
        }

        template<typename T>
        Connection subscribe(T* obj, void (T::*method)(Args...)) {
            return subscribe([obj, method](Args... args) {
                (obj->*method)(args...);
            });
        }

        void emit(Args... args) {
            for (auto& [_, cb] : callbacks_) {
                cb(args...);
            }
        }

        size_t subscriber_count() const {
            return callbacks_.size();
        }
    };
}
