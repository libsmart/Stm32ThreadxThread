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
