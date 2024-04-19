/*
 * SPDX-FileCopyrightText: 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 Roland Rusch, easy-smart solution GmbH <roland.rusch@easy-smart.ch>
 *
 * This file is part of libsmart/Stm32ThreadxThread, which is distributed under the terms
 * of the BSD 3-Clause License. You should have received a copy of the BSD 3-Clause
 * License along with libsmart/Stm32ThreadxThread. If not, see <https://spdx.org/licenses/BSD-3-Clause.html>.
 *
 * ----------------------------------------------------------------------------
 * Portions of the code are derived from Benedek Kupper's work,
 * which is licensed under the MIT License. You can find the original work at:
 * <https://github.com/IntergatedCircuits/threadx-mcpp/>
 *
 * Portions of the code are derived from Embedded Artistry's work,
 * which is dedicated to the public domain under the CC0 1.0 Universal (CC0 1.0) Public Domain Dedication.
 * You can find the original work at: <https://github.com/embeddedartistry/embedded-resources>
 * ----------------------------------------------------------------------------
 */

#ifndef LIBSMART_STM32THREADXTHREAD_STM32THREADXTHREAD_HPP
#define LIBSMART_STM32THREADXTHREAD_STM32THREADXTHREAD_HPP

#include <cstdint>
#include <type_traits>
#include "tx_api.h"
#include "Stm32ThreadxTickTimer.hpp"

namespace Stm32ThreadxThread {

    /**
     * @brief Bounce Function
     *
     * This template function acts as a bounce function to call a member function (Method) on an object (T) passed as a
     * pointer using the input thread_input. It forwards any additional parameters to the member function call.
     *
     * @tparam T The class type of the object on which the member function will be called
     * @tparam Method The type of the member function
     * @tparam m Pointer to the member function
     * @tparam Params The types of additional parameters to forward to the member function call
     *
     * @param thread_input The integral value representing the pointer to the object on which the member function will be called
     * @param params The additional parameters to be forwarded to the member function
     *
     * @return The return value of the member function call
     *
     * @note The caller is responsible for ensuring that thread_input represents a valid pointer to an object of type T
     * and the member function pointed to by m is of correct signature and can be called with the provided parameters.
     */
    template<class T, class Method, Method m, class ...Params>
    static auto bounce(ULONG thread_input, Params... params) ->
    decltype(((*reinterpret_cast<T *>(thread_input)).*m)(params...)) {
        return ((*reinterpret_cast<T *>(thread_input)).*m)(params...);
    }

#define BOUNCE(c, m) bounce<c, decltype(&c::m), &c::m>

    namespace native {
        // these macros may use native type casts, so need some redirection
        constexpr UINT TOP_PRIORITY = TX_MAX_PRIORITIES;
        constexpr ULONG MIN_STACK_SIZE = TX_TIMER_THREAD_STACK_SIZE;
        constexpr UINT THREAD_EXIT_ID = TX_THREAD_EXIT;
        using UINT = UINT;
        using ULONG = ULONG;
        using TX_THREAD_STRUCT = TX_THREAD_STRUCT;
    }

    /**
     * @class thread
     *
     * @brief Provides functionality to create and manage threads.
     *
     * This class provides a high-level interface to create and manage threads in the application.
     * It allows suspending, resuming, joining, and getting various properties of a thread.
     */
    class thread : private native::TX_THREAD_STRUCT {
    public:
        using threadEntry = void (*)(native::ULONG id);
        using id = std::uintptr_t;

        ~thread();

        /**
         * @brief Create a new thread.
         *
         * This function creates a new thread with the specified parameters. The thread is created by calling `tx_thread_create()` function.
         *
         * @note The name can be set using the `setName()` function.
         * See `thread::thread()` for default values of `name` and `stack_size`.
         *
         * @see thread::thread(), setName()
         */
        void createThread();

