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

#include <cassert>
#include "Stm32ThreadxThread.hpp"

using namespace Stm32ThreadxThread;
using namespace Stm32ThreadxThread::native;

thread::thread(void *pstack,
               std::uint32_t stack_size,
               threadEntry func,
               native::ULONG param,
               priority prio,
               const char *name) : pstack(pstack), stack_size(stack_size), func(func), param(param), prio(prio),
                                   name(name), TX_THREAD_STRUCT() {}

void thread::createThread() {
    // https://github.com/eclipse-threadx/rtos-docs/blob/main/rtos-docs/threadx/chapter4.md#tx_thread_create
    auto result = tx_thread_create(
            this,                       // TX_THREAD *thread_ptr
            const_cast<char *>(name),   // CHAR *name_ptr
            func,                       // VOID (*entry_function)(ULONG id)
            param,                      // ULONG entry_input
            pstack,                     // VOID *stack_start
            stack_size,                 // ULONG stack_size
            prio,                       // UINT priority
            prio,                       // UINT preempt_threshold
            TX_NO_TIME_SLICE,           // ULONG time_slice
            TX_DONT_START);             // UINT auto_start
    assert(result == TX_SUCCESS);
}


thread::~thread() {
    if (tx_thread_state != TX_COMPLETED) {
        auto result = tx_thread_terminate(this);
        assert(result == TX_SUCCESS);
    }
    auto result = tx_thread_delete(this);
    assert(result == TX_SUCCESS);
}


void thread::suspend() {
    tx_thread_suspend(this);
}

void thread::resume() {
    tx_thread_resume(this);
}

void thread::terminate() {
    tx_thread_terminate(this);
}

void thread::reset() {
    tx_thread_reset(this);
}

thread::priority thread::getPriority() const {
    return tx_thread_user_priority;
}

void thread::setPriority(priority prio) {
    priority::value_type old_prio;
    tx_thread_priority_change(this, prio, &old_prio);
}

thread::id thread::getId() const {
    return id(this);
}

const char *thread::getName() {
    return const_cast<const char *>(tx_thread_name);
}


thread::state thread::getState() const {
    state s;
    switch (tx_thread_state) {
        case TX_READY:
            s = (getCurrent() == this) ? state::running : state::ready;
            break;
        case TX_COMPLETED:
            s = state::completed;
            break;
        case TX_TERMINATED:
            s = state::terminated;
            break;
        default:
            s = state::suspended;
            break;
    }
    return s;
}

thread *thread::getCurrent() {
    return reinterpret_cast<thread *>(tx_thread_identify());
}

void this_thread::yield() {
    tx_thread_relinquish();
}

thread::id this_thread::getId() {
    return thread::getCurrent()->getId();
}

void this_thread::sleepFor(tick_timer::duration rel_time) {
    auto result = tx_thread_sleep(toTicks(rel_time));
    assert(result == TX_SUCCESS);
}


#ifndef TX_DISABLE_NOTIFY_CALLBACKS

void thread::set_entry_exit_callback(entry_exit_callback func, void* param)
    {
        if (TX_SUCCESS == tx_thread_entry_exit_notify(this, reinterpret_cast<void(*)(TX_THREAD *, unsigned)>(func)))
        {
            // TODO: make sure that tx_user.h contains this:
            // #define TX_THREAD_USER_EXTENSION     void* entry_exit_param_;
            entry_exit_param_ = param;
        }
    }

// fun name collision between macro used right above, and member variable used below
#undef tx_thread_entry_exit_notify

    thread::entry_exit_callback thread::get_entry_exit_callback() const
    {
        return reinterpret_cast<entry_exit_callback>(this->tx_thread_entry_exit_notify);
    }

    void* thread::get_entry_exit_param() const
    {
        return this->entry_exit_param_;
    }

    bool thread::joinable() const
    {
        auto state = get_state();
        return (state != state::completed) && (state != state::terminated) && (nullptr == get_entry_exit_param());
    }

    void thread::join_exit_callback(thread *t, UINT id)
    {
        if (id == TX_THREAD_EXIT)
        {
            auto *exit_cond = reinterpret_cast<semaphore*>(t->get_entry_exit_param());

            exit_cond->release();
        }
    }

    void thread::join()
    {
        assert(joinable()); // else invalid_argument
        assert(this->get_id() != this_thread::get_id()); // else resource_deadlock_would_occur

        binary_semaphore exit_cond;
        set_entry_exit_callback(&thread::join_exit_callback, reinterpret_cast<void*>(&exit_cond));

        // wait for signal from thread exit
        exit_cond.acquire();

        // signal received, thread is deleted, return
    }

#endif // !TX_DISABLE_NOTIFY_CALLBACKS
