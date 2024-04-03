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

#ifndef LIBSMART_STM32THREADXTICKTIMER_HPP
#define LIBSMART_STM32THREADXTICKTIMER_HPP

#include <chrono>
#include "tx_api.h"
//#include "Stm32ThreadxThread.hpp"


namespace Stm32ThreadxThread {
    namespace native {
        // these macros use native type casts, so need some redirection
        constexpr ULONG infinite_delay = TX_WAIT_FOREVER;
        constexpr ULONG tick_rate_Hz = TX_TIMER_TICKS_PER_SECOND;
        using ULONG = ULONG;
    }

    /**
     * @brief A class that wraps the ThreadX tick timer.
     */
    class tick_timer {
    public:
        using rep = native::ULONG;
        using period = std::ratio<1, native::tick_rate_Hz>;
        using duration = std::chrono::duration<rep, period>;
        using time_point = std::chrono::time_point<tick_timer>;
        static constexpr bool is_steady = true;

        /**
         * @brief Wraps the current OS tick count into a clock time point.
         * @return The current tick count as time_point
         * @remark Thread and ISR context callable
         */
        static time_point now();
    };

    /**
     * @brief Converts duration to the underlying tick count.
     *
     * This function takes a time duration in the `tick_timer` scale and returns the equivalent
     * tick count. The `tick_timer::duration` parameter represents a duration in terms of
     * ticks.
     *
     * @param duration The time duration in the `tick_timer` scale.
     * @return The tick count equivalent to the given duration.
     *
     * @note The `tick_timer::rep` type is expected to be an unsigned long integer.
     * The `tick_timer::duration` type is defined as `std::chrono::duration<rep, period>`,
     * where `rep` is the underlying tick representation and `period` is the tick period.
     *
     * @see tick_timer::rep
     * @see tick_timer::duration
     */
    constexpr tick_timer::rep toTicks(const tick_timer::duration &duration) {
        return duration.count();
    }

    /**
     * @brief Converts time point to the underlying tick count.
     *
     * This function takes a time point in the `tick_timer` scale and returns the equivalent
     * tick count. The `tick_timer::time_point` parameter represents a point in time in terms of ticks
     * since the start of the `tick_timer`.
     *
     * @param time The time point from the start of the `tick_timer`.
     * @return The tick count equivalent to the given time point.
     *
     * @note The `tick_timer::time_point` type is defined as `std::chrono::time_point<tick_timer>`,
     * where `tick_timer` is a user-defined tick-based clock. The underlying representation
     * of the time point is a duration in terms of ticks.
     *
     * @see tick_timer::time_point
     * @see tick_timer::duration
     */
    constexpr tick_timer::rep toTicks(const tick_timer::time_point &time) {
        return toTicks(time.time_since_epoch());
    }

    /**
     * @brief  Dedicated @ref tick_timer::duration expression that ensures infinite wait time on an operation.
     */
    constexpr tick_timer::duration infinity{native::infinite_delay};
}

#endif //LIBSMART_STM32THREADXTICKTIMER_HPP