        /**
         * @brief Suspends the execution of the current thread.
         *
         *
         * Suspends the execution of the current thread by calling `tx_thread_suspend()`.
         * This function is a member function of the `thread` class.
         * It can be used to temporarily pause the current thread's execution.
         *
         * @note Calling `suspend()` on a thread that is already suspended has no effect.
         *
         * @see resume()
         */
        void suspend();

        /**
         * @brief Resumes the execution of the current thread.
         *
         * This function is a member function of the `thread` class.
         * It resumes the execution of the current thread by calling `tx_thread_resume()`.
         * When a thread is resumed, it continues execution from where it was previously suspended.
         *
         * @note Calling `resume()` on a thread that is not suspended has no effect.
         *
         * @see suspend()
         */
        void resume();

        /**
         * @brief Terminate the current thread.
         *
         * This function terminates the execution of the current thread by calling the _txe_thread_terminate() function.
         * Terminating a thread means that its execution is ended immediately, and its resources are released.
         */
        void terminate();

        /**
         * @brief Get the ID of the thread.
         *
         * This function returns the ID of the thread on which it is called. The ID is represented as an `id` object,
         * which is a typedef for `std::uintptr_t`.
         *
         * @return The ID of the thread.
         */
        [[nodiscard]] id getId() const;

        /**
         * @brief Returns the name of the thread.
         *
         * This function returns the name of the thread on which it is called. The name is represented as a null-terminated string of type `const char*`.
         * The name can be set using the `setName()` function.
         *
         * @return The name of the thread.
         */
        const char *getName();

        /**
         * @enum state
         * @brief Defines the possible states of a process.
         *
         * This enumeration class defines the possible states of a process: running, ready, completed, terminated, and suspended.
         */
        enum class state {
            running = 0,
            ready,
            completed,
            terminated,
            suspended,
        };

        /**
         * @brief Get the current state of the thread.
         *
         * This function returns the current state of the thread. The possible states are running, ready, completed,
         * terminated, and suspended, represented by the `state` enum class.
         *
         * @note The `nodiscard` attribute indicates that the return value of this function should not be ignored.
         *
         * @return The current state of the thread.
         *
         * @see thread::state
         */
        [[nodiscard]] state getState() const;

        /**
         * @brief Gets a pointer to the current thread object.
         *
         * This function returns a pointer to the current thread object.
         *
         * @return A pointer to the current thread object.
         */
        static thread *getCurrent();

        /**
         * @class priority
         * @brief Represents a priority value.
         */
        class priority {
        public:
            using value_type = native::UINT;

            constexpr priority()
                    : value_(1) {}

            constexpr priority(value_type value)
                    : value_(value) {}

            explicit operator value_type &() {
                return value_;
            }

            constexpr operator value_type() const {
                return value_;
            }

            static constexpr priority max() {
                return native::TOP_PRIORITY;
            }

            static constexpr priority min() {
                return 0;
            }

        private:
            value_type value_;
        };

        /**
         * @brief Get the priority of the thread.
         *
         * This function returns the priority of the thread. The priority is represented by an object of the `priority` class.
         *
         * @note The `nodiscard` attribute indicates that the return value of this function should not be ignored.
         *
         * @return The priority of the thread.
         *
         * @see thread::priority
         */
        [[nodiscard]] priority getPriority() const;

        /**
         * @brief Set the priority of the thread.
         *
         * This function sets the priority of the thread to the specified value. The priority is represented by an object of the `priority` class.
         * The new priority is used to update the thread's priority using the `_txe_thread_priority_change()` function.
         *
         * @note The thread's priority can range from the minimum priority value (0) to the maximum priority value (native::TOP_PRIORITY).
         *
         * @param prio The new priority value for the thread.
         *
         * @see thread::priority, _txe_thread_priority_change()
         */
        void setPriority(priority prio);

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        private:
            using entry_exit_callback = void (*)(thread*, native::UINT id);

