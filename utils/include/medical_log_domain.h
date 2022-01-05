/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SENSORS_LOG_DOMAIN_H
#define SENSORS_LOG_DOMAIN_H

#include "hilog/log.h"

namespace OHOS {
namespace MedicalSensorLogDomain {
constexpr uint32_t COMMON = 0xD002700;
constexpr uint32_t SENSOR_ADAPTER = 0xD002701;
constexpr uint32_t SENSOR_SERVICE = 0xD002702;
constexpr uint32_t SENSOR_UTILS = 0xD002703;
constexpr uint32_t SENSOR_TEST = 0xD002704;
constexpr uint32_t SENSOR_NATIVE = 0xD002705;
constexpr uint32_t SENSOR_JNI = 0xD002706;
constexpr uint32_t SENSORS_IMPLEMENT = 0xD002707;
constexpr uint32_t SENSORS_INTERFACE = 0xD002708;
}  // namespace MedicalSensorLogDomain
}  // namespace OHOS
#endif  // SENSORS_LOG_DOMAIN_H
