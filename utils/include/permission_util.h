/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
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

#ifndef PERMISSION_UTIL_H
#define PERMISSION_UTIL_H

#include <mutex>
#include <set>
#include <string>
#include <unordered_map>

#include "refbase.h"
#include "singleton.h"

namespace OHOS {
namespace Sensors {
struct MedicalThreadInfo {
    int32_t pid;
    int32_t uid;
    MedicalThreadInfo() : pid(0), uid(0) {};
    MedicalThreadInfo(int32_t pid, int32_t uid) : pid(pid), uid(uid) {};
};

class PermissionUtil : public Singleton<PermissionUtil> {
public:
    PermissionUtil() = default;
    virtual ~PermissionUtil();
    /* check local caller's permission by permission name */
    bool CheckCallingPermission(const std::string &permissionName);
    bool CheckCallingPermission(const int32_t afeId);
    bool RegistPermissionChanged(const MedicalThreadInfo &appThreadInfo);
    void UnregistPermissionChanged(const MedicalThreadInfo &appThreadInfo);
    void UpdatePermissionStatus(int32_t uid, const std::string &permissionName, bool permissionStatus);

private:
    bool IsPermissionRegisted(int32_t uid);
    std::mutex permissionStatusMutex_;
    std::unordered_map<int, std::unordered_map<std::string, bool>> appPermissionStatus_;

    static std::unordered_map<uint32_t, std::string> sensorPermissions_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // PERMISSION_UTIL_H