            void set_entry_exit_callback(entry_exit_callback func, void* param);
            entry_exit_callback get_entry_exit_callback() const;
            void* get_entry_exit_param() const;
            static void join_exit_callback(thread *t, native::UINT id);

        public:
            /// @brief  Waits for the thread to finish execution.
            /// @note   May only be called when the thread is joinable, and not from the owned thread's context
            void join();

            /// @brief  Checks if the thread is joinable (potentially executing).
            /// @return true if the thread is valid and hasn't been joined, false otherwise
            /// @remark Thread and ISR context callable
            bool joinable() const;

#endif // !TX_DISABLE_NOTIFY_CALLBACKS

    protected:
        static constexpr const char *DEFAULT_NAME = "N/A";
        static constexpr size_t DEFAULT_STACK_SIZE = native::MIN_STACK_SIZE;

        thread(void *pstack, std::uint32_t stack_size,
               threadEntry func, native::ULONG param,
               priority prio, const char *name);

    private:
        thread(const thread &) = delete;

        thread &operator=(const thread &) = delete;

        thread(const thread &&) = delete;

        thread &operator=(const thread &&) = delete;

        void *pstack{};
        std::uint32_t stack_size{};
        threadEntry func{};
        native::ULONG param{};
        priority prio{};
        const char *name{};
    };


    /**
     * @class static_thread
     * @brief Class for creating and managing static threads.
     *
     * This class extends the `thread` class and provides functionality to create and manage static threads in the application.
     * It allows creating threads with a fixed stack size and provides constructors for various types of entry functions.
     * The stack size is defined by the template parameter `STACK_SIZE_BYTES`.
     *
     * @tparam STACK_SIZE_BYTES The size of the stack in bytes.
     */
    template<const std::size_t STACK_SIZE_BYTES>
    class static_thread : public thread {
    public:
        static constexpr std::size_t STACK_SIZE = STACK_SIZE_BYTES;

        static_thread(threadEntry func, native::ULONG param,
                      priority prio = priority(), const char *name = DEFAULT_NAME)
                : thread(stack_, sizeof(stack_) / sizeof(stack_[0]),
                         func, param, prio, name) {
        }

        static_thread(threadEntry func, void *param,
                      priority prio = priority(), const char *name = DEFAULT_NAME)
                : thread(stack_, sizeof(stack_) / sizeof(stack_[0]),
                         func, param, prio, name) {
        }

        template<typename T>
        static_thread(typename std::enable_if<(sizeof(T) <= sizeof(std::uintptr_t)),
                void (*)(T)>::type func, T arg,
                      priority prio = priority(), const char *name = DEFAULT_NAME)
                : static_thread(reinterpret_cast<threadEntry>(func),
                                reinterpret_cast<void *>(static_cast<std::uintptr_t>(arg)),
                                prio, name) {
        }

        template<typename T>
        static_thread(void (*func)(T *), T *arg,
                      priority prio = priority(), const char *name = DEFAULT_NAME)
                : static_thread(reinterpret_cast<threadEntry>(func),
                                reinterpret_cast<void *>(arg),
                                prio, name) {
        }

        template<typename T>
        static_thread(void (*func)(T *), T &arg,
                      priority prio = priority(), const char *name = DEFAULT_NAME)
                : static_thread(reinterpret_cast<threadEntry>(func),
                                reinterpret_cast<void *>(arg),
                                prio, name) {
        }

        template<class T>
        static_thread(T &obj, void (T::*member_func)(),
                      priority prio = priority(), const char *name = DEFAULT_NAME)
                : static_thread(reinterpret_cast<threadEntry>(member_func),
                                reinterpret_cast<void *>(&obj),
                                prio, name) {
        }

        void operator delete(void *p) {
            ::delete p;
        }

    private:
        unsigned char stack_[STACK_SIZE_BYTES];
    };

