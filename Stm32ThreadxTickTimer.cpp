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

#include "Stm32ThreadxTickTimer.hpp"

using namespace Stm32ThreadxThread;
using namespace Stm32ThreadxThread::native;

tick_timer::time_point tick_timer::now() {
    rep ticks = tx_time_get();
    return time_point(duration(ticks));
}
