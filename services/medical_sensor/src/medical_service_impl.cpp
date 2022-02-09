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

#include "medical_service_impl.h"
#include <cmath>
#include <cstring>
#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::SENSOR_SERVICE, "MedicalSensorServiceImpl" };
}

DataCacheFunc MedicalSensorServiceImpl::cacheData_ = nullptr;
sptr<ReportDataCache> MedicalSensorServiceImpl::reportDataCache_ = nullptr;
std::mutex MedicalSensorServiceImpl::dataMutex_;
std::condition_variable MedicalSensorServiceImpl::dataCondition_;

ErrCode MedicalSensorServiceImpl::InitSensorServiceImpl()
{
    HiLog::Info(LABEL, "%{public}s begin", "InitSensorServiceImpl");
    sensorInterface_ = NewSensorInterfaceInstance();
    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, " %{public}s,", "test sensorHdi get Module instance failed\n\r");
        return ERR_INVALID_VALUE;
    }
    HiLog::Info(LABEL, "%{public}s end", "InitSensorServiceImpl");
    return ERR_OK;
}

std::vector<MedicalSensor> MedicalSensorServiceImpl::GetSensorList() const
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);

    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, " %{public}s,", "test sensorHdi get Module instance failed\n\r");
        return std::vector<MedicalSensor>();
    }

    struct SensorInformation *sensorInfo = nullptr;
    int32_t count = 0;
    int32_t ret = sensorInterface_->GetAllSensors(&sensorInfo, &count);
    if (ret != 0) {
        HiLog::Error(LABEL, "Get sensor list failed!");
        return std::vector<MedicalSensor>();
    }
    HiLog::Info(LABEL, "GetAllSensors success, sensor counts: %{public}d", count);
    std::vector<MedicalSensor> list;
    for (int i = 0; i < count; i++) {
        const std::string sensorName(sensorInfo->sensorName);
        const std::string vendorName(sensorInfo->vendorName);
        const int32_t sensorId = sensorInfo->sensorId;
        HiLog::Info(LABEL, " %{public}d, %{public}s, %{public}s", sensorId, sensorName.c_str(), 
                    vendorName.c_str());
        MedicalSensor sensor;
        sensor.SetSensorId(sensorId);
        sensor.SetName(sensorName.c_str());
        sensor.SetVendor(vendorName.c_str());
        list.push_back(sensor);
        sensorInfo++;
    }
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return list;
}

ErrCode MedicalSensorServiceImpl::EnableSensor(uint32_t sensorId) const
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);

    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, " %{public}s,", "test sensorHdi get Module instance failed\n\r");
        return ERR_INVALID_VALUE;
    }

    int32_t ret = sensorInterface_->Enable(sensorId);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s is failed", __func__);
        return -1;
    }
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ERR_OK;
}

ErrCode MedicalSensorServiceImpl::DisableSensor(uint32_t sensorId) const
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);

    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, " %{public}s,", "test sensorHdi get Module instance failed\n\r");
        return ERR_INVALID_VALUE;
    }

    int32_t ret = sensorInterface_->Disable(sensorId);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s is failed", __func__);
        return -1;
    }
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ERR_OK;
}

ErrCode MedicalSensorServiceImpl::SetOption(uint32_t sensorId, uint32_t opt) const
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);

    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, " %{public}s,", "test sensorHdi get Module instance failed\n\r");
        return ERR_INVALID_VALUE;
    }

    int32_t ret = sensorInterface_->SetOption(sensorId, opt);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s is failed", __func__);
        return -1;
    }
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ERR_OK;

}

ErrCode MedicalSensorServiceImpl::RunCommand(uint32_t sensorId, int32_t cmd, int32_t params) const
{
    return ERR_OK;
}

ErrCode MedicalSensorServiceImpl::SetSensorConfig(uint32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs) const
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);

    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, " %{public}s,", "test sensorHdi get Module instance failed\n\r");
        return ERR_INVALID_VALUE;
    }

    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return ERR_OK;
}

int32_t MedicalSensorServiceImpl::AfeDataCallback(const struct SensorEvents *event)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);

    if ((event == nullptr) || (event->dataLen == 0)) {
        HiLog::Error(LABEL, "%{public}s event is NULL", __func__);
        return ERR_INVALID_VALUE;
    }

    if ((reportDataCache_ == nullptr) || (cacheData_ == nullptr)) {
        HiLog::Error(LABEL, "%{public}s reportDataCache_ and cacheData_ cannot be null", __func__);
        return ERR_INVALID_VALUE;
    }
    (void)(reportDataCache_->*cacheData_)(reinterpret_cast<const struct SensorEvent*>(event), 
                                                reportDataCache_);
    dataCondition_.notify_one();
    return ERR_OK;
}

ErrCode MedicalSensorServiceImpl::Register(RecordDataCallback cb) const
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, " %{public}s,", "test sensorHdi get Module instance failed\n\r");
        return ERR_INVALID_VALUE;
    }
    int32_t ret = sensorInterface_->Register(SENSORS_HEALTH, cb);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s failed", __func__);
        return ERR_INVALID_VALUE;
    }
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ERR_OK;
}

ErrCode MedicalSensorServiceImpl::RegisteDataReport(DataCacheFunc cacheData, sptr<ReportDataCache> reportDataCallback)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (reportDataCallback == nullptr) {
        HiLog::Error(LABEL, "%{public}s failed, reportDataCallback cannot be null", __func__);
        return ERR_NO_INIT;
    }

    Register(AfeDataCallback);

    cacheData_ = cacheData;
    reportDataCache_ = reportDataCallback;
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ERR_OK;
}

ErrCode MedicalSensorServiceImpl::Unregister(void) const
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);

    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, " %{public}s,", "test sensorHdi get Module instance failed\n\r");
        return ERR_INVALID_VALUE;
    }

    int32_t ret = sensorInterface_->Unregister(SENSORS_HEALTH, NULL);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s failed", __func__);
        return ERR_INVALID_VALUE;
    }
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ERR_OK;
}
}  // namespace Sensors
}  // namespace OHOS