    /**
     * @brief Namespace offering control on the current thread of execution
     */
    namespace this_thread {
        /**
         * @brief Yields execution of the current thread so the OS can schedule other thread(s) for the remainder of the time slice.
         *
         * This function allows the operating system to release the execution of the current thread, giving other threads a chance to execute.
         * The function call does not block the thread, but instead provides a hint to the OS scheduler that the current thread is willing to yield the CPU if necessary.
         * The OS will then schedule another thread to execute, which improves overall system performance.
         *
         * @note The actual implementation of this function may vary depending on the specific operating system or platform.
         * The function may call a platform-specific API to yield execution or behave in a different manner.
         *
         * @see this_thread::yield()
         * @see tx_thread_relinquish()
         */
        void yield();

        /**
         * @brief Provides a unique identifier of the current thread.
         *
         * This function returns the unique identifier of the current thread.
         *
         * @return The current thread's unique identifier
         */
        thread::id getId();

        /**
         * @brief Sleeps for a specified duration of time.
         *
         * This function causes the current thread to sleep (i.e., pause its execution)
         * for the specified relative duration of time. The duration is specified in the
         * `Stm32ThreadxThread::tick_timer::duration` scale.
         *
         * @param rel_time The duration of time to sleep for.
         *
         * @note This function internally uses `tx_thread_sleep` to put the thread to sleep.
         *
         * @see `tx_thread_sleep`
         * @see `toTicks`
         *
         * @since <Version>
         */
        void sleepFor(Stm32ThreadxThread::tick_timer::duration rel_time);

        /**
         * @brief Blocks the current thread's execution for a given duration.
         *
         * This function pauses the execution of the current thread for the specified
         * relative duration of time. The duration is specified by the template
         * parameters `Rep` and `Period` which represent the type and period of the time
         * duration, respectively.
         *
         * @tparam Rep The type representing the number of ticks in the duration.
         * @tparam Period The ratio representing the tick period.
         *
         * @param rel_time The duration of time to block the current thread.
         *
         * @note This function internally uses `std::this_thread::sleep_for` to block
         * the thread for the specified duration.
         *
         * @see `std::this_thread::sleepFor`
         */
        template<class Rep, class Period>
        void sleepFor(const std::chrono::duration<Rep, Period> &rel_time) {
            this_thread::sleepFor(std::chrono::duration_cast<tick_timer::duration>(rel_time));
            // workaround to prevent this function calling itself
//            const auto ticks_sleep_for = static_cast<void (*)(tick_timer::duration)>(&sleepFor);
//            ticks_sleep_for(std::chrono::duration_cast<tick_timer::duration>(rel_time));
        }

        /**
         * @brief Blocks the current thread's execution until the given deadline.
         *
         * This function blocks the execution of the current thread until the specified deadline
         * is reached. The deadline is specified by the template parameters `Clock` and `Duration`,
         * representing the clock and duration types, respectively.
         *
         * @tparam Clock The clock type used to represent time points.
         * @tparam Duration The duration type used to represent time intervals.
         *
         * @param abs_time The absolute time point representing the deadline to block the thread.
         *
         * @note This function internally calls `sleepFor` to block the thread for the required
         * duration.
         *
         * @see sleepFor
         */
        template<class Clock, class Duration>
        void sleepUntil(const std::chrono::time_point<Clock, Duration> &abs_time) {
            sleepFor(abs_time - Clock::now());
        }

#if 0 && (configUSE_TASK_NOTIFICATIONS == 1)

        bool notify_wait_for(const tick_timer::duration& rel_time,
                    thread::notify_flag clr_at_entry = 0, thread::notify_flag clr_at_exit = 0,
                    thread::notify_flag *received = nullptr);

            notify_value notify_value_wait_for(const tick_timer::duration& rel_time,
                    thread::notify_flag clr_at_entry = 0, thread::notify_flag clr_at_exit = 0,
                    thread::notify_flag *received = nullptr);

#endif // (configUSE_TASK_NOTIFICATIONS == 1)
    }
}

#endif //LIBSMART_STM32THREADXTHREAD_STM32THREADXTHREAD_HPP
