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

#include "permission_util.h"

#include <thread>

#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_UTILS, "PermissionUtil" };
constexpr uint32_t SENSOR_PPG_ID = 256;
constexpr uint32_t SENSOR_HEART_RATE_ID = 83886336;
constexpr int32_t GET_SERVICE_MAX_COUNT = 30;
constexpr uint32_t WAIT_MS = 200;
const std::string PPG_PERMISSION = "ohos.permission.PPG";
const std::string READ_HEALTH_DATA_PERMISSION = "ohos.permission.READ_HEALTH_DATA";
}  // namespace

std::unordered_map<uint32_t, std::string> PermissionUtil::sensorPermissions_ = {
    { SENSOR_PPG_ID, PPG_PERMISSION },
    { SENSOR_HEART_RATE_ID, READ_HEALTH_DATA_PERMISSION }
};

PermissionUtil::~PermissionUtil()
{
    appPermissionStatus_.clear();
}

bool PermissionUtil::CheckCallingPermission(const int32_t afeId)
{
    auto permissionIt = sensorPermissions_.find(afeId);
    if (permissionIt == sensorPermissions_.end()) {
        return true;
    }
    return true;
}

bool PermissionUtil::IsPermissionRegisted(int32_t uid)
{
    HiLog::Debug(LABEL, "%{public}s appPermissionStatus_.size : %{public}d", __func__,
        int32_t { appPermissionStatus_.size() });
    std::lock_guard<std::mutex> permissionLock(permissionStatusMutex_);
    auto permissionIt = appPermissionStatus_.find(uid);
    if (permissionIt != appPermissionStatus_.end()) {
        HiLog::Debug(LABEL, "%{public}s uid : %{public}d permission has registered", __func__, uid);
        return true;
    }
    return false;
}

bool PermissionUtil::RegistPermissionChanged(const MedicalThreadInfo &appThreadInfo)
{
    HiLog::Debug(LABEL, "%{public}s begin, uid : %{public}d, pid : %{public}d", __func__, appThreadInfo.uid,
                 appThreadInfo.pid);
    // Avoid registering callback functions repeatedly
    if (IsPermissionRegisted(appThreadInfo.uid)) {
        HiLog::Debug(LABEL, "%{public}s uid : %{public}d permission has registered", __func__, appThreadInfo.uid);
        return true;
    }
    int32_t retry = 0;
    while (retry < GET_SERVICE_MAX_COUNT) {
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
        HiLog::Error(LABEL, "%{public}s registered failed, retry : %{public}d", __func__, retry);
        retry++;
    }
    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return true;
}

void PermissionUtil::UnregistPermissionChanged(const MedicalThreadInfo &appThreadInfo)
{
    HiLog::Debug(LABEL, "%{public}s begin, uid : %{public}d, pid : %{public}d", __func__, appThreadInfo.uid,
                 appThreadInfo.pid);
    std::lock_guard<std::mutex> permissionLock(permissionStatusMutex_);
    auto permissionIt = appPermissionStatus_.find(appThreadInfo.uid);
    if (permissionIt != appPermissionStatus_.end()) {
        HiLog::Debug(LABEL, "%{public}s erase uid : %{public}d permission", __func__, appThreadInfo.uid);
        appPermissionStatus_.erase(permissionIt);
        return;
    }
    HiLog::Debug(LABEL, "%{public}s end", __func__);
}

void PermissionUtil::UpdatePermissionStatus(int32_t uid, const std::string &permissionName, bool permissionStatus)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    std::lock_guard<std::mutex> permissionLock(permissionStatusMutex_);
    auto permissionIt = appPermissionStatus_.find(uid);
    if (permissionIt == appPermissionStatus_.end()) {
        std::unordered_map<std::string, bool> permissionMap;
        permissionMap.insert(std::make_pair(permissionName, permissionStatus));
        appPermissionStatus_.insert(std::make_pair(uid, permissionMap));
        HiLog::Debug(LABEL, "%{public}s end", __func__);
        return;
    }
    permissionIt->second[permissionName] = permissionStatus;
    HiLog::Debug(LABEL, "%{public}s end", __func__);
}
}  // namespace Sensors
}  // namespace OHOS
