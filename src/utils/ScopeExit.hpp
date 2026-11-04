#include <type_traits>

template<typename F>
concept Callable = std::is_invocable_v<F>;

template<Callable F>
class ScopeExit {
    F m_cb;
    bool m_canceled = false;
public:
    explicit ScopeExit(F cb): m_cb(cb) {}
    void cancel() {this->m_canceled = true;}

    ~ScopeExit() {
        if (!this->m_canceled) {
            this->m_cb();
        }
    }
};
